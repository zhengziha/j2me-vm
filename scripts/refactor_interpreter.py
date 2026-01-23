import re

file_path = '/Users/zhengzihang/my-src/j2me-vm/src/core/Interpreter.cpp'
with open(file_path, 'r') as f:
    lines = f.readlines()

new_lines = []
in_constructor = False
constructor_found = False

for line in lines:
    if 'Interpreter::Interpreter(j2me::loader::JarLoader& loader)' in line and not constructor_found:
        if '{}' in line:
             line = line.replace('{}', '{ initInstructionTable(); }')
             constructor_found = True
        else:
             pass
    new_lines.append(line)

lines = new_lines
new_lines = []

in_switch = False
switch_start_line = -1
switch_end_line = -1
brace_count = 0
captured_switch = False

for i, line in enumerate(lines):
    if 'switch (opcode) {' in line and not captured_switch:
        in_switch = True
        switch_start_line = i
        brace_count = 1 
        continue
    
    if in_switch:
        brace_count += line.count('{')
        brace_count -= line.count('}')
        
        if brace_count == 0: 
            switch_end_line = i
            in_switch = False
            captured_switch = True
            break

if not captured_switch:
    print("Could not find switch block")
    exit(1)

switch_content = lines[switch_start_line+1 : switch_end_line]

parsed_cases = []
current_opcodes = []
current_body_lines = []
case_regex = re.compile(r'case\s+(OP_\w+):')

for line in switch_content:
    stripped = line.strip()
    match = case_regex.search(line)
    
    if match:
        has_code = any(l.strip() and not l.strip().startswith('//') for l in current_body_lines)
        if has_code:
             parsed_cases.append({'opcodes': current_opcodes, 'body': current_body_lines})
             current_opcodes = []
             current_body_lines = []
        
        current_opcodes.append(match.group(1))
    elif stripped == 'default:':
        if current_body_lines:
             parsed_cases.append({'opcodes': current_opcodes, 'body': current_body_lines})
             current_opcodes = []
             current_body_lines = []
        current_opcodes = ['DEFAULT']
    else:
        if current_opcodes:
            current_body_lines.append(line)

if current_opcodes and current_body_lines:
    parsed_cases.append({'opcodes': current_opcodes, 'body': current_body_lines})

init_code = []
init_code.append("void Interpreter::initInstructionTable() {\n")
init_code.append("    instructionTable.resize(256);\n")
init_code.append("    // Default handler\n")
init_code.append("    for(int i=0; i<256; i++) instructionTable[i] = [](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame>, util::DataReader&, uint8_t opcode) -> bool { \n")
init_code.append("        std::cerr << \"Unknown Opcode: 0x\" << std::hex << (int)opcode << std::dec << std::endl;\n")
init_code.append("        return true;\n")
init_code.append("    };\n\n")

for case in parsed_cases:
    opcodes = case['opcodes']
    body = case['body']
    
    if 'DEFAULT' in opcodes:
        continue

    assignments = " = ".join([f"instructionTable[{op}]" for op in opcodes])
    
    init_code.append(f"    {assignments} = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {{\n")
    init_code.append("        do {\n")
    init_code.extend(body)
    init_code.append("\n        } while(0);\n")
    init_code.append("        return true;\n")
    init_code.append("    };\n\n")

init_code.append("}\n")

final_lines = lines[:switch_start_line]
final_lines.append("    if (instructionTable[opcode]) {\n")
final_lines.append("        return instructionTable[opcode](thread, frame, codeReader, opcode);\n")
final_lines.append("    } else {\n")
final_lines.append("        std::cerr << \"Unknown Opcode: 0x\" << std::hex << (int)opcode << std::dec << std::endl;\n")
final_lines.append("        return true;\n")
final_lines.append("    }\n")
final_lines.extend(lines[switch_end_line+1:])

insert_pos = len(final_lines) - 1
for i in range(len(final_lines)-1, -1, -1):
    if 'namespace core' in final_lines[i]:
        insert_pos = i
        break

final_lines.insert(insert_pos, "".join(init_code))

with open(file_path, 'w') as f:
    f.writelines(final_lines)
