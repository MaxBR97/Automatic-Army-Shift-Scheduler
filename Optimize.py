from flask import Flask, request
from scipy.optimize import minimize
import numpy as np
import ast

app = Flask(__name__)

@app.route('/solve_optimization', methods=['POST'])
def solve_optimization():
    data = request.json

    # Extract optimization parameters (objective, constraints, etc.)
    objective = data['objective']
    constraints = data.get('constraints', [])

    # Define the objective function
    def objective_function(x):
        return eval(objective)

    # Define constraints if applicable

    # Call scipy.optimize.minimize
    result = minimize(objective_function, x0=np.zeros((3, 3, 3)), constraints=constraints)

    return {'solution': result.x.tolist(), 'message': result.message}

if __name__ == '__main__':
    app.run(port=5000) # Run Flask app on port 5000 (or your desired port)
