#include <node_api.h>
#include <vector>
#include <iostream>


//int*** problemVariables = nullptr; // Declare the global variable
namespace demo {
    using namespace std;

    typedef bool (*BoolFunctionPtr)(int***);
    typedef int (*IntFunctionPtr)(int***);
    int*** problemVariables = nullptr;
    int** t = nullptr;
    int** m = nullptr;
    int** k = nullptr;
    int** s = nullptr;
    int** u = nullptr;
    int** historyValues = nullptr; // i * 2
    int j_size=1, k_size=1, i_size=1;
    std::vector<BoolFunctionPtr> constraintLambdas; // = {Function1, Function2};
    std::vector<BoolFunctionPtr> feasibilityOfConstraints;
    IntFunctionPtr * objectiveFunction = nullptr;
    int currentMinValue = 999999999;
    int* currentBestAnswer = nullptr;

    #include <iostream>

void initializeVariables() {
    for (int j = 0; j < j_size; ++j) {
        for (int k = 0; k < dim2; ++k) {
            for (int i = 0; i < dim3; ++i) {
                problemVariables[i][j][k] = 0;
            }
        }
    }
}

void print3DArray(int*** arr, int dim1, int dim2, int dim3) {
    for (int i = 0; i < dim1; ++i) {
        std::cout << "Layer " << i << ":\n";
        for (int j = 0; j < dim2; ++j) {
            for (int k = 0; k < dim3; ++k) {
                std::cout << arr[i][j][k] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

    void print2DArray(int** arr, int dim1, int dim2) {
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                    std::cout << "arr[" << i << "][" << j << "] = "
                            << arr[i][j] << std::endl;
                
            }
        }
    }

    int evaluateObjectiveFunction(int*** arr) {
        int sum = 0;
        for (int i = 0; i < i_size; ++i) {
            int day = 0;
            int night = 0;
            for (int j = 0; j < j_size; ++j) {
            
                if(isNight(j)){
                    for (int k = 0; k < k_size; ++k) {
                        night+= problemVariables[j][k][i];
                    }
                }
                else {
                    for (int k = 0; k < k_size; ++k) {
                        day+= problemVariables[j][k][i];
                    }
                }
            }
            day += historyValues[i][0];
            night += historyValues[i][0];
            day = day*day;
            night = night*night*night;
            sum = sum + day + night;
        }
        return sum;
}
    // deep copy all values of 'toCopy' array to 'subject' array. Assume they are same dimensions.
    void deepCopyArr (int *** toCopy, int *** subject, int dim1, int dim2, int dim3) {

    }

    //TODO
    bool isNight(int j) {
        return true;
    }

    bool checkEnoughSoldiersInSolela() {
        for (int j = 0; j < j_size; ++j) {
                int total = 0;
                for (int i = 0; i < i_size; ++i) {
                    total += problemVariables[j][0][i];
                    
                }
                if(total >= 2 && isNight(j))
                    return false;
                else if(total >= 1)
                    return false;
        }
        return true;
    }

    bool checkEnoughSoldiersInShinGimel() {
        for (int j = 0; j < j_size; ++j) {
                if(!shinGimelNeeded(j))
                    continue;
                int total = 0;
                for (int i = 0; i < i_size; ++i) {
                    total += problemVariables[j][1][i];
                }
                if(total >= 2)
                    return false;
        }
        return true;
    }

    //TODO
    bool checkCommanderDoesntPatrol(int j, int k, int i){
        return true;
    }

    bool soldierIsPresentIfPatrols(int j, int k, int i){
        return true;
    }

    bool soldierDoesntPatrolAtDifferentPlacesSameTime(int j, int k, int i){
        return true;
    }

    void rec(int j, int k, int i, int valueToTry,bool solela, bool shinGimel, bool commander, bool present, bool differentStations) {

        problemVariables[j][k][i] = valueToTry;
        bool cont = false;

        if(!checkCommanderDoesntPatrol(j,k,i)) {
            return;
        }
        if(!soldierIsPresentIfPatrols(j,k,i) ) {
            return;
        }
        if(!soldierDoesntPatrolAtDifferentPlacesSameTime(j,k,i)) {
            return;
        }
        
        if(solela == false && !cont){
            solela = checkEnoughSoldiersInSolela();
            if(solela == false)
                cont = true;
        } 
        
        if(shinGimel == false && !cont) {
            shinGimel = checkEnoughSoldiersInShinGimel();
            if(shinGimel == false)
                cont = true;
        }

        //reached a statisfiable configuration, check for optimality
        if(!cont) {
            int curVal = evaluateObjectiveFunction(problemVariables);
            //optimal
            if(curVal < currentMinValue){
                deepCopyArr(problemVariables, currentBestAnswer,j_size,k_size,i_size);
            }
            
            return;
        }

        


        //recursive step
        if(i+1==i_size){
                if(k+1==k_size){
                    if(j+1==j_size){
                        return;
                    }
                    else {
                        rec(j+1,0,0,0,solela,shinGimel,commander, present, differentStations);
                        rec(j+1,0,0,1,solela,shinGimel,commander, present, differentStations);     
                    }
                }
                else {
                    rec(j,k+1,0,0,solela,shinGimel,commander, present, differentStations);
                    rec(j,k+1,0,1,solela,shinGimel,commander, present, differentStations);
                }
            }
            else {
                rec(j,k,i+1,0,solela,shinGimel,commander, present, differentStations);
                rec(j,k,i+1,1,solela,shinGimel,commander, present, differentStations);
            }
    }

    void optimize() {
        initializeVariables();
        rec(0,0,0,0,false,false,true,true,true);
        rec(0,0,0,0,false,false,true,true,true);
    }


    napi_value SetProblemVariables(napi_env env, napi_callback_info info) {
        napi_status status;

        // Get the number of arguments passed to the function
        size_t argc = 4;
        napi_value args[4];
        status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        if (status != napi_ok || argc < 4) {
            napi_throw_error(env, nullptr, "Expected 4 arguments");
            return nullptr;
        }

        // Extract the array and dimensions from the arguments
        napi_value jsArray = args[0];
        int dim1, dim2, dim3;
        status = napi_get_value_int32(env, args[1], &dim1);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for dimension 1");
            return nullptr;
        }
        status = napi_get_value_int32(env, args[2], &dim2);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for dimension 2");
            return nullptr;
        }
        status = napi_get_value_int32(env, args[3], &dim3);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for dimension 3");
            return nullptr;
        }
        j_size=dim1;
        k_size=dim2;
        i_size=dim3;
        // Check if the argument is an array
        bool is_array;
        status = napi_is_array(env, jsArray, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Argument must be an array");
            return nullptr;
        }

        // Cleanup previous allocation if any
        // Note: You might want to implement cleanupProblemVariables()
        // to deallocate memory when re-assigning problemVariables
        // cleanupProblemVariables();

        // Allocate memory for the problemVariables based on dimensions
        problemVariables = new int**[dim1];
        for (int i = 0; i < dim1; ++i) {
            problemVariables[i] = new int*[dim2];
            for (int j = 0; j < dim2; ++j) {
                problemVariables[i][j] = new int[dim3];
            }
        }

        // Parsing the incoming JavaScript array into problemVariables
        for (int i = 0; i < dim1; ++i) {
            napi_value jsRow;
            status = napi_get_element(env, jsArray, i, &jsRow);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting row from the array");
                return nullptr;
            }

            // Check if the row is an array
            status = napi_is_array(env, jsRow, &is_array);
            if (status != napi_ok || !is_array) {
                napi_throw_error(env, nullptr, "Row must be an array");
                return nullptr;
            }

            for (int j = 0; j < dim2; ++j) {
                napi_value jsInnerRow;
                status = napi_get_element(env, jsRow, j, &jsInnerRow);
                if (status != napi_ok) {
                    napi_throw_error(env, nullptr, "Error getting inner row from the array");
                    return nullptr;
                }

                // Check if the inner row is an array
                status = napi_is_array(env, jsInnerRow, &is_array);
                if (status != napi_ok || !is_array) {
                    napi_throw_error(env, nullptr, "Inner row must be an array");
                    return nullptr;
                }

                for (int k = 0; k < dim3; ++k) {
                    napi_value jsValue;
                    status = napi_get_element(env, jsInnerRow, k, &jsValue);
                    if (status != napi_ok) {
                        napi_throw_error(env, nullptr, "Error getting value from the array");
                        return nullptr;
                    }

                    int value;
                    status = napi_get_value_int32(env, jsValue, &value);
                    if (status != napi_ok) {
                        napi_throw_error(env, nullptr, "Invalid value in the array");
                        return nullptr;
                    }

                    problemVariables[i][j][k] = value;
                }
            }
        }

        return nullptr;
    }

    napi_value SetT(napi_env env, napi_callback_info info) {
    napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 3;
    napi_value args[3];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1, dim2;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[2], &dim2);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 2");
        return nullptr;
    }

    // Check if the argument is an array
    bool is_array;
    status = napi_is_array(env, jsArray, &is_array);
    if (status != napi_ok || !is_array) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return nullptr;
    }

    // Cleanup previous allocation if any
    // Note: You might want to implement cleanupT()
    // to deallocate memory when re-assigning 't'
    // cleanupT();

    // Allocate memory for 't' based on dimensions
    t = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        t[i] = new int[dim2];
    }

    // Parsing the incoming JavaScript array into 't'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }

        // Check if the row is an array
        status = napi_is_array(env, jsRow, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Row must be an array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsValue;
            status = napi_get_element(env, jsRow, j, &jsValue);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting value from the array");
                return nullptr;
            }

            int value;
            status = napi_get_value_int32(env, jsValue, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            t[i][j] = value;
        }
    }

    return nullptr;
}

napi_value SetU(napi_env env, napi_callback_info info) {
    napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 3;
    napi_value args[3];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1, dim2;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[2], &dim2);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 2");
        return nullptr;
    }

    // Check if the argument is an array
    bool is_array;
    status = napi_is_array(env, jsArray, &is_array);
    if (status != napi_ok || !is_array) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return nullptr;
    }

    // Cleanup previous allocation if any
    // Note: You might want to implement cleanupU()
    // to deallocate memory when re-assigning 'u'
    // cleanupU();

    // Allocate memory for 'u' based on dimensions
    u = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        u[i] = new int[dim2];
    }

    // Parsing the incoming JavaScript array into 'u'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }

        // Check if the row is an array
        status = napi_is_array(env, jsRow, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Row must be an array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsValue;
            status = napi_get_element(env, jsRow, j, &jsValue);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting value from the array");
                return nullptr;
            }

            int value;
            status = napi_get_value_int32(env, jsValue, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            u[i][j] = value;
        }
    }

    return nullptr;
}

napi_value SetS(napi_env env, napi_callback_info info) {
    napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 3;
    napi_value args[3];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1, dim2;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[2], &dim2);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 2");
        return nullptr;
    }

    // Check if the argument is an array
    bool is_array;
    status = napi_is_array(env, jsArray, &is_array);
    if (status != napi_ok || !is_array) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return nullptr;
    }

    // Cleanup previous allocation if any
    // Note: You might want to implement cleanupS()
    // to deallocate memory when re-assigning 's'
    // cleanupS();

    // Allocate memory for 's' based on dimensions
    s = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        s[i] = new int[dim2];
    }

    // Parsing the incoming JavaScript array into 's'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }

        // Check if the row is an array
        status = napi_is_array(env, jsRow, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Row must be an array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsValue;
            status = napi_get_element(env, jsRow, j, &jsValue);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting value from the array");
                return nullptr;
            }

            int value;
            status = napi_get_value_int32(env, jsValue, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            s[i][j] = value;
        }
    }

    return nullptr;
}


napi_value SetK(napi_env env, napi_callback_info info) {
    napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 3;
    napi_value args[3];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1, dim2;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[2], &dim2);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 2");
        return nullptr;
    }

    // Check if the argument is an array
    bool is_array;
    status = napi_is_array(env, jsArray, &is_array);
    if (status != napi_ok || !is_array) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return nullptr;
    }

    // Cleanup previous allocation if any
    // Note: You might want to implement cleanupK()
    // to deallocate memory when re-assigning 'k'
    // cleanupK();

    // Allocate memory for 'k' based on dimensions
    k = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        k[i] = new int[dim2];
    }

    // Parsing the incoming JavaScript array into 'k'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }

        // Check if the row is an array
        status = napi_is_array(env, jsRow, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Row must be an array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsValue;
            status = napi_get_element(env, jsRow, j, &jsValue);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting value from the array");
                return nullptr;
            }

            int value;
            status = napi_get_value_int32(env, jsValue, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            k[i][j] = value;
        }
    }

    return nullptr;
}

    napi_value SetM(napi_env env, napi_callback_info info) {
    napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 3;
    napi_value args[3];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1, dim2;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[2], &dim2);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 2");
        return nullptr;
    }

    // Check if the argument is an array
    bool is_array;
    status = napi_is_array(env, jsArray, &is_array);
    if (status != napi_ok || !is_array) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return nullptr;
    }

    // Cleanup previous allocation if any
    // Note: You might want to implement cleanupM()
    // to deallocate memory when re-assigning 'm'
    // cleanupM();

    // Allocate memory for 'm' based on dimensions
    m = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        m[i] = new int[dim2];
    }

    // Parsing the incoming JavaScript array into 'm'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }

        // Check if the row is an array
        status = napi_is_array(env, jsRow, &is_array);
        if (status != napi_ok || !is_array) {
            napi_throw_error(env, nullptr, "Row must be an array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsValue;
            status = napi_get_element(env, jsRow, j, &jsValue);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Error getting value from the array");
                return nullptr;
            }

            int value;
            status = napi_get_value_int32(env, jsValue, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            m[i][j] = value;
        }
    }

    return nullptr;
    }



    napi_value AppendToConstraintLambdas(napi_env env, napi_callback_info info) {
        napi_status status;

        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        if (status != napi_ok || argc < 1) {
            napi_throw_error(env, nullptr, "Expected 1 argument");
            return nullptr;
        }
        napi_value jsClosure = args[0];

        // Convert the JavaScript closure to a C++ function pointer (BoolFunctionPtr)
        BoolFunctionPtr fnPtr;
        status = napi_unwrap(env, jsClosure, reinterpret_cast<void**>(&fnPtr));
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to unwrap JavaScript closure!!");
            return nullptr;
        }

        // Append the function pointer to constraintLambdas
        constraintLambdas.push_back(fnPtr);
        return nullptr;
    }

    napi_value AppendToFeasibilityOfConstraints(napi_env env, napi_callback_info info) {
        napi_status status;

        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        if (status != napi_ok || argc < 1) {
            napi_throw_error(env, nullptr, "Expected 1 argument");
            return nullptr;
        }

        napi_value jsClosure = args[0];

        // Convert the JavaScript closure to a C++ function pointer (BoolFunctionPtr)
        BoolFunctionPtr fnPtr;
        status = napi_unwrap(env, jsClosure, reinterpret_cast<void**>(&fnPtr));
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to unwrap JavaScript closure");
            return nullptr;
        }

        // Append the function pointer to feasibilityOfConstraints
        feasibilityOfConstraints.push_back(fnPtr);

        return nullptr;
    }

    napi_value createClosure(napi_env env, napi_callback_info info) {
    napi_status status;

    size_t argc = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument");
        return nullptr;
    }

    napi_value jsFunction = args[0];

    // Create a persistent reference to the JavaScript function
    napi_ref* funcRef = new napi_ref;
    status = napi_create_reference(env, jsFunction, 1, funcRef);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create reference to the function");
        return nullptr;
    }

    // Convert the reference to a napi_value and return it
    napi_value result;
    status = napi_create_external(env, funcRef, nullptr, nullptr, &result);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create closure");
        return nullptr;
    }
   
    return result;
}


    napi_value Init(napi_env env, napi_value exports) {
        napi_status status;

        napi_value fn, fn1, fn2, fn3, ss, sm, st, sk, su;
        status = napi_create_function(env, nullptr, 0, SetProblemVariables, nullptr, &fn);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetK, nullptr, &sk);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetT, nullptr, &st);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetU, nullptr, &su);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetM, nullptr, &sm);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetS, nullptr, &ss);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setProblemVariables", fn);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setM", sm);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setU", su);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setT", st);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setK", sk);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setS", ss);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, AppendToConstraintLambdas, nullptr, &fn1);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, AppendToFeasibilityOfConstraints, nullptr, &fn2);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "appendToConstraintLambdas", fn1);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set appendToConstraintLambdas as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "appendToFeasibilityOfConstraints", fn2);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set appendToFeasibilityOfConstraints as a property on exports");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, createClosure, nullptr, &fn3);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "createClosure", fn3);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        return exports;
    }


    NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
}
