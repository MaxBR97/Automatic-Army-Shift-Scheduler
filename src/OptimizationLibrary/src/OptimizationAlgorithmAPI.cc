#include <node_api.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <cstring>


//int*** problemVariables = nullptr; // Declare the global variable
namespace demo {
    using namespace std;

    typedef bool (*BoolFunctionPtr)(int***);
    typedef int (*IntFunctionPtr)(int***);
    int*** problemVariables = nullptr;
    int* t = nullptr;
    int** m = nullptr;
    int** k = nullptr;
    int** s = nullptr;
    int** u = nullptr;
    int** historyValues = nullptr; // i * 2
    bool* nightShifts = nullptr;
    int* shinGimelTimes = nullptr;
    int j_size=1, k_size=1, i_size=1;
    std::vector<BoolFunctionPtr> constraintLambdas; // = {Function1, Function2};
    std::vector<BoolFunctionPtr> feasibilityOfConstraints;
    IntFunctionPtr * objectiveFunction = nullptr;
    int currentMinValue = 999999999;
    int*** currentBestAnswer = nullptr;
    long sumOfRecursiveCalls = 0;
    int quantizeProgression = 10; //

    #include <iostream>

void delete3DArray(int ***arr, int dim1, int dim2, int dim3) {
    if(arr != nullptr) {
    for (int i = 0; i < dim1; ++i) {
        for (int j = 0; j < dim2; ++j) {
            delete[] arr[i][j];
        }
        delete[] arr[i];
    }
    delete[] arr;
    }
}


void initializeVariables() {
    delete3DArray(currentBestAnswer,j_size,k_size,i_size);
    delete3DArray(problemVariables, j_size, k_size, i_size);

    currentBestAnswer = new int**[j_size];
    for (int j = 0; j < j_size; ++j) {
        currentBestAnswer[j] = new int*[k_size];
        for (int k = 0; k < k_size; ++k) {
            currentBestAnswer[j][k] = new int[i_size];
            for (int i = 0; i < i_size; ++i) {
                //cout<< " j_size: " << j << " k_size: " << k<< " i_size: " <<i<<endl;
                currentBestAnswer[j][k][i] = -1;
            }
        }
    }

    problemVariables = new int**[j_size];
    for (int j = 0; j < j_size; ++j) {
        problemVariables[j] = new int*[k_size];
        for (int k = 0; k < k_size; ++k) {
            problemVariables[j][k] = new int[i_size];
            for (int i = 0; i < i_size; ++i) {
                //cout<< " j_size: " << j << " k_size: " << k<< " i_size: " <<i<<endl;
                problemVariables[j][k][i] = 0;
            }
        }
    }
}

void print3DArray(int*** arr, int dim1, int dim2, int dim3) {
    for (int i = 0; i < dim1; ++i) {
        for (int j = 0; j < dim2; ++j) {
            std::cout << "Layer " << i << "," << j << ":\n";
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
    void print1DArray(int* arr, int dim1) {
        for (int i = 0; i < dim1; ++i) {
                    std::cout << "arr[" << i << "] = "
                            << arr[i] << std::endl;    
        }
    }
    
    bool isNight(int j) {
        return nightShifts[j];
    }

    int evaluateObjectiveFunction(int*** arr) {
        //print3DArray(problemVariables,j_size,k_size,i_size);
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
            night += historyValues[i][1];
            day = day*day;
            night = night*night*night;
            int soldierSum = day+night;
            sum = sum + soldierSum;
        }
        return sum;
}
    // deep copy all values of 'toCopy' array to 'subject' array. Assume they are same dimensions.
    void deepCopyArr(int ***toCopy, int ***subject, int dim1, int dim2, int dim3) {
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                for (int k = 0; k < dim3; ++k) {
                    subject[i][j][k] = toCopy[i][j][k];
                }
            }
        }
    }


    bool checkEnoughSoldiersInSolela() {
        for (int j = 0; j < j_size; ++j) {
                int total = 0;
                for (int i = 0; i < i_size; ++i) {
                    total += problemVariables[j][0][i];
                }
                if(total < 2 && isNight(j))
                    return false;
                else if(total < 1 && !isNight(j))
                    return false;
        }
        return true;
    }

    bool isShinGimelNeeded(int j) {
        return shinGimelTimes[j] == 2;
    }

    bool checkEnoughSoldiersInShinGimel() {
        for (int j = 0; j < j_size; ++j) {
                if(!isShinGimelNeeded(j))
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
        if(sumOfRecursiveCalls % 100000000 == 0)
            cout<<"recursive calls: "<<sumOfRecursiveCalls <<endl;
        sumOfRecursiveCalls ++;
        //cout << " j: " << j << " k: " << k << " i: " << i <<endl;
        //print3DArray(problemVariables,j_size,k_size,i_size);
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
                        //cout<<"reached the end"<<endl;
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

            return;
    }

    void optimize() {
        //j_size = 2;
        cout<< " j_size: " << j_size << " k_size: " << k_size<< " i_size: " <<i_size<<endl;
        initializeVariables();
        
        rec(0,0,0,0,false,false,true,true,true);
        cout << "50% done" << endl;
        rec(0,0,0,1,false,false,true,true,true);
    }

    //recieves 3 dimensions (int) of variables
    napi_value SetProblemVariables(napi_env env, napi_callback_info info) {
        napi_status status;

        size_t argc = 3;
        napi_value args[3];
        status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        if (status != napi_ok || argc < 3) {
            napi_throw_error(env, nullptr, "Expected 3 arguments");
            return nullptr;
        }

        int32_t j_, k_, i_;

        status = napi_get_value_int32(env, args[0], &j_);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for j_size");
            return nullptr;
        }

        status = napi_get_value_int32(env, args[1], &k_);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for k_size");
            return nullptr;
        }

        status = napi_get_value_int32(env, args[2], &i_);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Invalid value for i_size");
            return nullptr;
        }

        // Set the received values to global variables j_size, k_size, i_size
        // Replace the names of the global variables with your actual variable names
        j_size = j_;
        k_size = k_;
        i_size = i_;

        return nullptr;
}

     // recieves 1d array and its size  
    
    napi_value SetT(napi_env env, napi_callback_info info) {
        napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 2;
    napi_value args[2];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
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
    t = new int[dim1];
    for (int i = 0; i < dim1; ++i) {
        t[i] = 0;
    }

    // Parsing the incoming JavaScript array into 'm'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }
       
        //     napi_value jsValue;
        //     status = napi_get_element(env, jsRow, i, &jsValue);
        //     if (status != napi_ok) {
        //         napi_throw_error(env, nullptr, "Error getting value from the array");
        //         return nullptr;
        //     }

            int value;
            status = napi_get_value_int32(env, jsRow, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            t[i] = value;
        
    }
    print1DArray(t,dim1);
    return nullptr;

}
    // recieves 2D array, and its 2 dimensions
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
    //recieves 2D array and its 2 dimensions
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
    // Note: You might want to implement cleanupM()
    // to deallocate memory when re-assigning 'm'
    // cleanupM();

    // Allocate memory for 'm' based on dimensions
    s = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        s[i] = new int[dim2];
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

            s[i][j] = value;
        }
    }
    print2DArray(s,dim1,dim2);
    return nullptr;
    
    }

    //recieves 2D array and its 2 dimensions 
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
    // Note: You might want to implement cleanupM()
    // to deallocate memory when re-assigning 'm'
    // cleanupM();

    // Allocate memory for 'm' based on dimensions
    k = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        k[i] = new int[dim2];
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

            k[i][j] = value;
        }
    }
    print2DArray(k,dim1,dim2);
    return nullptr;
}
    //recieves 2D array and its 2 dimensions
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
    print2DArray(m,dim1,dim2);
    return nullptr;
    }

    //receives 1D boolean array and its dimension
   napi_value SetNightShifts(napi_env env, napi_callback_info info) {
       napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 2;
    napi_value args[2];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
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
    nightShifts = new bool[dim1];
    for (int i = 0; i < dim1; ++i) {
        nightShifts[i] = 0;
    }

    // Parsing the incoming JavaScript array into 'm'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }
       
        //     napi_value jsValue;
        //     status = napi_get_element(env, jsRow, i, &jsValue);
        //     if (status != napi_ok) {
        //         napi_throw_error(env, nullptr, "Error getting value from the array");
        //         return nullptr;
        //     }

            bool value;
            status = napi_get_value_bool(env, jsRow, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            nightShifts[i] = value;
        
    }
    for(int as=0; as<dim1; as++){
        cout<< nightShifts[as] << endl;
    }
    return nullptr;
}
    
    //recieves a 1D integers array and its dimension. each number means the number of required shin gimel soldiers required
    napi_value SetShinGimelTimes(napi_env env, napi_callback_info info) {
        napi_status status;

    // Get the number of arguments passed to the function
    size_t argc = 2;
    napi_value args[2];
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments");
        return nullptr;
    }

    // Extract the array and dimensions from the arguments
    napi_value jsArray = args[0];
    int dim1;
    status = napi_get_value_int32(env, args[1], &dim1);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid value for dimension 1");
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
    shinGimelTimes = new int[dim1];
    for (int i = 0; i < dim1; ++i) {
        shinGimelTimes[i] = 0;
    }

    // Parsing the incoming JavaScript array into 'm'
    for (int i = 0; i < dim1; ++i) {
        napi_value jsRow;
        status = napi_get_element(env, jsArray, i, &jsRow);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Error getting row from the array");
            return nullptr;
        }
       
        //     napi_value jsValue;
        //     status = napi_get_element(env, jsRow, i, &jsValue);
        //     if (status != napi_ok) {
        //         napi_throw_error(env, nullptr, "Error getting value from the array");
        //         return nullptr;
        //     }

            int value;
            status = napi_get_value_int32(env, jsRow, &value);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Invalid value in the array");
                return nullptr;
            }

            shinGimelTimes[i] = value;
        
    }
    print1DArray(shinGimelTimes,dim1);
    return nullptr;
}

    //receives a 2D integers array and its dimensions.
    napi_value SetHistory(napi_env env, napi_callback_info info) {
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
    historyValues = new int*[dim1];
    for (int i = 0; i < dim1; ++i) {
        historyValues[i] = new int[dim2];
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

            historyValues[i][j] = value;
        }
    }
    print2DArray(historyValues,dim1,dim2);
    return nullptr;
    }

// returns a 3D numbers array
    napi_value Solve(napi_env env, napi_callback_info info) {
    napi_status status;

    // Assuming dimensions of the problemVariables array are known
    // Replace these values with actual dimensions
    int dim1 = j_size;
    int dim2 = k_size;
    int dim3 = i_size;
    cout<<"starting optimization process" << endl;
    optimize();
    cout << "finished optimizing!" << endl;

    // Create a JavaScript 3D array to hold the problemVariables data
    napi_value jsArray;
    status = napi_create_array_with_length(env, dim1, &jsArray);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create JavaScript array");
        return nullptr;
    }

    for (int i = 0; i < dim1; ++i) {
        napi_value jsInnerArray;
        status = napi_create_array_with_length(env, dim2, &jsInnerArray);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create JavaScript inner array");
            return nullptr;
        }

        for (int j = 0; j < dim2; ++j) {
            napi_value jsInnerInnerArray;
            status = napi_create_array_with_length(env, dim3, &jsInnerInnerArray);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Failed to create JavaScript inner inner array");
                return nullptr;
            }

            for (int k = 0; k < dim3; ++k) {
                napi_value jsValue;
                status = napi_create_int32(env, problemVariables[i][j][k], &jsValue);
                if (status != napi_ok) {
                    napi_throw_error(env, nullptr, "Failed to create JavaScript integer");
                    return nullptr;
                }

                status = napi_set_element(env, jsInnerInnerArray, k, jsValue);
                if (status != napi_ok) {
                    napi_throw_error(env, nullptr, "Failed to set JavaScript element");
                    return nullptr;
                }
            }

            status = napi_set_element(env, jsInnerArray, j, jsInnerInnerArray);
            if (status != napi_ok) {
                napi_throw_error(env, nullptr, "Failed to set JavaScript inner element");
                return nullptr;
            }
        }

        status = napi_set_element(env, jsArray, i, jsInnerArray);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set JavaScript element");
            return nullptr;
        }
    }

    return jsArray;
}

    napi_value Init(napi_env env, napi_value exports) {
        napi_status status;

        napi_value spv, sns, sh, sst, ss, sm, st, sk, su, s;
        status = napi_create_function(env, nullptr, 0, SetProblemVariables, nullptr, &spv);
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

        status = napi_create_function(env, nullptr, 0, SetNightShifts, nullptr, &sns);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetHistory, nullptr, &sh);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, Solve, nullptr, &s);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }

        status = napi_create_function(env, nullptr, 0, SetShinGimelTimes, nullptr, &sst);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }


        status = napi_set_named_property(env, exports, "setProblemVariables", spv);
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

        status = napi_set_named_property(env, exports, "setNightShifts", sns);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setHistory", sh);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "setShinGimelTimes", sst);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "solve", s);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        return exports;
    }

    NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
}
