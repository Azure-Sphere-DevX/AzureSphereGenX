class Builder():
    def __init__(self, data, signatures, variables_block, handlers_block):
        self.bindings = list(elem for elem in data.get('bindings').get('direct_methods') if elem.get('enabled', True) == True)

        self.signatures = signatures
        self.variables_block = variables_block
        self.handlers_block = handlers_block

    def build(self):
        for binding in self.bindings:

            key = binding.get('name')

            binding.update({"signature_template": 'sig_direct_method'})
            self.signatures.update({key: binding})

            binding.update({"variable_template": [('declare_direct_method', 'DIRECT_METHOD_BINDING')]})
            self.variables_block.update({key: binding})

            binding.update({"handler_template": 'handler_direct_method'})
            self.handlers_block.update({key: binding})