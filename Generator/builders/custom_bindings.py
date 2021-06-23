class Builder():
    def __init__(self, data, signatures, variables_block, handlers_block, timer_block, includes_block):
        self.bindings = list(elem for elem in data.get('bindings').get('custom') if elem.get('enabled', True) == True)

        self.signatures = signatures
        self.variables_block = variables_block
        self.handlers_block = handlers_block
        self.timer_block = timer_block
        self.includes_block = includes_block

    def get_value(self, properties, key):
        if properties is None:
            return ''
        return "" if properties.get(key) is None else properties.get(key)

    def process_binding(self, binding):
        key = binding.get('name')
        properties = binding.get('properties')   

        if binding.get('binding_template') is not None:
            binding.update({'binding': binding.get('binding_template')})

        if binding.get('signature_template') is not None:
            self.signatures.update({key: binding})
            properties = binding.get('properties')

        if binding.get('include_template') is not None:
            self.includes_block.update({key: binding})

        if binding.get('variable_template') is not None:
            self.variables_block.update({key: binding})

        if binding.get('handler_template') is not None:
            self.handlers_block.update({key: binding})

        timer_template = binding.get('timer_template')
        if timer_template is not None:
            self.timer_block.update({key: binding})

        elif self.get_value(properties, 'period') != "":                
            binding.update({"timer_template": 'declare_timer_periodic'})
            properties.update({"period": "{" + self.get_value(properties, 'period') + "}"})
            self.timer_block.update({key: binding})


    def build(self):
        for binding in self.bindings:

            components = binding.get('components')
            if components is not None:
                for component in components:
                    # create a new binding from the component
                    new_binding = {
                        "name": component.get('name'),
                        'binding_template' : component.get('binding_template'),
                        'signature_template': component.get('signature_template'),
                        'variable_template': component.get('variable_template'),
                        'handler_template': component.get('handler_template'),
                        'include_template': component.get('include_template')
                    }
                    self.bindings.append(new_binding)
                    self.process_binding(new_binding)

            else:
                self.process_binding(binding)


            