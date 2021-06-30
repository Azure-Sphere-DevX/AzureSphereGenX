import os
import json


class Builder():
    def __init__(self, data, templates, signatures_block, timer_block, variables_block, handlers_block, includes_block):

        self.bindings = list(elem for elem in data.get('bindings').get(
            'custom') if elem.get('enabled', True) == True)

        self.templates = templates
        self.signatures_block = signatures_block
        self.timer_block = timer_block
        self.variables_block = variables_block
        self.handlers_block = handlers_block
        self.includes_block = includes_block

        self.manifest_updates = {}
        self.manifest = {}

        self.components_dict = {}

    def mergeDict(self, dict1, dict2):
        ''' Merge dictionaries and keep values of common keys in list'''
        dict3 = {**dict1, **dict2}
        for key, value in dict3.items():
            if key in dict1 and key in dict2:
                    dict3[key] = [value , dict1[key]]
        return dict3

    def load_custom_bindings(self):
        path = 'custom_bindings'

        custom_bindings = os.listdir(path)

        for key in custom_bindings:

            binding_path = path + "/" + key
            components = os.listdir(binding_path)

            component_list = []

            for component in components:
                component_list.append(component)

            self.components_dict.update({key: component_list})

    def list_custom_bindings(self):
        print(self.components_dict)

    def list_custom_binding_components(self):
        for component in self.components_dict:
            print(component)
            print(self.components_dict.get(component, []))

    def build_binding(self, component_name, component_filename):
        component = component_name.lower()
        name = component_name.split('.')[0]
        binding_type = ''
        add_signature = False

        binding = {}
        binding.update({'name': name})

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                properties = json.load(f)
        else:
            properties = {}

        properties = properties.get('properties')

        binding.update({'properties': properties})

        if '.device_twin' in component:
            binding_type = 'device_twin'
            binding_template = "DEVICE_TWIN_BINDING"
            if properties is not None:
                cloud2device = properties.get('cloud2device')
        elif '.direct_method' in component:
            binding_type = 'direct_method'
            binding_template = "DIRECT_METHOD_BINDING"
            binding.update({"signature_template": "sig_direct_method"})
        elif '.timer' in component:
            timer_type = None

            if properties is not None:
                timer_type = properties.get('type')

            if timer_type is not None and timer_type.lower() == 'oneshot':
                binding_type = 'timer_oneshot'
            else:
                binding_type = 'timer_periodic'

            binding_template = "TIMER_BINDING"
            binding.update({"signature_template": "sig_timer"})
        elif '.gpio_output' in component:
            binding_type = 'gpio_output'
            binding_template = "GPIO_BINDING"
        elif '.gpio_input' in component:
            binding_type = 'gpio_input'
            binding_template = "GPIO_BINDING"
        else:
            binding_type = ''
            binding_template = ''

        binding.update({"variable_template": [
                       ("declare_{type}".format(type=binding_type), binding_template)]})
        # binding.update({"binding": binding_template})
        self.variables_block.update({name: binding})

        signature_template = binding.get('signature_template')
        if signature_template is not None:
            self.signatures_block.update({name: binding})

    def build_include(self, component_name, component_filename):
        name = component_name.split('.')[0]

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                template = f.read()
        else:
            template = ''

        self.templates.update({component_name: template})

        binding = {}
        binding.update({'name': name})
        binding.update({'include_template': component_name})
        self.includes_block.update({name: binding})

    def build_handler(self, component_name, component_filename):
        name = component_name.split('.')[0]

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                template = f.read()
        else:
            template = ''

        self.templates.update({component_name: template})

        binding = {}
        binding.update({'name': name})
        binding.update({'handler_template': component_name})
        self.handlers_block.update({name: binding})

    def build_signature(self, component_name, component_filename):
        name = component_name.split('.')[0]

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                template = f.read()
        else:
            template = ''

        self.templates.update({component_name: template})

        binding = {}
        binding.update({'name': name})
        binding.update({'signature_template': component_name})
        self.signatures_block.update({name: binding})

    def build_manifest(self, component_name, component_filename):
        with open(component_filename, 'r') as f:
            custom_manifest = json.load(f)

        manifest_updates = {}
        new_manifest = {}
        new_manifest = {**self.manifest, **new_manifest}

        manifest_updates = custom_manifest.get('manifest')
        for key, value in manifest_updates.items():
            if type(value) is list:
                manifest_list = self.manifest.get(key, [])
                if type(manifest_list) is dict:
                    for item in value:
                        for k, v in item.items():
                            manifest_list_value = manifest_list.get(k)
                            if manifest_list_value is None:
                                manifest_list.update({k:v})
                            else:
                                if type(manifest_list_value) is list:
                                    new_list = manifest_list_value + v
                                    new_list = list(dict.fromkeys(new_list))
                                    manifest_list.update({k:new_list})
                                elif type(manifest_list_value) is str:
                                    manifest_list.update({k:v})                            
                    new_manifest.update({key:manifest_list})
                elif  type(manifest_list) is list:   
                    new_list = manifest_list + value
                    new_list = list(dict.fromkeys(new_list))
                    new_manifest.update({key:new_list})


    def get_custom_binding(self, component_key):

        component_list = self.components_dict.get(component_key, [])

        for component in component_list:

            component_filename = 'custom_bindings/' + component_key + '/' + component
            component_lower = component.lower()

            if '.binding' in component_lower:
                self.build_binding(component, component_filename)
            elif '.include' in component_lower:
                self.build_include(component, component_filename)
            elif '.handler' in component_lower:
                self.build_handler(component, component_filename)
            elif '.signature' in component_lower:
                self.build_signature(component, component_filename)
            elif '.manifest' in component:
                self.build_manifest(component, component_filename)

    def build(self, manifest):
        self.manifest = manifest
        self.load_custom_bindings()
        for binding in self.bindings:
            self.get_custom_binding(binding.get('name'))

        return self.manifest
