from scipy.optimize import minimize
import numpy as np
import json
import re
from collections import defaultdict
from functools import reduce
from sympy import symbols, sympify
from sympy import lambdify

inputFile = 'input.json'

def solve_optimization():
    # Load data from input.json
    with open(inputFile, 'r') as file:
        data = json.load(file)

    # Extract optimization parameters (objective, constraints, etc.)
    objective = data['objective']
    constraints = data.get('constraints', [])
    
    j_size= 1
    k_size = 1
    i_size = 1

    def extract_dimensions_from_file(file_path):
        with open(file_path, 'r') as file:
            data = json.load(file)

            dimensions = data.get('dimensions', {})
            i = dimensions.get('i', 0)
            j = dimensions.get('j', 0)
            k = dimensions.get('k', 0)

            return i, j, k
    i_size , j_size, k_size = extract_dimensions_from_file(inputFile)
    def calculateReductionIndex(x1,x2,x3):
        return x1*(k_size*i_size) + x2*i_size + x3

    def read_objective_function_from_json(file_path):

        # Read the objective function from the JSON file
        with open(file_path, 'r') as file:
            data = json.load(file)

        # Extract the objective function string from the data
        objective_function_str = data.get('objective', '')
        return objective_function_str
    
    def replace_and_calculate(s):
        pattern = r'z\[(\d+)\]\[(\d+)\]\[(\d+)\]'

        def replace_fn(match):
            extracted_numbers = list(map(int, match.groups()))
            result = calculateReductionIndex(extracted_numbers[0],extracted_numbers[1],extracted_numbers[2])
            return f'z[{result}]'

        replaced_string = re.sub(pattern, replace_fn, s)

        return replaced_string
        
    
    extractedString = read_objective_function_from_json(inputFile)
    formattedString  = replace_and_calculate(extractedString)
    print("Formatterd objective exp: ",formattedString)
    objective = formattedString
    def objective_function(z):
        return eval(objective)
###################### Take care of objective expression ####################
    # Reshape initial guess into a one-dimensional array
    initial_guess = np.array([
        [1, 1, 0],
        [1, 1, 1],
        [0, 1, 0]
    ]).flatten()

    initial_guess = np.zeros((j_size,k_size,i_size), dtype=int).flatten()

    linear_constraints = [
       # {'type': 'eq', 'fun': lambda z: z[26] + z[1] +z[1] - 1},  # Example constraint 1: z[0] + z[1] = 1
        #{'type': 'eq', 'fun': lambda z: z[1] - z[2]},      # Example constraint 2: z[1] = z[2]
        #{'type': 'eq', 'fun': lambda z: z[0] * (1 - z[0])}, # Constraint to enforce z[0] as binary (0 or 1)
        #{'type': 'eq', 'fun': lambda z: z[1] * (1 - z[1])}, # Constraint to enforce z[1] as binary (0 or 1)
    
        # Add more constraints as needed
    ]

    def add_binaryVariables_constraints(linear_constraints, num):
        for index in range(num):
            # example: {'type': 'eq', 'fun': lambda z: z[2] * (1 - z[2])}  # Constraint to enforce z[2] as binary (0 or 1)
            equation = {'type': 'eq', 'fun': lambda z, idx=index: z[idx] * (1 - z[idx])}
            linear_constraints.append(equation)

    add_binaryVariables_constraints(linear_constraints, j_size*k_size*i_size)


    def extract_substring(expression):
        # Define patterns for different operators
        operators = ['<=', '>=', '!=', '==']
        
        # Find the minimum index of operators in the expression
        indices = [expression.find(op) for op in operators if expression.find(op) != -1]
        if indices:
            min_index = min(indices)
            return expression[:min_index].strip()
        else:
            return expression.strip()

    def create_all_constraints_from_file(linear_constraints, string_array):
        substrings = [replace_and_calculate(extract_substring(expr)) for expr in string_array]
        
        for substring in substrings:
            #print("substring: ",substring)
            def constraint_lambda(z):
                return eval(substring)
            #print("use: ",constraint_lambda(initial_guess))
            linear_constraints.append( {'type': 'ineq', 'fun': constraint_lambda})
            
    create_all_constraints_from_file(linear_constraints,constraints)














    def add_expression_constraint(linear_constraints, constraint_expression):
        # Extract individual expressions within parentheses
        sub_expressions = re.findall(r'\([^()]*\)', constraint_expression)
        print("all subExps: ",sub_expressions)

        for sub_expr in sub_expressions:
            print("subExp: ",sub_expr)
            parts = sub_expr[1:-1].split()
            print(parts)
            if len(parts) == 5:  # Only consider expressions with 5 parts like "(z[21] + z[31])*2 != z[31]"
                index1 = int(re.findall(r'\d+', parts[0])[0])
                index2 = int(re.findall(r'\d+', parts[2])[0])
                comparison = parts[3]
                value = int(parts[4])

                if comparison == '<=':
                    equation = {'type': 'ineq', 'fun': lambda z, idx1=index1, idx2=index2, val=value: z[idx1] * z[idx2] - val}
                elif comparison == '>=':
                    equation = {'type': 'ineq', 'fun': lambda z, idx1=index1, idx2=index2, val=value: val - z[idx1] * z[idx2]}
                elif comparison == '==':
                    equation = {'type': 'eq', 'fun': lambda z, idx1=index1, idx2=index2, val=value: z[idx1] * z[idx2] - val}
                else:
                    raise ValueError("Invalid comparison operator")

                linear_constraints.append(equation)

    #add_expression_constraint(linear_constraints,"(z[51] + z[0] ** 2 ) * 3 != z[3]")

    # Call scipy.optimize.minimize with the flattened initial guess and constraints
    linear_constraints.append({'type': 'ineq', 'fun': lambda z, idx1=1, idx2=1, val=1: z[1] * z[1] - 1})
    result = minimize(objective_function, x0=initial_guess, constraints=linear_constraints, tol=1e-6)

    # Reshape the result back to the original shape
    result_array = result.x.reshape((j_size, k_size,i_size))

    # Write the optimization result to result.json
    with open('result.json', 'w') as file:
        json.dump({'solution': result_array.tolist(), 'message': result.message}, file)

if __name__ == '__main__':
    solve_optimization()
