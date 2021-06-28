class Builder():
    def __init__(self, data, signatures, variables_block, handlers_block, timer_block):
        self.bindings = list(elem for elem in data.get('bindings').get('timers') if elem.get('enabled', True) == True)

        self.signatures = signatures
        self.variables_block = variables_block
        self.handlers_block = handlers_block
        self.timer_block = timer_block

    def build(self):
        for binding in self.bindings:

            key = binding.get('name')
            properties = binding.get('properties')     

            binding.update({"signature_template": 'sig_timer'})
            self.signatures.update({key: binding})
            properties = binding['properties']

            timer_type = properties.get('type')
            period = properties.get('period')

            if timer_type is None or timer_type.lower() == 'periodic':
                # If period missing then set period to every 10 seconds
                period = period if period is not None else '{ 10, 0 }'

                binding.update({'period': period})

                # binding.update({"timer_template": 'declare_timer_periodic'})
                # self.timer_block.update({key: binding})

                
                binding.update({"variable_template": [('declare_timer_periodic', 'TIMER_BINDING')]})
                self.variables_block.update({key: binding})
                

                binding.update({"handler_template": 'handler_timer_periodic'})
                self.handlers_block.update({key: binding})
            else:
                # binding.update({"timer_template": 'declare_timer_oneshot'})
                # self.timer_block.update({key: binding})

                binding.update({"variable_template": [('declare_timer_oneshot', 'TIMER_BINDING')]})
                self.variables_block.update({key: binding})

                binding.update({"handler_template": 'handler_timer_oneshot'})
                self.handlers_block.update({key: binding})