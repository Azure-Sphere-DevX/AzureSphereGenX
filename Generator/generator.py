import json
import pathlib
import time
import watcher
import hashlib
import cleaner
import os

from builders import device_twin
from builders import direct_methods
from builders import timer_bindings
from builders import gpio_in_bindings
from builders import gpio_out_bindings
from builders import custom_bindings

# declare dictionaries
signatures = {}
timer_block = {}
variables_block = {}
handlers_block = {}
includes_block = {}
templates = {}

device_twins_updates = None
device_twin_variables = None

dt = None

builders = None

binding_variables = {}
code_lines = []

generated_project_path = '../GenX_Generated'


def load_bindings():
    global signatures, timer_block, variables_block, handlers_block, templates, builders, binding_variables, dt, includes_block

    signatures = {}
    timer_block = {}
    variables_block = {}
    handlers_block = {}
    includes_block = {}
    templates = {}
    binding_variables = {}

    time.sleep(0.5)
    with open('app_model.json', 'r') as j:
        data = json.load(j)

    dt = device_twin.Builder(data, signatures=signatures, variables_block=variables_block, handlers_block=handlers_block)
    dm = direct_methods.Builder(data, signatures=signatures, variables_block=variables_block, handlers_block=handlers_block)
    timers = timer_bindings.Builder(data, signatures=signatures, variables_block=variables_block, handlers_block=handlers_block, timer_block=timer_block)
    gpio_input = gpio_in_bindings.Builder(data, signatures=signatures, variables_block=variables_block,
                                    handlers_block=handlers_block, timer_block=timer_block)
    gpio_output = gpio_out_bindings.Builder(data, signatures=signatures, variables_block=variables_block,
                                    handlers_block=handlers_block, timer_block=timer_block)
    general = custom_bindings.Builder(data, signatures=signatures, variables_block=variables_block,
                            handlers_block=handlers_block, timer_block=timer_block, includes_block=includes_block)

    builders = [dt, dm, timers, gpio_input, gpio_output, general]


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


def build_buckets():
    for builder in builders:
        builder.build()


def render_signatures(f):
    for item in sorted(signatures):
        sig = signatures.get(item)
        name = sig.get('name')
        template_key = sig.get('signature_template')
        f.write(templates[template_key].format(name=name))


def render_timer_block(f):
    # write_comment_block(f, templates['comment_block_timer'])
    for item in sorted(timer_block):
        var = timer_block.get(item)

        name = var.get('name')
        properties = var.get('properties')

        binding = 'TIMER_BINDING'
        var_list = binding_variables.get(binding, [])
        var_list.append(name)
        binding_variables.update({binding: var_list})

        if properties is not None:
            period = get_value(properties, 'period', '{0, 0}')
            timer_type = 'DX_PERIODIC' if properties.get('type', 'periodic') == 'periodic' else 'DX_ONESHOT'
            template_key = var.get('timer_template')
            f.write(templates[template_key].format(name=name, period=period, timer_type=timer_type,
                                                   device_twins_updates=device_twins_updates,
                                                   device_twin_variables=device_twin_variables))


def render_variable_block(f):
    device_twin_types = {"integer": "DX_TYPE_INT", "float": "DX_TYPE_FLOAT", "double": "DX_TYPE_DOUBLE",
                            "boolean": "DX_TYPE_BOOL",  "string": "DX_TYPE_STRING"}

    for item in variables_block:
        var = variables_block.get(item)
        binding = var.get('binding')

        name = var.get('name')

        properties = var.get('properties')
        template_key = var.get('variable_template')        

        var_list = binding_variables.get(binding, [])
        var_list.append(name)
        binding_variables.update({binding: var_list})

        pin = get_value(properties, 'pin', 'GX_PIN_NOT_DECLARED_IN_GENX_MODEL')
        initialState = get_value(properties, 'initialState', 'GPIO_Value_Low')
        invert = "true" if get_value(properties, 'invertPin', True) else "false"

        twin_type = get_value(properties, 'type', None)
        twin_type = device_twin_types.get(twin_type, 'DX_TYPE_UNKNOWN')

        detect = get_value(properties, 'detect', 'DX_GPIO_DETECT_LOW')

        f.write(templates[template_key].format(
            name=name, pin=pin,
            initialState=initialState,
            invert=invert,
            detect=detect,
            twin_type=twin_type
        ))

def does_handler_exist(code_lines, handler):
    sig = handler + '_gx_handler'
    for line in code_lines:
        if sig in line:
            return True
    
    return False

def render_handler_block():
    for item in sorted(handlers_block):
        var = handlers_block.get(item)
        binding = var.get('binding')

        # if binding is not None and binding == key_binding:

        name = var.get('name')

        if does_handler_exist(code_lines=code_lines, handler=name):
            continue

        template_key = var.get('handler_template')
        block_chars = templates[template_key].format(name=name, device_twins_updates=device_twins_updates,
                                                device_twin_variables=device_twin_variables)

        hash_object = hashlib.md5(block_chars.encode())

        code_lines.append("\n")
        code_lines.append('/// GENX_BEGIN ID:{name} MD5:{hash}\n'.format(name=name, hash=hash_object.hexdigest()))
        
        code_lines.append(templates[template_key].format(name=name, device_twins_updates=device_twins_updates,
                                                device_twin_variables=device_twin_variables))
        code_lines.append('\n/// GENX_END ID:{name}'.format(name=name))
        code_lines.append("\n\n")


def render_includes_block():
    for item in includes_block:

        block = includes_block.get(item)
        name = block.get('name')
        filename = generated_project_path + '/gx_' + block.get('name') + '.h'

        if os.path.isfile(filename):
            continue

        template = block.get('include_template')

        with open(filename, 'w') as include_file:
            include_file.write(templates.get(template).format(name=name))

    with open(generated_project_path + '/gx_includes.h', 'w') as includes:
        includes.write('#pragma once\n\n')
        for name in includes_block:        
            includes.write('#include "gx_' + name + '.h"\n')

    

def render_bindings(f):
    bindings_tags = json.loads(templates.get('declare_bindings_tag'))
    for tag in bindings_tags:
        variable_list = ''

        var_list = binding_variables.get(tag , [])
        for var in var_list:
            variable_list += '&' + bindings_tags.get(tag) + '_' + var + ', '

        if variable_list != '':
            variable_list = variable_list[:-2]
        f.write(templates.get('declare_bindings_' + tag.lower()).format(variables = variable_list))


def write_main():
    global device_twins_updates, device_twin_variables
    with open(generated_project_path + '/gx_declarations.h', 'w') as df:

        df.write(templates["declarations"])

        render_signatures(df)
        device_twins_updates, device_twin_variables = dt.build_publish_device_twins()

        df.write("\n\n")
        render_timer_block(df)
        render_variable_block(df)
        df.write("\n\n")

        render_bindings(df)

        df.write(templates["declarations_footer"])

        render_handler_block()
        # render_handler_block("DEVICE_TWIN_BINDING")
        # render_handler_block("DIRECT_METHOD_BINDING")
        # render_handler_block("TIMER_BINDING")

        render_includes_block();

        with open(generated_project_path + "/main.c", "w") as mf:
            mf.writelines(code_lines)


def load_main():
    global code_lines
    with open(generated_project_path + "/main.c", "r") as mf:
        code_lines = mf.readlines()

    clean = cleaner.Clean(code_lines)
    code_lines = clean.clean_main(handlers_block)  

def validate_schema():
    pass
    # TODO: app.json schema validation


def process_update():

    load_bindings()
    load_templates()
    build_buckets()
    load_main()
    write_main()

# process_update()

watch_file = 'app_model.json'

watcher = watcher.Watcher(watch_file, process_update)  # also call custom action function
watcher.watch()  # start the watch going