from scipy.optimize import minimize
import numpy as np
import json
import re
from collections import defaultdict
from functools import reduce
from sympy import symbols, sympify
from sympy import lambdify
from pyomo.environ import ConcreteModel, Constraint, Var, Binary
from pyomo.opt import SolverFactory
from pyomo.environ import *
from pyomo.opt import SolverFactory


# Create a Concrete Model
model = ConcreteModel()

inputFile = 'input.json'

def solve_optimization():
    # Load data from input.json
    with open(inputFile, 'r') as file:
        data = json.load(file)

    # Extract optimization parameters (objective, constraints, etc.)
    objective = data['objective']
    constraints = data['constraints']
    
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
        replacingNums = []
        def replace_fn(match):
            extracted_numbers = list(map(int, match.groups()))
            result = calculateReductionIndex(extracted_numbers[0],extracted_numbers[1],extracted_numbers[2])
            replacingNums.append(result)
            return f'z[{result}]'
        
        replaced_string = re.sub(pattern, replace_fn, s)

        return replaced_string, replacingNums
        
    
    extractedString = read_objective_function_from_json(inputFile)
    formattedString  = replace_and_calculate(extractedString)[0]
    print("Formatterd objective exp: ",formattedString)
    model.z = Var(range(j_size*k_size*i_size+1), domain=Binary) 
    objective = formattedString
    def objective_function(z):
        return eval(objective)
    model.obj_expr = Expression(expr=objective_function(model.z))
    model.obj = Objective(expr=model.obj_expr, sense=minimize)

###################### Take care of objective expression ####################
    # Reshape initial guess into a one-dimensional array
    
    initial_guess = np.array([
        [1, 1, 0],
        [1, 1, 1],
        [0, 1, 0]
    ]).flatten()

    initial_guess = np.zeros((j_size,k_size,i_size), dtype=int).flatten()

    linear_constraints = []

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

    model.constraints = ConstraintList()
    
    def constraint_a(model, idx1, idx2):
        return model.z[idx1] + model.z[idx2] - 1 >= 0 

    def create_all_constraints_from_file(linear_constraints, string_array):
        substrings = [replace_and_calculate(expr) for expr in string_array]
        
        for [substring, indexes] in substrings:
            print("substring: ",substring)
            def constraint_lambda(z):
                return eval(substring)
            model.constraints.add(constraint_a(model, indexes[0], indexes[1]))
            #print("use: ",constraint_lambda(initial_guess))

        
    create_all_constraints_from_file(linear_constraints,constraints['a'])













    opt = SolverFactory('bonmin')
    result = opt.solve(model)

    # Print the results
    print("RESULT: ",result)
    # Reshape the result back to the original shape
    result_array = result.x.reshape((j_size, k_size,i_size))

    # Write the optimization result to result.json
    with open('result.json', 'w') as file:
        json.dump({'solution': result_array.tolist(), 'message': result.message}, file)

if __name__ == '__main__':
    solve_optimization()
