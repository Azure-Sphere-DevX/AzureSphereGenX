import hashlib

# This class will remove unmodified generated functions and associated signatures plus reference variables

preserved_items = []

class Clean():
    def __init__(self, lines):
        self.removed_blocks = []
        self.lines = lines

    def clean_main(self, handlers_block):

        block_found = False
        block_lines = []
        
        current_block = ''
        block_chars = ''
        output_lines = []
        md5_hash = ''

        for line in self.lines:
            if not block_found and line.startswith('/// GENX_BEGIN'):
                id = line.split('ID:')[1].split(' ')[0]
                md5_hash = line.split('MD5:')[1].split('\n')[0]
                block_found = True
                current_block = line[:-1]
                block_lines = []
                block_lines.append(line)
                block_chars = ''
            elif block_found and line.startswith('/// GENX_END'):
                block_found = False;  
                block_lines.append(line)          
                hash_object = hashlib.md5(block_chars[:-1].encode())
                same_hash = True if md5_hash == hash_object.hexdigest() else False
                if not same_hash or handlers_block.get(id) is not None:
                    # if not the same hash then remove the /// GENX_BEGIN GENX_END tags
                    if not same_hash:
                        del block_lines[0]
                        del block_lines[-1]
                    
                    for block_line in block_lines:
                        output_lines.append(block_line)
                    block_lines = []
                    block_chars = ''
                    id = md5_hash = line.split('ID:')[1].split('\n')[0]
                    preserved_items.append(id)    
                else:
                    self.removed_blocks.append(current_block)
            elif block_found:
                block_lines.append(line)
                block_chars += line
            else:
                output_lines.append(line)

        # remove blank lines at end of file
        while output_lines[len(output_lines) - 1] == '\n':
            del output_lines[-1]
        
        # Append a blank line
        output_lines.append('\n')
        
        return output_lines

# lines = []

# filename='../GenX_Generated/main.c'
# with open(filename, 'r') as j:
#     lines = j.readlines()

# clean = Clean(lines)
# cleaned_lines = clean.clean_main()

# with open(filename, 'w') as j:
#     j.writelines(cleaned_lines)

