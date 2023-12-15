const fs = require('fs');
const path = require('path');
import axios from 'axios';

// Assuming the data.json file is in the 'data' directory
const filePath = path.join(__dirname, '..', 'config.json');
const outputFile = 'output.json';

Date.prototype.toString = function (): string {
    return this.toLocaleString();
};
Date.prototype.toJSON = function (): string {
    return this.toLocaleString();
};

let planUntil:Date = new Date();
let planFrom:Date = new Date();
let patrolIntervals:number; //in minutes
let nightPatrolTime:{from:Date,until:Date} = {from: new Date("December 13, 2023 21:00:00"), until: new Date("December 13, 2023 06:00:00")};
let shinGimelTimes:{from:Date,until:Date} = {from: new Date(), until: new Date()};
let readyWithDawnTime:Date;
let indexToNameMap:Map<number,string> = new Map();
let nameToIndexMap:Map<string,number> = new Map();
let historyEvaluationMap:Map<number,{day:number, night:number}> = new Map(); //soldier index to {day history constant, night history constant}
let crewToValueIndex:Map<string,number> = new Map();
    crewToValueIndex.set("1a",1);
    crewToValueIndex.set("1b",3);
    crewToValueIndex.set("1c",5);
    crewToValueIndex.set("2a",7);
    crewToValueIndex.set("2b",11);
let problem : any= {
    variables: {} ,
    coefficients:{},
    constraints: [],
    objective: ''
};

async function parseConfigurationFile(): Promise<void>  {
    return new Promise<void>((resolve,reject) => {
    fs.readFile(filePath, 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            reject("rejected")
            return;
        }

        try {
            const jsonData = JSON.parse(data);

            planFrom = new Date(jsonData.planFrom);
            planUntil = new Date(jsonData.planUntil);
            patrolIntervals = jsonData.patrolIntervals;
            nightPatrolTime = {
                                from:stringToDateTimeParser(jsonData.nightPatrolTime.from),
                                until:stringToDateTimeParser(jsonData.nightPatrolTime.until)
                                }
            problem.variables["z"] = quantizeTime(planFrom,planUntil,patrolIntervals)
            
            //TODO: readyWithDawnTimes
            readyWithDawnTime = stringToDateTimeParser(jsonData.readyWithDawnTime);
            problem.coefficients["k"] = []
            problem.coefficients["m"] = []
            problem.coefficients["t"] = []
            problem.coefficients["s"] = []
            jsonData.soldiers.forEach((soldier, i) => {
                indexToNameMap.set(i,soldier.name)
                nameToIndexMap.set(soldier.name,i);
                problem.coefficients["k"][i] = []
                problem.coefficients["m"][i] = []
                problem.variables["z"].map((patrolTime,j) => {
                    soldier.commanderTimes.map((commanderTime) => {
                        commanderTime.from = stringToDateTimeParser(commanderTime.from)
                        commanderTime.until = stringToDateTimeParser(commanderTime.until)
                        problem.coefficients["k"][i][j] = isInFullDateRange(commanderTime.from, commanderTime.until, patrolTime.time) ? 1 : 0;
                    });
                });
                problem.variables["z"].forEach((patrolTime) => {
                    soldier.presentDates.map((presentTime,j) => {
                        presentTime.from = stringToDateTimeParser(presentTime.from)
                        presentTime.until = stringToDateTimeParser(presentTime.until)
                        problem.coefficients["m"][i][j] = isInFullDateRange(presentTime.from, presentTime.until, patrolTime.time) ? 1 : 0;
                    });
                });
                problem.coefficients["t"][i] = crewToValueIndex.get(soldier.crew);
                problem.coefficients["s"][i] = soldier.isMT || soldier.isMedic ? 1 : 0;
            });
            
            jsonData.history.solela.forEach((entry) => {
                let index = nameToIndexMap[entry.name]
                
                if(index>=0){
                    let hold = historyEvaluationMap[index]
                    if(!hold) {
                        historyEvaluationMap[index] = {day:0, night:0}
                        hold = historyEvaluationMap[index]
                    }
                    let curDay = hold.day ? hold.day : 0;
                    let curNight = hold.night ? hold.night : 0;
                    isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight+1) : hold.day = (curDay + 1);
                    
                }
            })

            jsonData.history.shinGimel.forEach((entry) => {
                let index = nameToIndexMap[entry.name]
                if(index>=0){
                    let hold = historyEvaluationMap[index]
                    if(!hold) {
                       historyEvaluationMap[index] = {day:0, night:0}
                       hold = historyEvaluationMap[index]
                    }
                    let curDay = hold.day ? hold.day : 0;
                    let curNight = hold.night ? hold.night : 0;
                    isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight+1) : hold.day = (curDay + 1);
                }
            })

            if (!problem.constraints) {
                problem.constraints = []; // Initialize constraints array if not already initialized
            }
            problem.variables["z"].map((patrolTime,j)=> {
                    //solelaPatrolSoldiersAmount(j);
                    //shinGimelSoldiersAmount(j);
                    for(var i=0; i<nameToIndexMap.size; i++){
                        commanderDontDoPatrols(j,i);
                        MTOrMedicDontDoShinGimelConstraint(j,i);
                        //twoFromSameTeamInShinGimelNotAllowed(j, i);
                        //soldierAllowedToBeAtMaximumOneStationAtOnce(j,i);
                        for(var k=0; k<=1; k++){
                            soldierPresencyConstraint(j,k,i);
                        }
                    }
                
                
            })
            problem.objective = ``;
            console.log(nameToIndexMap.size)
            nameToIndexMap.forEach( (i, soldierName) => {
                let dayExpression = ``;
                let nightExpression = ``;
                let dayHistory = 0;
                let nightHistory = 0;
                if(historyEvaluationMap.get(i)) {
                    dayHistory = historyEvaluationMap[i].day
                    nightHistory = historyEvaluationMap[i].night
                }
                console.log(" "," ",i)
                problem.variables["z"].map((patrolTime, j) => {
                    for (let k = 0; k<=1; k++) {
                        if(isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, patrolTime.time)) {
                            nightExpression = nightExpression.concat(`z[${j}][${k}][${i}] +`)
                        } else {
                            dayExpression = dayExpression.concat(`z[${j}][${k}][${i}] +`)
                        }
                    }

                })
                let dayExpExists = false, nightExpExists = false;;
                if(dayExpression.length != 0) {
                    dayExpression = dayExpression.concat(`${dayHistory}`)
                    dayExpression = `(`.concat(dayExpression).concat(`) ** 2`)
                    dayExpExists = true;
                }
                if(nightExpression.length != 0) {
                    nightExpression = nightExpression.concat(`${nightHistory}`)
                    nightExpression = `(`.concat(nightExpression).concat(`) ** 3`)
                    nightExpExists = true;
                }
                let totalExpForSoldier = ``
                if(dayExpExists && nightExpExists) {
                    totalExpForSoldier = dayExpression.concat(` + `).concat(nightExpression)
                }
                else {
                    totalExpForSoldier = dayExpression.concat(nightExpression)
                }

                if(problem.objective.length == 0) {
                    problem.objective = totalExpForSoldier
                }
                else {
                    problem.objective = problem.objective.concat(` + `).concat(totalExpForSoldier)
                }
            })
            
            console.log("Finished parsing config file")
            resolve()
        } catch (error) {
            console.error('Error parsing JSON:', error);
            reject("rejected")
        }
    });
})
}

function stringToDateTimeParser(str:string):Date {
    if(str.length<=8)
        str = "January 1, 1970 ".concat(str)
    return new Date(str);
}

function isInTimeRange(x:Date, y:Date, subject:Date) {
    // Extract time components (hours, minutes, seconds) from Date objects
    let xTime = x.getHours() * 3600 + x.getMinutes() * 60 + x.getSeconds();
    let yTime = y.getHours() * 3600 + y.getMinutes() * 60 + y.getSeconds();
    let objectiveTime = subject.getHours() * 3600 + subject.getMinutes() * 60 + subject.getSeconds();
    if (yTime < xTime) {
        // Adjust yTime if it refers to the next day
        return objectiveTime >= xTime || objectiveTime < yTime;
    }
    return objectiveTime >= xTime && objectiveTime < yTime;
}

function isInFullDateRange(x: Date, y: Date, subject: Date): boolean {
    return subject > x && subject < y;
}

function quantizeTime(from: Date, until: Date, interval: number): { time: Date; isNight: boolean }[] {
    const result: { time: Date; isNight: boolean }[] = [];
    let current = new Date(from);

    while (current < until) {
        const isNight = isInTimeRange(nightPatrolTime.from,nightPatrolTime.until,current); // Custom function to check if it's night
        result.push({ time: new Date(current), isNight });
        current = new Date(current.getTime() + interval * 60000); // Increment by interval minutes
    }

    return result;
}

function writeObjectToFile(objectData: object, filePath: string): void {
    try {
        const jsonData = JSON.stringify(objectData, null, 2); // Convert object to JSON string with 2 spaces indentation

        fs.writeFileSync(filePath, jsonData, 'utf8');
        console.log('Data has been written to the file:', filePath);
    } catch (error) {
        console.error('Error writing to file:', error);
    }
}

async function solveBinaryOptimizationProblem(optProblem): Promise<void> {
    // const optimizationProblem = {
    //     objective: 'x[0][0][0] + x[1][1][1] + x[2][2][2]', // Objective function with a 3D matrix of binary variables
    //     constraints: [
    //         'x[0][0][0] + x[1][1][1] >= 1', // Constraint 1
    //         'x[1][1][1] + x[2][2][2] >= 1', // Constraint 2
    //         'x[0][0][0] != x[1][1][1]',    // Not equal constraint between x[0][0][0] and x[1][1][1]
    //         // Add more constraints here as needed
    //     ]
    // };

    try {
        const response = await axios.get('http://localhost:5003/solve_optimization');
        console.log('Optimization solution:', response.data);
    } catch (error) {
        console.error('Error solving binary optimization problem:', error);
    }
}


function MTOrMedicDontDoShinGimelConstraint(timeIndex:number, soldierIndex:number) {
    problem.constraints.push(`z[${timeIndex}][1][${soldierIndex}] * ${problem.coefficients["s"][soldierIndex]} <= 0`);
}
function soldierPresencyConstraint(timeIndex: number, stationIndex: number, soldierIndex: number) {
    for(var k=0; k <=1; k++) {
        problem.constraints.push(`z[${timeIndex}][${k}][${soldierIndex}]  <= ${problem.coefficients["m"][soldierIndex][timeIndex] ? problem.coefficients["m"][soldierIndex][timeIndex] : 0 }`);
    }
}
function commanderDontDoPatrols(timeIndex: any, soldierIndex: number) {
    problem.constraints.push(`(z[${timeIndex}][0][${soldierIndex}] + z[${timeIndex}][1][${soldierIndex}]) * ${problem.coefficients["k"][soldierIndex][timeIndex] ? problem.coefficients["k"][soldierIndex][timeIndex] : 0 } <= 0`);
}

function solelaPatrolSoldiersAmount(j: any) {
    throw new Error('Function not implemented.');
}

function shinGimelSoldiersAmount(j: any) {
    throw new Error('Function not implemented.');
}

function twoFromSameTeamInShinGimelNotAllowed(j: any, i: number) {
    throw new Error('Function not implemented.');
}

function soldierAllowedToBeAtMaximumOneStationAtOnce(j: any, i: number) {
    throw new Error('Function not implemented.');
}

parseConfigurationFile().then(() => {
    let optimizationProblem = {
        objective: problem.objective,
        constraints: problem.constraints,
        dimensions: {j:problem.variables["z"].length,k:2,i:nameToIndexMap.size}
    };
    
    writeObjectToFile(optimizationProblem, outputFile)


    // solveBinaryOptimizationProblem(optimizationProblem)
    // .then((solution) => {
    //     console.log('Optimization solution:', solution);
    // })
    // .catch((error) => {
    //     console.error('Error:', error);
    // });
});


