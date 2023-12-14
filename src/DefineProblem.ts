const fs = require('fs');
const path = require('path');
import axios from 'axios';

// Assuming the data.json file is in the 'data' directory
const filePath = path.join(__dirname, '..', 'config.json');
const outputFile = 'output.json';

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
let problem = {
    variables: {},
    coefficients:{},
    constraints: {},
    objective: {}
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
            problem.variables["z"] = quantizeTime(planFrom,planUntil,patrolIntervals);
            problem.variables["z"].map((patrolTime)=> {
                problem.variables["z"]["solela"] = [];
                
            })
            shinGimelTimes = jsonData.shinGimelTimes.forEach(element => {
                problem.variables["z"].map((patrolTime)=> {
                    problem.variables["z"]["shinGimel"] = isInFullDateRange(element.from, element.until, patrolTime.time) ? [] : undefined;
                })
            });;

            //TODO: readyWithDawnTimes
            readyWithDawnTime = stringToDateTimeParser(jsonData.readyWithDawnTime);
            problem.coefficients["k"] = []
            problem.coefficients["m"] = []
            problem.coefficients["t"] = []
            problem.coefficients["s"] = []
            jsonData.soldiers.forEach((soldier, i) => {
                indexToNameMap[i]=soldier.name;
                nameToIndexMap[soldier.name] = i;
                problem.variables["z"].map((patrolTime) => {
                    soldier.commanderTimes.map((commanderTime) => {
                        problem.coefficients["k"][i] = []
                        problem.coefficients["k"][i][patrolTime.time] = isInFullDateRange(commanderTime.from, commanderTime.until, patrolTime.time) ? 1 : 0;
                    });
                });
                problem.variables["z"].forEach((patrolTime) => {
                    soldier.presentDates.map((presentTime) => {
                        problem.coefficients["m"][i] = []
                        problem.coefficients["m"][i][patrolTime.time] = isInFullDateRange(presentTime.from, presentTime.until, patrolTime.time) ? 1 : 0;
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

Date.prototype.toString = function (): string {
    return this.toLocaleString();
};
Date.prototype.toJSON = function (): string {
    return this.toLocaleString();
};


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
        const response = await axios.post('http://localhost:5000/solve_optimization', optProblem);
        console.log('Optimization solution:', response.data);
    } catch (error) {
        console.error('Error solving binary optimization problem:', error);
    }
}

parseConfigurationFile().then(() => {
    writeObjectToFile(problem, outputFile)
    writeObjectToFile(historyEvaluationMap, outputFile)

    let optimizationProblem = {
        objective: problem.objective,
        constraints: problem.constraints
    };

    solveBinaryOptimizationProblem(optimizationProblem)
    .then((solution) => {
        console.log('Optimization solution:', solution);
    })
    .catch((error) => {
        console.error('Error:', error);
    });



});