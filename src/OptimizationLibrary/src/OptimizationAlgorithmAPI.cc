#include <node_api.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <cstring>
#include <chrono>


//int*** problemVariables = nullptr; // Declare the global variable
namespace demo {
    using namespace std;

    typedef bool (*BoolFunctionPtr)(int***);
    typedef int (*IntFunctionPtr)(int***);
    int*** problemVariables = nullptr;
    int* t = nullptr; // crew number
    int** m = nullptr; // is present
    int** k = nullptr; // is commander
    int** s = nullptr; //is MT
    int** u = nullptr; //is readinessWithDawn
    int** historyValues = nullptr; // i * 2
    bool* nightShifts = nullptr;
    int* shinGimelTimes = nullptr;
    int j_size=1, k_size=1, i_size=1;
    std::vector<BoolFunctionPtr> constraintLambdas; // = {Function1, Function2};
    std::vector<BoolFunctionPtr> feasibilityOfConstraints;
    IntFunctionPtr * objectiveFunction = nullptr;
    int currentMinValue = 999999999;
    int*** currentBestAnswer = nullptr;
    int** accumulationForObjectiveFunction = nullptr;
    int* accumulationForSolela = nullptr;
    int* accumulationForShinGimel = nullptr;
    long sumOfRecursiveCalls = 0;
    int countWhatever = 0;
    long maxRecursions = 1000000000000;


    int minShiftsBreak = 4;

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


void initializeCalculation() {
    sumOfRecursiveCalls = 0;
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

    accumulationForObjectiveFunction = new int*[i_size];
    for (int i = 0; i < i_size; ++i) {
        accumulationForObjectiveFunction[i] = new int[2];
        for (int time = 0; time < 2; ++time) {
                accumulationForObjectiveFunction[i][time] = historyValues[i][time];
             }
    }
    
    accumulationForSolela = new int[j_size];
    for (int j = 0; j < j_size; ++j) {
        accumulationForSolela[j] = 0;
    }

    accumulationForShinGimel = new int[j_size];
    for (int j = 0; j < j_size; ++j) {
        accumulationForShinGimel[j] = 0;
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

    int calculateDifferenceInObjectiveFunction(int j, int k, int i) {
        if(problemVariables[j][k][i] == 1){
            int curVal = 0;
            int updatedVal = 0;
            int hold = 0;
            if(isNight(j)) {
                hold = accumulationForObjectiveFunction[i][1];
                curVal = hold * hold * hold;
                updatedVal = (hold + 1) * (hold + 1) * (hold + 1);
            }
            else {
                hold = accumulationForObjectiveFunction[i][0];
                curVal = hold * hold;
                updatedVal = (hold + 1) * (hold + 1) ;
            }
            return (updatedVal - curVal);
        }
        return 0;
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
                        night+= arr[j][k][i];
                    }
                }
                else {
                    for (int k = 0; k < k_size; ++k) {
                        day+= arr[j][k][i];
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

 int calculateDifferenceInObjectiveFunction2(int j, int k, int i) {
        if(problemVariables[j][k][i] == 1){
            int curVal = 0;
            int updatedVal = 0;
            int hold = 0;
            int spaceFromLastPatrol = 0;
            for(int dim1 = j-1; dim1>=0; dim1 --) {
                bool flag = false;
                for(int dim2 = 0; dim2<k_size; dim2++) {
                    if(problemVariables[dim1][dim2][i] == 1) {
                        spaceFromLastPatrol = j-dim1;
                        flag = true;
                        break;
                    }
                }
                if(flag)
                    break;
            }
            if(isNight(j)) {
                hold = accumulationForObjectiveFunction[i][1];
                curVal = hold * hold * hold * hold;
                updatedVal = (hold + 1) * (hold + 1) * (hold + 1) * (hold + 1);
            }
            else {
                hold = accumulationForObjectiveFunction[i][0];
                curVal = hold * hold;
                updatedVal = (hold + 1) * (hold + 1) ;
            }
            updatedVal += spaceFromLastPatrol * spaceFromLastPatrol * spaceFromLastPatrol;
            return (updatedVal - curVal);
        }
        return 0;
    }

    int evaluateObjectiveFunction2(int*** arr) {
        //print3DArray(problemVariables,j_size,k_size,i_size);
        int sum = 0;
        for (int i = 0; i < i_size; ++i) {
            int day = 0;
            int night = 0;
            int spaceFromLastPatrolSum = 0;
            for (int j = 0; j < j_size; ++j) {
                if(isNight(j)){
                    for (int k = 0; k < k_size; ++k) {
                        night+= arr[j][k][i];
                    }
                }
                else {
                    for (int k = 0; k < k_size; ++k) {
                        day+= arr[j][k][i];
                    }
                }
                if(arr[j][0][i] == 1 || arr[j][1][i] == 1) {
                    for(int dim1 = j-1; dim1>=0; dim1 --) {
                        bool flag = false;
                        for(int dim2 = 0; dim2<k_size; dim2++) {
                            if(problemVariables[dim1][dim2][i] == 1) {
                                spaceFromLastPatrolSum += (j-dim1) * (j-dim1) * (j-dim1);
                                flag = true;
                                break;
                            }
                        }
                        if(flag)
                            break;
                    }
                }
            }
            day += historyValues[i][0];
            night += historyValues[i][1];
            day = day*day;
            night = night*night*night*night;
            int soldierSum = day + night + spaceFromLastPatrolSum;
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


    bool checkEnoughSoldiersInSolela(int j, int k, int i) {
        // for (int j = 0; j < j_size; ++j) {
        //         int total = 0;
        //         for (int i = 0; i < i_size; ++i) {
        //             total += problemVariables[j][0][i];
        //         }
        //         if(total < 2 && isNight(j))
        //             return false;
        //         else if(total < 1 && !isNight(j))
        //             return false;
        // }
        // return true;
        bool flag = problemVariables[j][k][i] == 1 && k == 0;
        if(flag)
            accumulationForSolela[j] ++;
        for(int dim1 = 0; dim1<j_size; dim1++) {
            if(accumulationForSolela[dim1] < 2 && isNight(dim1)){
                if(flag)
                    accumulationForSolela[j] --;
                return false;
            }
            if(accumulationForSolela[dim1] < 1 && !isNight(dim1))
            {
                if(flag)
                    accumulationForSolela[j] --;
                return false;
            }
        }
        if(flag)
            accumulationForSolela[j] --;
        return true;

    }

    bool isShinGimelNeeded(int j) {
        return shinGimelTimes[j] == 2;
    }

    bool checkEnoughSoldiersInShinGimel(int j, int k, int i) {
        // for (int j = 0; j < j_size; ++j) {
        //         if(!isShinGimelNeeded(j))
        //             continue;
        //         int total = 0;
        //         for (int i = 0; i < i_size; ++i) {
        //             total += problemVariables[j][1][i];
        //         }
        //         if(total < 2)
        //             return false;
        // }
        // return true;
        bool flag = problemVariables[j][k][i] == 1 && k == 1;
        if(flag)
            accumulationForShinGimel[j] ++;
        for (int dim1=0; dim1<j_size; dim1++) {
            if(accumulationForShinGimel[dim1] < shinGimelTimes[dim1]){
                if(flag)
                    accumulationForShinGimel[j] --;
                return false;
            }
        }
        if(flag)
            accumulationForShinGimel[j] --;
        return true;
    }

    //TODO
    bool checkCommanderDoesntPatrol(int j, int a, int i){ // changed 'k' to 'a' because k is also a global variable of array
        if(k[i][j] == 1 && problemVariables[j][a][i] == 1)
            return false;
        return true;
    }

    bool soldierIsPresentIfPatrols(int j, int k, int i){
        if(m[i][j] == 0 && problemVariables[j][k][i] == 1)
            return false;
        return true;
    }

    bool soldierDoesntPatrolAtDifferentPlacesSameTime(int j, int k, int i){
        if(problemVariables[j][k][i] == 1)
        {
            for(int dim1=0; dim1< k_size; dim1 ++) {
                if(dim1 != k && problemVariables[j][dim1][i] == 1)
                    return false;
            }
        }
        return true;
    }

    bool minBreakToSoldier (int j, int k, int i) {
        for(int dim1 = j-1; dim1 >= j-minShiftsBreak; dim1--) {
            if(dim1 < 0)
                break;
            for(int dim2=0; dim2<k_size; dim2++) {
                if(problemVariables[dim1][dim2][i] == 1)
                    return false;
            }
        }
        return true;
    }

    bool soldiersFromDifferentCrewsInShinGimel(int j, int k, int i) {
        if(k == 1 && problemVariables[j][1][i] == 1){
            int crewNumber = t[i];
            for(int dim1 = 0; dim1< i_size; dim1++) {
                if(t[dim1] == crewNumber && dim1 != i){
                    if(problemVariables[j][1][dim1] == 1){
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool notTooManySolelaPatrolers(int j,int k,int i) {
        if(k==0 && problemVariables[j][k][i] == 1){
            if(isNight(j) && (accumulationForSolela[j] + 1 > 2)) {
                    return false;
            }
            else if(!isNight(j) && accumulationForSolela[j] + 1 > 1)
                return false;
        }
        return true;
    }

     bool notTooManyShinGimelPatrolers(int j,int k,int i) {
        if(k==1)
            { 
               if(accumulationForShinGimel[j] + 1 > shinGimelTimes[j])
                return false;
            }
            return true;
    }

    void updateAccumulation(int j, int k, int i, int x) {
        if (isNight(j)) {
            accumulationForObjectiveFunction[i][1] += x;
        }
        else
            accumulationForObjectiveFunction[i][0] += x;
        if(x == 1) {
            if(k==0)
                accumulationForSolela[j] += 1;
            else if(k==1)
                accumulationForShinGimel[j] +=1;
        }
        if(x == -1) {
            if(k==0)
                accumulationForSolela[j] -= 1;
            else if(k==1)
                accumulationForShinGimel[j] -=1;
        }
    }
    
    void updatePatrolersAccumulation(int j, int k, int i, int x) {
        if(x == 1) {
            if(k==0)
                accumulationForSolela[j] += 1;
            else if(k==1)
                accumulationForShinGimel[j] +=1;
        }
        if(x == -1) {
            if(k==0)
                accumulationForSolela[j] -= 1;
            else if(k==1)
                accumulationForShinGimel[j] -=1;
        }
    }

    void rec(int j, int k, int i,bool solela, bool shinGimel, int acc) {
        if(problemVariables[0][0][0] == 0)
            cout<<"It is 0"<<endl;    
        if(sumOfRecursiveCalls % 100000000 == 0)
            cout<<"recursive calls: "<<sumOfRecursiveCalls <<endl;
        if(sumOfRecursiveCalls > maxRecursions)
           return;
        sumOfRecursiveCalls ++;
        //cout << " j: " << j << " k: " << k << " i: " << i <<endl;
        //print3DArray(problemVariables,j_size,k_size,i_size);
        problemVariables[j][k][i] = 1;

        bool cont = false;
        if(!cont && !notTooManySolelaPatrolers(j,k,i)) {
            problemVariables[j][k][i] = 0;
            i = i_size - 1;
            cont = true;
        }
        if(!cont && !notTooManyShinGimelPatrolers(j,k,i)) {
            problemVariables[j][k][i] = 0;
            i = i_size - 1;
            cont = true;
        }
        
        if(!cont && !soldierDoesntPatrolAtDifferentPlacesSameTime(j,k,i)) {
            problemVariables[j][k][i] = 0;
            cont = true;
        }
        if(!cont && !minBreakToSoldier(j,k,i)) {
            problemVariables[j][k][i] = 0;
            cont = true;
        }
        if(!cont && !checkCommanderDoesntPatrol(j,k,i)) {
            problemVariables[j][k][i] = 0;
            cont = true;
        }
        if(!cont && !soldierIsPresentIfPatrols(j,k,i)) {
            problemVariables[j][k][i] = 0;
            cont = true;
        }
        if(!cont && !soldiersFromDifferentCrewsInShinGimel(j,k,i)){
            problemVariables[j][k][i] = 0;
            cont = true;
        }
        int diff = 0;
        if(!cont) {
            diff = calculateDifferenceInObjectiveFunction(j,k,i);
            if(!(acc + diff < currentMinValue)){
                //countWhatever++;
                problemVariables[j][k][i] = 0;
                cont = true;
               // cout<<"didnt pass number: " << countWhatever << "value was: " << acc+diff << "while minimum is: "<<currentMinValue << "iteration : "<<sumOfRecursiveCalls<< endl;
            }
        }
    
        
        if(solela == false && !cont){
           // cout <<"a" <<endl;
            solela = checkEnoughSoldiersInSolela(j,k,i);
            //cout <<"aa" <<endl;
        } 
        
        if(shinGimel == false && !cont) {
           // cout <<"b" <<endl;
            shinGimel = checkEnoughSoldiersInShinGimel(j,k,i);
            //cout <<"bb" <<endl;
        }

        //reached a statisfiable configuration, check for optimality
        if(!cont && solela && shinGimel && problemVariables[j][k][i] == 1 ) {
            int curVal = evaluateObjectiveFunction(problemVariables);
            cout<<"recursive calls: "<<sumOfRecursiveCalls <<endl;
            cout<<"option's value: "<<curVal <<" which is equivalent to: " << evaluateObjectiveFunction(problemVariables) <<endl;
            if(curVal != acc + calculateDifferenceInObjectiveFunction(j,k,i))
                cout << "ERROR" <<endl;
            //optimal
            if(curVal < currentMinValue){
                //cout <<"s" <<endl;
                currentMinValue = curVal;
                deepCopyArr(problemVariables, currentBestAnswer,j_size,k_size,i_size);
            }
            //cout <<"aa" <<endl;
            problemVariables[j][k][i] = 0;
            return;
        }

        //recursive step
        solela = false;
        shinGimel = false;
        if(i+1==i_size){
                if(k+1==k_size){
                    if(j+1==j_size){
                        //cout<<"reached the end"<<endl;
                        problemVariables[j][k][i] = 0;
                        return;
                    }
                    else {
                        if(!cont){
                            updateAccumulation(j,k,i,1);
                            rec(j+1,0,0,solela,shinGimel,acc + diff);
                            updateAccumulation(j,k,i,-1);
                        }
                        problemVariables[j][k][i] = 0;
                        rec(j+1,0,0,solela,shinGimel,acc);
                    }
                }
                else {
                        if(!cont){
                            updateAccumulation(j,k,i,1);
                            rec(j,k+1,0,solela,shinGimel,acc + diff);
                            updateAccumulation(j,k,i,-1);
                        }
                        problemVariables[j][k][i] = 0;
                        rec(j,k+1,0,solela,shinGimel,acc);
                }
            }
            else {
                    if(!cont){
                        updateAccumulation(j,k,i,1);
                        rec(j,k,i+1,solela,shinGimel,acc + diff);
                        updateAccumulation(j,k,i,-1);
                    }  
                    problemVariables[j][k][i] = 0;
                    rec(j,k,i+1,solela,shinGimel,acc); 
            }
            return;
    }

    void optimize() {
        //j_size = 2;
        cout<< " j_size: " << j_size << " k_size: " << k_size<< " i_size: " <<i_size<<endl;
        initializeCalculation();
        int startingSum = evaluateObjectiveFunction(problemVariables);
        auto start = std::chrono::steady_clock::now();
        rec(0,0,0,false,false,startingSum);
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = end - start;
        std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
        cout<< "Total recursive calls: " << sumOfRecursiveCalls <<endl;
        cout<<"Starting Objective Function Evaluation: "<<startingSum <<endl;
        cout << "Objective Function Evaluation For Result: "<< evaluateObjectiveFunction(currentBestAnswer) << endl;
        //print3DArray(currentBestAnswer,j_size,k_size,i_size);
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

    napi_value GetResultObjectiveValue(napi_env env, napi_callback_info info) {
    // Create a JavaScript number with the value of x_var
    napi_value result;
    napi_create_int32(env, currentMinValue, &result);

    return result;
    }

    napi_value GlobalSetter(napi_env env, napi_callback_info info) {
    // Get the number of arguments passed to the function
    size_t argc = 2;
    napi_value args[2];

    // Parse the arguments
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // Check if two arguments are passed
    if (argc != 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments");
        return nullptr;
    }

    // Extract number from the first argument
    int64_t num = 0;
    napi_get_value_int64(env, args[0], &num);

    // Extract string from the second argument
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &strSize);
    std::string str(strSize + 1, '\0');
    napi_get_value_string_utf8(env, args[1], &str[0], strSize + 1, nullptr);

    // Check if the string equals "abc" and set the number to the global variable x_var
    if (str.compare("setMin") ) {
        currentMinValue = static_cast<int>(num);
    }

    // Return undefined
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

 napi_value SetMaxRecursions(napi_env env, napi_callback_info info) {
    // Get the number of arguments passed to the function
    size_t argc = 2;
    napi_value args[2];

    // Parse the arguments
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // Check if two arguments are passed
    if (argc != 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments");
        return nullptr;
    }

    // Extract number from the first argument
    int64_t num = 0;
    napi_get_value_int64(env, args[0], &num);

    // Extract string from the second argument
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &strSize);
    std::string str(strSize + 1, '\0');
    napi_get_value_string_utf8(env, args[1], &str[0], strSize + 1, nullptr);

    // Check if the string equals "abc" and set the number to the global variable x_var
    if (str.compare("setMaxRecursions") ) {
        maxRecursions = static_cast<long>(num);
    }

    // Return undefined
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
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
    //print2DArray(s,dim1,dim2);
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
    //print2DArray(k,dim1,dim2);
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
    //print2DArray(m,dim1,dim2);
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
    // for(int as=0; as<dim1; as++){
    //     cout<< nightShifts[as] << endl;
    // }
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
    //print1DArray(shinGimelTimes,dim1);
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
    //print2DArray(historyValues,dim1,dim2);
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
                status = napi_create_int32(env, currentBestAnswer[i][j][k], &jsValue);
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

        napi_value spv, sns, sh, sst, ss, sm, st, sk, su, s,grov, gs,smr;
        status = napi_create_function(env, nullptr, 0, SetProblemVariables, nullptr, &spv);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }
        status = napi_create_function(env, nullptr, 0, SetMaxRecursions, nullptr, &smr);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }
        status = napi_create_function(env, nullptr, 0, GetResultObjectiveValue, nullptr, &grov);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create function");
            return nullptr;
        }
        status = napi_create_function(env, nullptr, 0, GlobalSetter, nullptr, &gs);
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
        status = napi_set_named_property(env, exports, "setMaxRecursions", smr);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }

        status = napi_set_named_property(env, exports, "getResultObjectiveValue", grov);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set setProblemVariables as a property on exports");
            return nullptr;
        }
        status = napi_set_named_property(env, exports, "globalSetter", gs);
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
