import sys
import math
functions = {}
call_graph = {}

def main():
    print("Hello world")
    su_files = []
    cflow_files = []
    n = len(sys.argv)
    for i in range(1, n):
        if '.su' in sys.argv[i]:
            su_files.append(sys.argv[i])

    for i in range(1, n):
        if '.cflow' in sys.argv[i]:
            cflow_files.append(sys.argv[i])

    parse_su_files(su_files)

    print(functions)

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


def parse_cflow_files(files):
    for file in files:
        f = open(file)
        extract_function(0, f.readlines())

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

def calculate_cost(function):
    cost = 0
    if function in functions:
        cost += int(functions[function])

    for call in call_graph[function]:
        if call in functions and not (call in call_graph):
            cost += int(functions[call])
        if call in call_graph:
            cost += int(calculate_cost(call))
    return cost


def get_level(line):
    try:
        return int(math.floor(line.split(' ').count('')) / 4)

    except ValueError:
        return 0


if __name__ == '__main__':
    main()
