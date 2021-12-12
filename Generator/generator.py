import json
import pathlib
import time

from yaml.events import NodeEvent
import watcher
import hashlib
import cleaner
import os
import yaml
import io

from builders import azure_iot_config
from builders import class_bindings


# declare dictionaries
signatures_block = {}
timer_block = {}
variables_block = {}
handlers_block = {}
includes_block = {}
templates = {}


manifest_updates = {}
device_twins_updates = None
device_twin_variables = None
autostart_oneshot_timer_list = []

dt = None

builders = None

binding_variables = {}
code_lines = []

# generated_project_path = '../GenX_Generated'
generated_project_path = None


def load_bindings():
    global signatures_block, timer_block, variables_block, handlers_block, templates, builders, binding_variables, dt, includes_block, manifest_updates, generated_project_path

    signatures_block = {}
    timer_block = {}
    variables_block = {}
    handlers_block = {}
    includes_block = {}
    templates = {}
    binding_variables = {}
    manifest_updates = {}

    # time.sleep(0.5)
    # with open('app_model.json', 'r') as j:
    #     data = json.load(j)

    with open(r'app_model.yaml', 'r') as file:
        data = yaml.load(file, Loader=yaml.FullLoader)
        print(data)  
        print(data["genx"]["project_path"]) 
        generated_project_path = data["genx"]["project_path"]


    classes = class_bindings.Builder(data, templates=templates, signatures_block=signatures_block, timer_block=timer_block, variables_block=variables_block,
                                     handlers_block=handlers_block, includes_block=includes_block)

    azure_iot = azure_iot_config.Builder(
        data, manifest_updates=manifest_updates)

    with open(generated_project_path + "/app_manifest.json", "r") as f:
        manifest = json.load(f)

    manifest_updates = azure_iot.build(manifest)
    manifest_updates = classes.build(manifest_updates)


def get_value(properties, key, default):
    if properties is None or key is None:
        return default
    return properties.get(key, default)


def write_comment_block(f, msg):
    f.write("\n/****************************************************************************************\n")
    f.write("* {msg}\n".format(msg=msg))
    f.write("****************************************************************************************/\n")


def load_templates():
    for path in pathlib.Path("templates").iterdir():
        if path.is_file():
            template_key = path.name.split(".")[0]
            with open(path, "r") as tf:
                templates.update({template_key: tf.read()})


def render_signatures(f):
    for item in sorted(signatures_block):
        sig = signatures_block.get(item)
        name = sig.get('name')
        template_key = sig.get('signature_template')
        f.write(templates[template_key].format(name=name))


def render_variable_block(f):
    device_twin_types = {"int": "DX_TYPE_INT", "float": "DX_TYPE_FLOAT", "double": "DX_TYPE_DOUBLE",
                         "bool": "DX_TYPE_BOOL",  "string": "DX_TYPE_STRING"}

    bindings_tags = yaml.load(templates.get('declare_bindings_tag'), Loader=yaml.FullLoader)

    for item in variables_block:
        var = variables_block.get(item)

        name = var.get('name')

        properties = var.get('properties')
        variable_template = var.get('variable_template')

        template_key = variable_template.get('declare')
        binding = variable_template.get('binding')

        var_list = binding_variables.get(binding, [])
        var_list.append(name)
        binding_variables.update({binding: var_list})

        pin = get_value(properties, 'pin', 'GX_PIN_NOT_DECLARED_IN_GENX_MODEL')
        initialState = get_value(properties, 'initialState', 'GPIO_Value_Low')
        invert = "true" if get_value(properties, 'invertPin', True) else "false"

        twin_type = get_value(properties, 'type', None)
        twin_type = device_twin_types.get(twin_type, 'DX_TYPE_UNKNOWN')

        detect = get_value(properties, 'detect', 'DX_GPIO_DETECT_LOW')
        period = get_value(properties, 'period', '{ 0, 0 }')
        context = get_value(properties, 'context', None)

        context_name = ""
        if context is not None:
            context_name = context.get('name', '')
            context_type = context.get('type', None)
            if context_type is not None:
                context_name_prefix = bindings_tags.get(context_type, None)
                if context_name_prefix is not None:
                    context_name = context_name_prefix + "_" + context_name
            

        if template_key is None or templates.get(template_key) is None:
            print('Key: "{template_key}" not found'.format(
                template_key=template_key))
            continue
        else:
            f.write(templates[template_key].format(
                name=name, 
                pin=pin,
                initialState=initialState,
                invert=invert,
                detect=detect,
                twin_type=twin_type,
                period=period,
                context_name=context_name
            ))

        if properties is not None and properties.get('type', '').lower() == 'oneshot' and properties.get('autostart', False) == True:
            autostart_oneshot_timer_list.append(name)


def does_handler_exist(code_lines, handler):
    sig = handler + '_gx_handler'
    for line in code_lines:
        if sig in line:
            return True

    return False


def render_handler_block():
    for item in handlers_block:
        var = handlers_block.get(item)
        binding = var.get('binding')
        type = None
        lambda_property = ""
        context_type = ""

        # if binding is not None and binding == key_binding:

        name = var.get('name')
        properties = var.get('properties')
        if properties is not None:
            type = properties.get('type')
            lambda_property = properties.get('lambda', '')
            context = properties.get('context', None)
            if context is not None:
                context_type = context.get('type')
       

        if does_handler_exist(code_lines=code_lines, handler=name):
            continue

        template_key = var.get('handler_template')
        if template_key is None or templates.get(template_key) is None:
            print('Key: "{template_key}" not found'.format(
                template_key=template_key))
            continue
        else:
            try:
                block_chars = templates.get(template_key).format(name=name, device_twins_updates=device_twins_updates,
                                                                 device_twin_variables=device_twin_variables, type=type, 
                                                                 context_type=context_type, lambda_code=lambda_property)
            except:
                print('ERROR: Problem formatting template "{template_key}". Check regular brackets in the template are escaped as {{{{.'.format(
                    template_key=template_key))
                continue

        hash_object = hashlib.md5(block_chars.encode())

        code_lines.append("\n")
        code_lines.append(
            '/// GENX_BEGIN ID:{name} MD5:{hash}\n'.format(name=name, hash=hash_object.hexdigest()))

        code_lines.append(templates[template_key].format(name=name, device_twins_updates=device_twins_updates,
                                                         device_twin_variables=device_twin_variables, type=type, 
                                                         context_type=context_type, lambda_code=lambda_property))
        code_lines.append('\n/// GENX_END ID:{name}'.format(name=name))
        code_lines.append("\n\n")


def render_includes_block():

    if not os.path.isdir(generated_project_path + '/gx_includes'):
        os.mkdir(generated_project_path + '/gx_includes')

    for item in includes_block:

        block = includes_block.get(item)
        name = block.get('name')
        filename = generated_project_path + \
            '/gx_includes/gx_' + block.get('name') + '.h'

        if os.path.isfile(filename):
            continue

        template_key = block.get('include_template')

        if template_key is None or templates.get(template_key) is None:
            print('Key: "{template_key}" not found'.format(
                template_key=template_key))
            continue
        else:
            with open(filename, 'w') as include_file:
                include_file.write(templates.get(
                    template_key).format(name=name))

    with open(generated_project_path + '/gx_includes/gx_includes.h', 'w') as includes:
        includes.write('#pragma once\n\n')
        for name in includes_block:
            includes.write('#include "gx_' + name + '.h"\n')


def render_bindings(f):
    # bindings_tags = json.loads(templates.get('declare_bindings_tag'))
    bindings_tags = yaml.load(templates.get('declare_bindings_tag'), Loader=yaml.FullLoader)
    for tag in bindings_tags:
        variable_list = ''

        var_list = binding_variables.get(tag, [])
        for var in var_list:
            variable_list += '&' + bindings_tags.get(tag) + '_' + var + ', '

        if variable_list != '':
            variable_list = variable_list[:-2]
        f.write(templates.get('declare_bindings_' +
                tag.lower()).format(variables=variable_list))


def render_autostart_timers(f):
    autostart_timer_list = ''
    for autostart_timer in autostart_oneshot_timer_list:
        autostart_timer_list += '&' + 'tmr_' + autostart_timer + ', '

    if autostart_timer_list != '':
        autostart_timer_list = autostart_timer_list[:-2]
    f.write(templates.get('declare_bindings_dx_timer_autostart_oneshot').format(
        autostart_timer_list=autostart_timer_list))


def write_main():
    global device_twins_updates, device_twin_variables

    if not os.path.isdir(generated_project_path + '/gx_includes'):
        os.mkdir(generated_project_path + '/gx_includes')

    with open(generated_project_path + '/main.h', 'w') as df:

        df.write(templates["declarations"])

        render_signatures(df)
        # device_twins_updates, device_twin_variables = dt.build_publish_device_twins()

        df.write("\n")
        write_comment_block(df, 'Binding declarations')
        # render_timer_block(df)
        render_variable_block(df)
        df.write("\n")

        render_bindings(df)
        render_autostart_timers(df)

        df.write(templates["declarations_footer"])

        render_handler_block()

        render_includes_block()

        with open(generated_project_path + "/main.c", "w") as mf:
            mf.writelines(code_lines)


def update_manifest():
    with open(generated_project_path + "/app_manifest.json", "w") as f:
        json.dump(manifest_updates, f, indent=2)


def load_main():
    global code_lines
    with open(generated_project_path + "/main.c", "r") as mf:
        code_lines = mf.readlines()

    clean = cleaner.Clean(code_lines)
    code_lines = clean.clean_main(handlers_block)


def validate_schema():
    pass
    # TODO: app.json schema validation


def init_stuff():
    global autostart_oneshot_timer_list

    autostart_oneshot_timer_list = []


def process_update():
    init_stuff()
    load_bindings()
    load_templates()
    load_main()
    write_main()
    update_manifest()


# process_update()


watch_file = 'app_model.yaml'

# also call custom action function
watcher = watcher.Watcher(watch_file, process_update)
watcher.watch()  # start the watch going
