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
const addon = require('../build/Release/OptimizationAlgorithmAPI.node');
// Assuming the data.json file is in the 'data' directory
const filePath = path.join(__dirname, '..', 'input.json');
const outputFile = 'expressionsOutput.json';
const shiftsOutputFile = 'result.json';
const configurationFile = 'config.json';
Date.prototype.toString = function () {
    return this.toLocaleString();
};
Date.prototype.toJSON = function () {
    return this.toLocaleString();
};
let maxRecursions = 1000000000;
let numberOfIterations = 1;
let minShiftsBreak = 8;
let randomizedRecursionsIterations = 0;
let shuffleNames = true;
let planUntil = new Date();
let planFrom = new Date();
let patrolIntervals; //in minutes
let nightPatrolTime = { from: new Date("December 13, 2023 21:00:00"), until: new Date("December 13, 2023 06:00:00") };
let shinGimelTimes = [];
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
crewToValueIndex.set("9", 13);
crewToValueIndex.set("mapik", 17);
let problem = {
    variables: {},
    coefficients: {},
    constraints: [],
    objective: ''
};
function parseInputFile() {
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
                    //TODO: readyWithDawnTimes
                    readyWithDawnTime = stringToDateTimeParser(jsonData.readyWithDawnTime);
                    problem.coefficients["k"] = [];
                    problem.coefficients["m"] = [];
                    problem.coefficients["t"] = [];
                    problem.coefficients["s"] = [];
                    if (shuffleNames)
                        shuffleArray(jsonData.soldiers);
                    jsonData.soldiers.forEach((soldier, i) => {
                        indexToNameMap.set(i, soldier.name);
                        nameToIndexMap.set(soldier.name, i);
                        problem.coefficients["k"][i] = [];
                        problem.coefficients["m"][i] = [];
                        problem.variables["z"].map((patrolTime, j) => {
                            soldier.commanderTimes.map((commanderTime) => {
                                commanderTime.from = stringToDateTimeParser(commanderTime.from);
                                commanderTime.until = stringToDateTimeParser(commanderTime.until);
                                problem.coefficients["k"][i][j] = isInFullDateRange(commanderTime.from, commanderTime.until, patrolTime.time) ? 1 : 0;
                            });
                            if (soldier.commanderTimes.length == 0) {
                                problem.coefficients["k"][i][j] = 0;
                            }
                        });
                        problem.variables["z"].forEach((patrolTime, j) => {
                            problem.coefficients["m"][i][j] = 0;
                            soldier.presentDates.forEach((presentTime, index) => {
                                presentTime.from = stringToDateTimeParser(presentTime.from);
                                presentTime.until = stringToDateTimeParser(presentTime.until);
                                if (isInFullDateRange(presentTime.from, presentTime.until, patrolTime.time))
                                    problem.coefficients["m"][i][j] = 1;
                            });
                        });
                        problem.coefficients["t"][i] = crewToValueIndex.get(soldier.crew);
                        problem.coefficients["s"][i] = [];
                        for (let as = 0; as < problem.variables["z"].length; as++) {
                            if (soldier.isMT || soldier.isMedic)
                                problem.coefficients["s"][i][as] = 1;
                            else
                                problem.coefficients["s"][i][as] = 0;
                        }
                    });
                    nameToIndexMap.forEach((val, key) => {
                        historyEvaluationMap.set(val, { day: 0, night: 0 });
                    });
                    jsonData.history.solela.forEach((entry) => {
                        entry.names.forEach((name) => {
                            let index = nameToIndexMap.get(name);
                            if (index >= 0) {
                                let hold = historyEvaluationMap.get(index);
                                let curDay = hold.day ? hold.day : 0;
                                let curNight = hold.night ? hold.night : 0;
                                isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight + 1) : hold.day = (curDay + 1);
                            }
                        });
                    });
                    jsonData.history.shinGimel.forEach((entry) => {
                        entry.names.forEach((name) => {
                            let index = nameToIndexMap.get(name);
                            if (index >= 0) {
                                let hold = historyEvaluationMap.get(index);
                                let curDay = hold.day ? hold.day : 0;
                                let curNight = hold.night ? hold.night : 0;
                                isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, stringToDateTimeParser(entry.time)) ? hold.night = (curNight + 1) : hold.day = (curDay + 1);
                            }
                        });
                    });
                    if (!problem.constraints) {
                        problem.constraints = []; // Initialize constraints array if not already initialized
                    }
                    problem.variables["z"].map((patrolTime, j) => {
                        //solelaPatrolSoldiersAmount(j);
                        //shinGimelSoldiersAmount(j);
                        for (var i = 0; i < nameToIndexMap.size; i++) {
                            commanderDontDoPatrols(j, i);
                            MTOrMedicDontDoShinGimelConstraint(j, i);
                            //twoFromSameTeamInShinGimelNotAllowed(j, i);
                            //soldierAllowedToBeAtMaximumOneStationAtOnce(j,i);
                            for (var k = 0; k <= 1; k++) {
                                soldierPresencyConstraint(j, k, i);
                            }
                        }
                    });
                    //shinGimelTimes
                    jsonData.shinGimelTimes.forEach((shinGimelTime, index) => {
                        problem.variables["z"].forEach((element, index2) => {
                            if (isInFullDateRange(stringToDateTimeParser(shinGimelTime.from), stringToDateTimeParser(shinGimelTime.until), element.time)) {
                                element["shinGimel"] = 2;
                            }
                        });
                    });
                    problem.objective = ``;
                    nameToIndexMap.forEach((i, soldierName) => {
                        let dayExpression = ``;
                        let nightExpression = ``;
                        let dayHistory = 0;
                        let nightHistory = 0;
                        if (historyEvaluationMap.get(i)) {
                            dayHistory = historyEvaluationMap.get(i).day;
                            nightHistory = historyEvaluationMap.get(i).night;
                        }
                        problem.variables["z"].map((patrolTime, j) => {
                            for (let k = 0; k <= 1; k++) {
                                if (isInTimeRange(nightPatrolTime.from, nightPatrolTime.until, patrolTime.time)) {
                                    nightExpression = nightExpression.concat(`z[${j}][${k}][${i}] +`);
                                }
                                else {
                                    dayExpression = dayExpression.concat(`z[${j}][${k}][${i}] +`);
                                }
                            }
                        });
                        let dayExpExists = false, nightExpExists = false;
                        ;
                        if (dayExpression.length != 0) {
                            dayExpression = dayExpression.concat(`${dayHistory}`);
                            dayExpression = `(`.concat(dayExpression).concat(`) ** 2`);
                            dayExpExists = true;
                        }
                        if (nightExpression.length != 0) {
                            nightExpression = nightExpression.concat(`${nightHistory}`);
                            nightExpression = `(`.concat(nightExpression).concat(`) ** 3`);
                            nightExpExists = true;
                        }
                        let totalExpForSoldier = ``;
                        if (dayExpExists && nightExpExists) {
                            totalExpForSoldier = dayExpression.concat(` + `).concat(nightExpression);
                        }
                        else {
                            totalExpForSoldier = dayExpression.concat(nightExpression);
                        }
                        if (problem.objective.length == 0) {
                            problem.objective = totalExpForSoldier;
                        }
                        else {
                            problem.objective = problem.objective.concat(` + `).concat(totalExpForSoldier);
                        }
                    });
                    console.log("Finished parsing input file");
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
function prepareProblemDomainAndSolve() {
    let nightShifts = [];
    let historyValues = [];
    let shinGimelTimes = [];
    //console.log("shifts: ",problem.variables["z"])
    for (let j = 0; j < problem.variables["z"].length; j++) {
        if (problem.variables["z"][j].isNight)
            nightShifts[j] = true;
        else
            nightShifts[j] = false;
    }
    for (let i = 0; i < historyEvaluationMap.size; i++) {
        historyValues[i] = [];
        historyValues[i][0] = historyEvaluationMap.get(i).day + 1;
        historyValues[i][1] = historyEvaluationMap.get(i).night + 1;
    }
    problem.variables["z"].forEach((patrolTime, j) => {
        if (problem.variables["z"][j]["shinGimel"]) {
            shinGimelTimes[j] = problem.variables["z"][j]["shinGimel"];
        }
        else
            shinGimelTimes[j] = 0;
    });
    //console.log("vars dimensions: ", problem.variables["z"].length, 2 , problem.coefficients["m"].length)
    addon.setProblemVariables(problem.variables["z"].length, 2, problem.coefficients["m"].length);
    //console.log("t: ",problem.coefficients["t"])
    addon.setT(problem.coefficients["t"], problem.coefficients["t"].length);
    //console.log("m: ",problem.coefficients["m"])
    addon.setM(problem.coefficients["m"], problem.coefficients["m"].length, problem.coefficients["m"][0].length);
    //console.log("k: ",problem.coefficients["k"])
    addon.setK(problem.coefficients["k"], problem.coefficients["k"].length, problem.coefficients["k"][0].length);
    //addon.setU(problem.coefficients["u"], problem.coefficients["u"].length, problem.coefficients["u"][0].length)
    //console.log("s: ",problem.coefficients["s"])
    addon.setS(problem.coefficients["s"], problem.coefficients["s"].length, problem.coefficients["s"][0].length);
    //console.log("nightShifts: ",nightShifts)
    addon.setNightShifts(nightShifts, nightShifts.length);
    //console.log("historyValues: ",historyValues)
    addon.setHistory(historyValues, historyValues.length, 2);
    //console.log("shinGimelTimes: ", shinGimelTimes);
    addon.setShinGimelTimes(shinGimelTimes, shinGimelTimes.length);
    const solution = addon.solve(); //solution is a 3d array
    return solution;
}
function unparseSolution(arr) {
    const jsonObject = { "solela": [], "shinGimel": [] };
    for (let j = 0; j < problem.variables["z"].length; j++) {
        for (let k = 0; k < 2; k++) {
            let shiftEntry = { names: [], time: "" };
            for (let i = 0; i < nameToIndexMap.size; i++) {
                shiftEntry.time = problem.variables["z"][j].time.toString();
                if (arr[j][k][i] == 1) {
                    shiftEntry.names.push(indexToNameMap.get(i));
                }
            }
            if (k == 0)
                jsonObject.solela.push(shiftEntry);
            else if (k == 1)
                jsonObject.shinGimel.push(shiftEntry);
        }
    }
    writeObjectToFile(jsonObject, shiftsOutputFile);
}
function MTOrMedicDontDoShinGimelConstraint(timeIndex, soldierIndex) {
    problem.constraints.push(`z[${timeIndex}][1][${soldierIndex}] * ${problem.coefficients["s"][soldierIndex]} <= 0`);
}
function soldierPresencyConstraint(timeIndex, stationIndex, soldierIndex) {
    for (var k = 0; k <= 1; k++) {
        problem.constraints.push(`z[${timeIndex}][${k}][${soldierIndex}]  <= ${problem.coefficients["m"][soldierIndex][timeIndex] ? problem.coefficients["m"][soldierIndex][timeIndex] : 0}`);
    }
}
function commanderDontDoPatrols(timeIndex, soldierIndex) {
    problem.constraints.push(`(z[${timeIndex}][0][${soldierIndex}] + z[${timeIndex}][1][${soldierIndex}]) * ${problem.coefficients["k"][soldierIndex][timeIndex] ? problem.coefficients["k"][soldierIndex][timeIndex] : 0} <= 0`);
}
function solelaPatrolSoldiersAmount(j) {
    throw new Error('Function not implemented.');
}
function shinGimelSoldiersAmount(j) {
    throw new Error('Function not implemented.');
}
function twoFromSameTeamInShinGimelNotAllowed(j, i) {
    throw new Error('Function not implemented.');
}
function soldierAllowedToBeAtMaximumOneStationAtOnce(j, i) {
    throw new Error('Function not implemented.');
}
fs.readFile(configurationFile, 'utf8', (err, data) => {
    if (err) {
        console.error(err);
        return;
    }
    try {
        const configData = JSON.parse(data);
        shuffleNames = configData.shuffleNames;
        numberOfIterations = configData.numberOfArrayShuffleIterations;
        maxRecursions = configData.maxRecursionsPerIteration;
        minShiftsBreak = configData.minShiftsBreak;
        randomizedRecursionsIterations = configData.randomizedRecursionsIterations;
        addon.setMaxRecursions(maxRecursions, "setMaxRecursions");
        addon.setMinShiftsBreak(minShiftsBreak, "setMinShiftsBreak");
        addon.setRandomizedRecursionsIterations(randomizedRecursionsIterations, "setRandomizedRecursionsIterations");
        //minimumShiftRest = 
    }
    catch (_a) {
        console.log("error parsing config file");
    }
    let bestMin = 9999999;
    for (let iterations = 0; iterations < numberOfIterations; iterations++) {
        parseInputFile().then(() => {
            let optimizationProblem = {
                objective: problem.objective,
                constraints: problem.constraints,
                dimensions: { j: problem.variables["z"].length, k: 2, i: nameToIndexMap.size }
            };
            //writeObjectToFile(optimizationProblem, outputFile);
            addon.globalSetter(bestMin, "setMin");
            console.log("------------------------------");
            const solution = prepareProblemDomainAndSolve();
            let hold = addon.getResultObjectiveValue();
            console.log("result value: ", hold);
            if (hold < bestMin) {
                bestMin = hold;
                unparseSolution(solution);
            }
            console.log("------------------------------");
            // solveBinaryOptimizationProblem(optimizationProblem)
            // .then((solution) => {
            //     console.log('Optimization solution:', solution);
            // })
            // .catch((error) => {
            //     console.error('Error:', error);
            // });
        });
    }
});
function shuffleArray(array) {
    for (var i = array.length - 1; i > 0; i--) {
        var j = Math.floor(Math.random() * (i + 1));
        var temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}
