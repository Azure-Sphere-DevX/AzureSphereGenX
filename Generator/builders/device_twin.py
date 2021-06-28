

class Builder():
    def __init__(self, data, signatures, variables_block, handlers_block):
        self.bindings = list(elem for elem in data.get('bindings').get('device_twins') if elem.get('enabled', True) == True)

        self.signatures = signatures
        self.variables_block = variables_block
        self.handlers_block = handlers_block

        self.device_twin_template = {"integer": "handler_device_twin_int", "float": "handler_device_twin_float",
                                     "double": "handler_device_twin_double", "boolean": "handler_device_twin_bool",
                                     "string": "handler_device_twin_string"}


    def build(self):
        for binding in self.bindings:
            
            key = binding.get('name')
            properties = binding.get('properties')            

            if properties.get("cloud2device") == True:

                binding.update({"signature_template": 'sig_device_twin'})
                self.signatures.update({key: binding})

                binding.update({"variable_template": [('declare_device_twin_with_handler', 'DEVICE_TWIN_BINDING')]})
                self.variables_block.update({key: binding})

                binding.update({"handler_template": self.device_twin_template[properties.get('type')]})
                self.handlers_block.update({key: binding})

            else:
                binding.update({"variable_template": [('declare_device_twin', 'DEVICE_TWIN_BINDING')]})
                self.variables_block.update({key: binding})

    def build_publish_device_twins(self):
        # global device_twins_updates, device_twin_variables
        device_twins_updates = ''
        device_twin_variables = ''

        for item in self.variables_block:
            var = self.variables_block.get(item)
            if var.get("binding") == "DEVICE_TWIN_BINDING":

                property_name = var.get("name", None)
                properties = var.get('properties')
                property_type = properties.get("type", None)                

                if property_type == 'integer':
                    device_twin_variables += '    int {property_name}_value = 10;\n'.format(
                        property_name=property_name)
                    device_twins_updates += '        dx_deviceTwinReportState(&dt_{property_name}, &{property_name}_value);     // DX_TYPE_INT\n'.format(
                        property_name=property_name)
                elif property_type == 'float':
                    device_twin_variables += '    float {property_name}_value = 30.0f;\n'.format(
                        property_name=property_name)
                    device_twins_updates += '        dx_deviceTwinReportState(&dt_{property_name}, &{property_name}_value);     // DX_TYPE_FLOAT\n'.format(
                        property_name=property_name)
                elif property_type == 'double':
                    device_twin_variables += '    double {property_name}_value = 10.0;\n'.format(
                        property_name=property_name)
                    device_twins_updates += '        dx_deviceTwinReportState(&dt_{property_name}, &{property_name}_value);     // DX_TYPE_DOUBLE\n'.format(
                        property_name=property_name)
                elif property_type == 'boolean':
                    device_twin_variables += '    bool {property_name}_value = true;\n'.format(
                        property_name=property_name)
                    device_twins_updates += '        dx_deviceTwinReportState(&dt_{property_name}, &{property_name}_value);     // DX_TYPE_BOOL\n'.format(
                        property_name=property_name)
                elif property_type == 'string':
                    device_twin_variables += '    char {property_name}_value[] = "hello, world";\n'.format(
                        property_name=property_name)
                    device_twins_updates += '        dx_deviceTwinReportState(&dt_{property_name}, {property_name}_value);     // DX_TYPE_STRING\n'.format(
                        property_name=property_name)

        return (device_twins_updates, device_twin_variables)
