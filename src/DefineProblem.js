var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
const fs = require('fs');
const path = require('path');
// Assuming the data.json file is in the 'data' directory
const filePath = path.join(__dirname, '..', 'config.json');
const outputFile = 'output.json';
let planUntil = new Date();
let planFrom = new Date();
let patrolIntervals; //in minutes
let nightPatrolTime = { from: new Date("December 13, 2023 21:00:00"), until: new Date("December 13, 2023 06:00:00") };
let shinGimelTimes = { from: new Date(), until: new Date() };
let readyWithDawnTime;
let indexToNameMap = new Map();
let nameToIndexMap = new Map();
let historyEvaluationMap = new Map(); //soldier index to {day history constant, night history constant}
let crewToValueIndex = new Map();
crewToValueIndex.set("1a", 1);
crewToValueIndex.set("1b", 3);
crewToValueIndex.set("1c", 5);
crewToValueIndex.set("2a", 7);
crewToValueIndex.set("2b", 11);
let problem = {
    variables: {},
    coefficients: {},
    constraints: {},
    objective: {}
};
function parseConfigurationFile() {
    return __awaiter(this, void 0, void 0, function* () {
        return new Promise((resolve, reject) => {
            fs.readFile(filePath, 'utf8', (err, data) => {
                if (err) {
                    console.error(err);
                    reject("rejected");
                    return;
                }
                try {
                    const jsonData = JSON.parse(data);
                    planFrom = new Date(jsonData.planFrom);
                    planUntil = new Date(jsonData.planUntil);
                    patrolIntervals = jsonData.patrolIntervals;
                    nightPatrolTime = {
                        from: stringToDateTimeParser(jsonData.nightPatrolTime.from),
                        until: stringToDateTimeParser(jsonData.nightPatrolTime.until)
                    };
                    problem.variables["z"] = quantizeTime(planFrom, planUntil, patrolIntervals);
                    problem.variables["z"].map((patrolTime) => {
                        problem.variables["z"]["solela"] = [];
                    });
                    shinGimelTimes = jsonData.shinGimelTimes.forEach(element => {
                        problem.variables["z"].map((patrolTime) => {
                            problem.variables["z"]["shinGimel"] = isInFullDateRange(element.from, element.until, patrolTime.time) ? [] : undefined;
                        });
                    });
                    ;
                    //TODO: readyWithDawnTimes
                    readyWithDawnTime = stringToDateTimeParser(jsonData.readyWithDawnTime);
                    problem.coefficients["k"] = [];
                    problem.coefficients["m"] = [];
                    problem.coefficients["t"] = [];
                    problem.coefficients["s"] = [];
                    jsonData.soldiers.forEach((soldier, i) => {
                        indexToNameMap[i] = soldier.name;
                        nameToIndexMap[soldier.name] = i;
                        problem.variables["z"].map((patrolTime) => {
                            soldier.commanderTimes.map((commanderTime) => {
                                problem.coefficients["k"][i] = [];
                                problem.coefficients["k"][i][patrolTime.time] = isInFullDateRange(commanderTime.from, commanderTime.until, patrolTime.time) ? 1 : 0;
                            });
                        });
                        problem.variables["z"].forEach((patrolTime) => {
                            soldier.presentDates.map((presentTime) => {
                                problem.coefficients["m"][i] = [];
                                problem.coefficients["m"][i][patrolTime.time] = isInFullDateRange(presentTime.from, presentTime.until, patrolTime.time) ? 1 : 0;
                            });
                        });
                        problem.coefficients["t"][i] = crewToValueIndex.get(soldier.crew);
                        problem.coefficients["s"][i] = soldier.isMT || soldier.isMedic ? 1 : 0;
                    });
                    jsonData.history.solela.forEach((entry) => {
                        let index = nameToIndexMap[entry.name];
                        console.log("fgsa ", index);
                        if (index >= 0) {
                            let hold = historyEvaluationMap[index];
                            if (!hold) {
                                historyEvaluationMap[index] = { day: 0, night: 0 };
                                hold = historyEvaluationMap[index];
                            }
                            let curDay = hold.day ? hold.day : 0;
                            let curNight = hold.night ? hold.night : 0;
                            isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight + 1) : hold.day = (curDay + 1);
                        }
                        console.log(historyEvaluationMap);
                    });
                    jsonData.history.shinGimel.forEach((entry) => {
                        let index = nameToIndexMap[entry.name];
                        if (index >= 0) {
                            let hold = historyEvaluationMap[index];
                            if (!hold) {
                                historyEvaluationMap[index] = { day: 0, night: 0 };
                                hold = historyEvaluationMap[index];
                            }
                            let curDay = hold.day ? hold.day : 0;
                            let curNight = hold.night ? hold.night : 0;
                            isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight + 1) : hold.day = (curDay + 1);
                        }
                    });
                    console.log("finished parsing config file");
                    resolve();
                }
                catch (error) {
                    console.error('Error parsing JSON:', error);
                    reject("rejected");
                }
            });
        });
    });
}
function stringToDateTimeParser(str) {
    if (str.length <= 8)
        str = "January 1, 1970 ".concat(str);
    return new Date(str);
}
function isInTimeRange(x, y, subject) {
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
function isInFullDateRange(x, y, subject) {
    return subject > x && subject < y;
}
function quantizeTime(from, until, interval) {
    const result = [];
    let current = new Date(from);
    while (current < until) {
        const isNight = isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, current); // Custom function to check if it's night
        result.push({ time: new Date(current), isNight });
        current = new Date(current.getTime() + interval * 60000); // Increment by interval minutes
    }
    return result;
}
function writeObjectToFile(objectData, filePath) {
    try {
        const jsonData = JSON.stringify(objectData, null, 2); // Convert object to JSON string with 2 spaces indentation
        fs.writeFileSync(filePath, jsonData, 'utf8');
        console.log('Data has been written to the file:', filePath);
    }
    catch (error) {
        console.error('Error writing to file:', error);
    }
}
Date.prototype.toString = function () {
    return this.toLocaleString();
};
Date.prototype.toJSON = function () {
    return this.toLocaleString();
};
parseConfigurationFile().then(() => {
    writeObjectToFile(problem, outputFile);
    writeObjectToFile(historyEvaluationMap, outputFile);
});
