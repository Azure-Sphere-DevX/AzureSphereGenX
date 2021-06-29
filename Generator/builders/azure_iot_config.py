
import json


class Builder():
    def __init__(self, data, manifest_updates):

        self.bindings = list(elem for elem in data.get('bindings').get(
            'azure_iot') if elem.get('enabled', True) == True)

        self.manifest_updates = manifest_updates

    def mergeDict(self, dict1, dict2):
        ''' Merge dictionaries and keep values of common keys in list'''
        dict3 = {**dict1, **dict2}
        for key, value in dict3.items():
            if key in dict1 and key in dict2:
                print(type(key))
                dict3[key] = [value, dict1[key]]
        return dict3

    def build(self, existing_manifest):
        manifest_updates = {}
        for item in self.bindings:
            print(item)
            manifest = item.get('manifest')
            return manifest
            manifest_updates = self.mergeDict(manifest_updates, manifest)

        return manifest_updates


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
