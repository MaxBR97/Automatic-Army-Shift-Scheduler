const addon = require('./build/Release/OptimizationAlgorithmAPI.node');

let z = [[[1,2],[3,4]],[[1,2],[3,5]]]

let lamb = (arr) => {
    return false;
};

let fnPtr = addon.createClosure(lamb); // Hypothetical function in addon to create closure

console.log(addon.appendToConstraintLambdas(fnPtr));

console.log(addon.setProblemVariables(z,2,2,2)); 

