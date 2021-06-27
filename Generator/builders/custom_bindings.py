import os
import json


signatures_block = {}
timer_block = {}
variables_block = {}
handlers_block = {}
includes_block = {}
templates = {}


class Builder():
    def __init__(self, data, templates, signatures_block, timer_block, variables_block, handlers_block, includes_block):

        self.bindings = list(elem for elem in data.get('bindings').get('custom') if elem.get('enabled', True) == True)

        self.templates = templates
        self.signatures_block = signatures_block
        self.timer_block = timer_block
        self.variables_block = variables_block
        self.handlers_block = handlers_block
        self.includes_block = includes_block

        self.components_dict = {}

    def load_custom_bindings(self):
        path = 'custom_bindings'

        custom_bindings = os.listdir(path)

        for key in custom_bindings:
            print(key)

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

        binding.update({'properties': properties.get('properties', {})})

        if '.device_twin' in component:
            binding_type = 'device_twin'
            binding_template = "DEVICE_TWIN_BINDING"
        elif '.direct_method' in component:
            binding_type = 'direct_method'
            binding_template = "DIRECT_METHOD_BINDING"
            binding.update({"signature_template": "sig_direct_method"})
        elif '.timer' in component:
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

        cloud2device = properties.get('cloud2device')

        binding.update({"variable_template": "declare_{type}".format(type=binding_type)})
        binding.update({"binding": binding_template})
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

    def get_custom_binding(self, component_key):

        component_list = self.components_dict.get(component_key, [])

        for component in component_list:

            component_filename = 'custom_bindings/' + component_key + '/' + component

            if '.binding' in component:
                self.build_binding(component, component_filename)
            elif '.include' in component:
                self.build_include(component, component_filename)
            elif '.handler' in component:
                self.build_handler(component, component_filename)
            elif '.signature' in component:
                self.build_signature(component, component_filename)

    def build(self):
        self.load_custom_bindings()
        for binding in self.bindings:
            self.get_custom_binding(binding.get('name'))



# cb = Builder(templates, signatures_block, timer_block,
#                      variables_block, handlers_block, includes_block)
# cb.load_custom_bindings()
# # cb.list_custom_bindings()
# cb.list_custom_binding_components()
# cb.get_custom_binding('DeferredUpdate')


# print(signatures_block)
# print(variables_block)
# print(includes_block)
# print(handlers_block)

# print(templates)
