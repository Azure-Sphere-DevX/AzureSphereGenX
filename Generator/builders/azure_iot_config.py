import json


class Builder():
    def __init__(self, data, manifest_updates):

        self.bindings = list(elem for elem in data.get('bindings').get(
            'azure_iot') if elem.get('enabled', True) == True)

        self.manifest_updates = manifest_updates

    def build(self, manifest):

        new_manifest = {**manifest, **{}}

        for item in self.bindings:
            manifest_updates = item.get('manifest')

            for key, value in manifest_updates.items():
                if type(value) is list:

                    manifest_list = manifest.get(key, [])

                    if type(manifest_list) is dict:
                        for item in value:
                            for k, v in item.items():

                                manifest_list.update({k: v})

                        new_manifest.update({key: manifest_list})
                        
                    elif type(manifest_list) is list:

                        new_manifest.update({key: value})

        return new_manifest
