import sys
import math
import subprocess
import os
functions = {}
call_graph = {}
banned_functions = []
recursive_calls = {}


def main():
    print("Hello world")
    su_files = []
    cflow_files = []
    c_files = []
    n = len(sys.argv)
    for i in range(1, n):
        if '.su' in sys.argv[i]:
            su_files.append(sys.argv[i])

    for i in range(1, n):
        if '.c' in sys.argv[i]:
            c_files.append(sys.argv[i])

    parse_su_files(su_files)

    cflow_files = create_cflow_files(c_files)

    parse_cflow_files(cflow_files)

    for function in call_graph:
        cost = calculate_cost(function)
        print("Function: "+function + ", cost: {cost}".format(cost=cost))


def parse_su_files(files):
    for file in files:
        f = open(file)
        for line in f.readlines():
            func_name = line.split(":")[3].split('\t')[0]
            stack_usage = line.split(":")[3].split('\t')[1]
            functions[func_name+'()'] = stack_usage
        f.close()

def create_cflow_files(c_files):
    cflow_files = []
    if(not os.path.exists("./cflow")):
        os.mkdirs("./cflow")

    for file in c_files:
        output = subprocess.check_output(["cflow", file]).decode()
        split_on_slash = file.split('/')
        cflow_filename = "./cflow/" + split_on_slash[len(split_on_slash)-1].split('.c')[0] + ".cflow"

        f = open(cflow_filename, "w")
        f.write(output)
        f.close()
        cflow_files.append(cflow_filename)

    return cflow_files


def parse_cflow_files(files):
    for file in files:
        f = open(file)
        index = 0
        max = sum(1 for _ in f)
        f.close()
        f = open(file)
        lines = f.readlines()
        f.close()
        while (index < max):
            index = extract_function(index, lines)
            print("spin")

    print(call_graph)


def extract_function(start, lines):
    start_level = get_level(lines[start])
    function_name = ""
    if start_level == 0:
        function_name = lines[start].split(' ')[0]
    else:
        function_name = lines[start].split(' ')[start_level * 4]
    calls = []
    index = start + 1

    while index < len(lines):
        line = lines[index]
        level = get_level(line)
        if level == start_level:
            call_graph[function_name] = calls
            return index
        if ' at ' in line:
            index = extract_function(index, lines) - 1
        calls.append(line.split(' ')[level*4].split('\n')[0])
        index += 1
    call_graph[function_name] = calls
    return index

def calculate_cost(function):
    cost = 0

    if function in recursive_calls:
        recursive_calls[function] += 1
        if recursive_calls[function] > 200:
            if function in functions:
                return int(functions[function])
            else:
                return 0
    else:
        recursive_calls[function] = 1

    if function in banned_functions:
        if function in functions:
            return int(functions[function])
        else:
            return 0

    if function in functions:
        cost += int(functions[function])

    for call in call_graph[function]:
        if call in functions and not (call in call_graph):
            cost += int(functions[call])
        if call in call_graph:
            try:
                cost += int(calculate_cost(call))
            except RecursionError:
                banned_functions.append(call)
                if call in functions:
                    return int(functions[call])
                else:
                    return 0

    return cost


def get_level(line):
    try:
        return int(math.floor(line.split(' ').count('')) / 4)

    except ValueError:
        return 0


if __name__ == '__main__':
    main()
