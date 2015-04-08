prompt = ":"
line = 0
buffer = []

class Range:
    def __init__(self, start, stop):
        self.start = start
        self.stop = stop
        
class Number:
    def __init__(self, value):
        self.value = value
        
def error(description):
    current_error = description
    print("?")
    print(current_error)
    commands = ""
    
def advance():
    global commands, command
    if len(commands) > 0:
        command = commands[0]
        commands = commands[1:]

def parse_NUMBER():
    number_str = ""
    if command.isdigit():
        while command.isdigit():
            number_str += command
            advance()
        return int(number_str)
    else:
        return False

def parse_range():
    start = parse_rangspec("left")
    if not start:
        return False
    if not command == ",":
        return False
        #error("Unknown command")
    advance()
    end = parse_rangspec("right")
    if not end:
        error("Expected number")
    return Range(start, end)
    
def parse_rangspec(side):
    number = parse_NUMBER()
    if number:
        return Number(number)
    else:
        if side == "left":
            return Number(0)
        else:
            return Number(len(buffer))
            
def get_line():
    return input()
    
def get_lines():
    input_lines = []
    while True:
        input_line = input()
        if input_line == ".":
            break
        else:
            input_lines.append(input_line)
    return input_lines
        
def parse_command():
    range = parse_range()
    if range:
        line = range
        return True
    number = parse_NUMBER()
    if number:
        line = number
        return True
    if command == "p":
        advance()
        if isinstance(line, Range):
            for line in range(line.start, line.end):
                print(buffer[line])
        else:
            print(line)
    elif command == "c":
        line = get_line()
    elif command == "a":
        lines = get_lines()
        print(lines)
        if isinstance(line, Range):
            line = Range.start
        for input_line in lines:
            buffer.insert(line + 1, input_line)
    else:
        error("Unknown command")
    
def parse_commands():
    while len(commands) > 0:
        advance()
        parse_command()
           
while True:
    commands = input(prompt)
    while len(commands) > 0:
        advance()
        parse_command()