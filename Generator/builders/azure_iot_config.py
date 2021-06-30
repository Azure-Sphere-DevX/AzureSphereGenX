import json


class Builder():
    def __init__(self, data, manifest_updates):

        self.bindings = list(elem for elem in data.get('bindings').get(
            'azure_iot') if elem.get('enabled', True) == True)

        self.manifest_updates = manifest_updates

    def string_escape(self, s, encoding='utf-8'):
        return (s.encode('latin1')         # To bytes, required by 'unicode-escape'
                .decode('unicode-escape') # Perform the actual octal-escaping decode
                .encode('latin1')         # 1:1 mapping back to bytes
                .decode(encoding))        # Decode original encoding

    def build(self, manifest):
        manifest_updates = {}
        new_manifest = {}
        new_manifest = {**manifest, **new_manifest}

        for item in self.bindings:
            manifest_updates = item.get('manifest')

            for key, value in manifest_updates.items():
                if type(value) is list:
                    manifest_list = manifest.get(key, [])
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
                        new_list = value + manifest_list
                        new_list = list(dict.fromkeys(new_list))
                        new_manifest.update({key:new_list})

        return new_manifest


            

        # return existing_manifest


            # for key, value in manifest.items():
            #     if existing_manifest.get(key) is None:
            #         existing_manifest.update({key:value})
            #     else:
            #         print(key, value)
            #         print(type(value))
            #         if type(value) is list:
                        
            #             for v in value:
            #                 keys = v.keys()
            #                 values = v.values()
            #                 existing_list = existing_manifest.get(v, [])
            #                 for item in values:
            #                     if item not in v:
            #                         existing_list.append(item)
            #                         print(item)
                            
            #             print(existing_list)
            #             existing_manifest.update({key : existing_list})

        return existing_manifest
