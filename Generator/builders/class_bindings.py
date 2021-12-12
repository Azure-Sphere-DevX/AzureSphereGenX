import os
import json
import yaml


class Builder():
    def __init__(self, data, templates, signatures_block, timer_block, variables_block, handlers_block, includes_block):

        self.bindings = list(elem for elem in data.get('components') if elem.get('enabled', True) == True)

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
                dict3[key] = [value, dict1[key]]
        return dict3

    def load_classes(self):
        path = 'classes'

        classes = os.listdir(path)

        for key in classes:

            binding_path = path + "/" + key
            components = os.listdir(binding_path)

            component_list = []

            for component in components:
                component_list.append(component)

            self.components_dict.update({key: component_list})

    def list_classes(self):
        print(self.components_dict)

    def list_custom_binding_components(self):
        for component in self.components_dict:
            print(component)
            print(self.components_dict.get(component, []))

    def build_binding(self, binding, component_name, component_filename):
        component_name = component_name.split('.')[0]

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                # properties = json.load(f)
                properties = yaml.load(f, Loader=yaml.FullLoader)
        else:
            properties = {}

        if properties.get('name') is not None:
            name = properties.get('name')
        else:
            name = binding.get('name')
        
        new_binding = {}
        new_binding.update({'name': name})

        variable_template = properties.get('variable_template')
        template_properties = properties.get('properties', {})
        binding_properties = binding.get('properties', {})
        combined_properties = {**template_properties, **binding_properties}

        new_binding.update({'properties': combined_properties})
        new_binding.update({"variable_template": variable_template})

        key = component_filename + "_" + name
        print(key)
        self.variables_block.update({key: new_binding})

    def build_include(self, binding, component_name, component_filename):
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

    def build_handler(self, binding, component_name, component_filename):
        # name = component_name.split('.')[0]
        name = binding.get('name')

        properties = binding.get('properties')

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                template = f.read()
        else:
            template = ''

        self.templates.update({component_name: template})

        new_binding = {}
        new_binding.update({'name': name})
        new_binding.update({'handler_template': component_name})
        new_binding.update({'properties': properties})
        key = component_name + "_" + name
        print(key)
        self.handlers_block.update({key: new_binding})

    def build_signature(self, binding, component_name, component_filename):

        if os.path.exists(component_filename):
            with open(component_filename, 'r') as f:
                template = yaml.load(f, Loader=yaml.FullLoader)
        else:
            template = {}

        self.templates.update({component_name: template})
        name = binding.get('name')
        # signature_template = template.get('signature_template')
        # if signature_template is not None:
        #     declare = signature_template.get('declare')
        declare = template.get('signature_template', 'SIGNATURE_MISSING')

        binding = {}
        binding.update({'name': name})
        binding.update({'signature_template': declare})
        key = component_name + "_" + name
        self.signatures_block.update({key: binding})

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
                                manifest_list.update({k: v})
                            else:
                                if type(manifest_list_value) is list:
                                    new_list = manifest_list_value + v
                                    new_list = list(dict.fromkeys(new_list))
                                    manifest_list.update({k: new_list})
                                else:
                                    # elif type(manifest_list_value) is str:
                                    manifest_list.update({k: v})
                    new_manifest.update({key: manifest_list})
                elif type(manifest_list) is list:
                    new_list = manifest_list + value
                    new_list = list(dict.fromkeys(new_list))
                    new_manifest.update({key: new_list})

    def get_custom_binding(self, binding, class_key):

        component_list = self.components_dict.get(class_key, [])

        for component in component_list:

            component_filename = 'classes/' + class_key + '/' + component
            component_lower = component.lower()

            if '.binding' in component_lower:
                self.build_binding(binding, component, component_filename)
            elif '.include' in component_lower:
                self.build_include(binding, component, component_filename)
            elif '.handler' in component_lower:
                self.build_handler(binding, component, component_filename)
            elif '.signature' in component_lower:
                self.build_signature(binding, component, component_filename)
            elif '.manifest' in component:
                self.build_manifest(component, component_filename)

    def build(self, manifest):
        self.manifest = manifest
        self.load_classes()
        for binding in self.bindings:
            self.get_custom_binding(binding, binding.get('class'))

        return self.manifest
