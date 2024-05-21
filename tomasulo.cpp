#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

// Declaring standard namespaces
using std::string;
using std::vector;
using std::map;
using std::to_string;
using std::cout;
using std::endl;
using std::ifstream;
using std::cerr;
using std::istringstream;

// Class responsible for instruction data
class Instruction {
    public:
    string operation;
    string targetRegister;  // Which register is associated to the instruction
    string operandOne; 
    string operandTwo; 
    int cyclesLeft;
    bool isIssued;
    bool isExecuting;
    bool isDone;

    Instruction(string operation, string targetRegister, string operandOne, string operandTwo)
        : operation(operation), targetRegister(targetRegister), operandOne(operandOne), operandTwo(operandTwo), 
        cyclesLeft(0), isIssued(false), isExecuting(false), isDone(false) {}
};

// Class responsible for functional unit data
class FunctionalUnit {
    public:
    string type; 
    int latencyCycles; 
    bool isBusy;
    Instruction* instruction;

    FunctionalUnit(string type, int latencyCycles) : type(type), latencyCycles(latencyCycles), isBusy(false), instruction(nullptr) {}
};

// Class responsible for register data
class Register {
    public:
    string name;
    int value;  // Current value saved on a register
    bool isReading; // If the register is currently being read
    bool isWriting; // If the register is currently being written
    Instruction* instruction; 

    Register(string name, int value) : name(name), value(value), isReading(false), isWriting(false), instruction(nullptr) {}
};

// Class responsible for performing Tomasulo's algorithm
class Tomasulo {
    public:
    vector<Instruction*> instructions; 
    vector<FunctionalUnit*> addUnits; // Add operation FUs
    vector<FunctionalUnit*> mulUnits; // Multiplication operation FUs
    vector<FunctionalUnit*> storeWordUnits; // Store word operation FUs
    vector<Register*> registers; 
    map<string, string> rename; // Intended rename for a determined register 
    int currentCycle; 
    vector<int> cacheMemory;

    Tomasulo(vector<Instruction*> instructions, map<string, int> values)
        : instructions(instructions), currentCycle(1), cacheMemory(32, 2) {
        addUnits = createFunctionalUnits(values["addFUAgg"], "add", values["addFULatency"]);
        mulUnits = createFunctionalUnits(values["mulFUAgg"], "mul", values["mulFULatency"]);
        storeWordUnits = createFunctionalUnits(values["swFUAgg"], "sw", values["swFULatency"]);
        registers = createRegisters(values["registerAgg"]);
    }

    /**
    * Create n given FUs amount.
    *
    * @param amount how many FUs should be created.
    * @param type type of an operation associated to the FU.
    * @param latencyCycles how many latency cycles a FU will have.
    * @return array of configured FU.
    */
    vector<FunctionalUnit*> createFunctionalUnits(int amount, string type, int latencyCycles) {
        vector<FunctionalUnit*> units;
        for (int i = 0; i < amount; i++) {
            units.push_back(new FunctionalUnit(type, latencyCycles));
        }
        return units;
    }

    /**
    * Create n given registers amount.
    *
    * @param amount how many registers should be created.
    * @return array of configured registers.
    */
    vector<Register*> createRegisters(int amount) {
        vector<Register*> reg;
        for (int i = 0; i < amount; i++) {
            reg.push_back(new Register("F" + to_string(i), 1));
        }
        for (int i = 0; i < amount; i++) {
            reg.push_back(new Register("R" + to_string(i), 1));
        }
        return reg;
    }

    /**
    * Checks if the pipeline is already completed.
    *
    * @return boolean indicating if the algorithm is finished.
    */
    bool isFinished() {
        for (size_t i = 0; i < instructions.size(); i++) {
            if (!instructions[i]->isDone) {
                return false;
            }
        }
        return true;
    }

    /**
    * Given a determined register, renames it.
    *
    * @param target reference for a given register.
    * @param index memory index of the given register.
    */
    void renameRegister(Register* target, size_t index) {
        size_t auxRegister = registers.size() / 2;
        string auxName = "R0";

        // Find the next available temporary register
        while (registers[auxRegister]->isReading || registers[auxRegister]->isWriting) {
            auxRegister++;
            if (auxRegister >= registers.size()) {
                return;
            } else {
                auxName = registers[auxRegister]->name;
            }
        }

        if (rename.find(target->name) != rename.end()) {
            rename[auxName] = rename[target->name];
        } else {
            rename[auxName] = target->name;
        }

        // Rename every next occurrence of the renamed register
        for (size_t i = index; i < instructions.size(); i++) {
            if (instructions[i]->targetRegister == target->name) {
                instructions[i]->targetRegister = auxName;
            }

            if (instructions[i]->operandOne == target->name) {
                instructions[i]->operandOne = auxName;
            }

            if (instructions[i]->operandTwo == target->name) {
                instructions[i]->operandTwo = auxName;
            }
        }
    }

    /**
    * Responsible to achieve emit step.
    */
    void issue() {
        size_t constrain = instructions.size();

        if (currentCycle < instructions.size()) {
            constrain = currentCycle;
        }

        for (size_t i = 0; i < constrain; i++) {
            Instruction* instruction = instructions[i];
            FunctionalUnit* unit = nullptr;

            if (instruction->isExecuting || instruction->isDone || !instruction->isIssued) {
                // Check FU availability
                instruction->isIssued = true;
                unit = nullptr;
            } else if (instruction->operation == "add" || instruction->operation == "sub") {
                unit = findAvailableFU(addUnits);
            } else if (instruction->operation == "mul" || instruction->operation == "div") {
                unit = findAvailableFU(mulUnits);
            } else if (instruction->operation == "sw" || instruction->operation == "lw") {
                unit = findAvailableFU(storeWordUnits);
            }

            if (unit) {
                Register* aux;
                if (instruction->operation == "sw" || instruction->operation == "lw") {
                    aux = new Register("aux", stoi(instruction->operandOne));
                } else {
                    aux = getRegister(instruction->operandOne);
                }

                Register* destRegister = getRegister(instruction->targetRegister);
                Register* src1Register = aux;
                Register* src2Register = getRegister(instruction->operandTwo);

                // False dependency handler
                if (destRegister->isReading || destRegister->isWriting) {
                    renameRegister(destRegister, i);
                }

                // True dependency handler
                if (!src1Register->isWriting && !src2Register->isWriting) {
                    instruction->isExecuting = true;
                    instruction->cyclesLeft = unit->latencyCycles;

                    unit->isBusy = true;
                    unit->instruction = instruction;

                    // Update every register used by an instruction
                    destRegister->isWriting = true;
                    destRegister->instruction = instruction;

                    src1Register->isReading = true;
                    src1Register->instruction = instruction;

                    src2Register->isReading = true;
                    src2Register->instruction = instruction;
                }
            }
        }
    }

    /**
    * Handle algorithm execution step.
    */
    void execute() {
        for (size_t i = 0; i < addUnits.size(); i++) {
            FunctionalUnit* unit = addUnits[i];
            if (unit->isBusy) {
                unit->instruction->cyclesLeft--;
                if (unit->instruction->cyclesLeft == 0) {
                    unit->instruction->isExecuting = false;
                }
            }
        }

        for (size_t i = 0; i < mulUnits.size(); i++) {
            FunctionalUnit* unit = mulUnits[i];
            if (unit->isBusy) {
                unit->instruction->cyclesLeft--;
                if (unit->instruction->cyclesLeft == 0) {
                    unit->instruction->isExecuting = false;
                }
            }
        }

        for (size_t i = 0; i < storeWordUnits.size(); i++) {
            FunctionalUnit* unit = storeWordUnits[i];
            if (unit->isBusy) {
                unit->instruction->cyclesLeft--;
                if (unit->instruction->cyclesLeft == 0) {
                    unit->instruction->isExecuting = false;
                }
            }
        }
    }

    /**
    * Handle register write step.
    */
    void handleWrite() {
        write(addUnits);
        write(mulUnits);
        write(storeWordUnits);
    }

   
    /**
    * Write to a register.
    *
    * @param unitType type of a given FU.
    */
    void write(vector<FunctionalUnit*>& unitType) {
        for (size_t i = 0; i < unitType.size(); i++) {
            FunctionalUnit* unit = unitType[i];
            if (unit->isBusy && !unit->instruction->isExecuting && !unit->instruction->isDone) {
                unit->instruction->isDone = true;
                unit->isBusy = false;

                // Updated FU's target register's value
                Register* targetRegister = getRegister(unit->instruction->targetRegister);
                Register* operandOneRegister = getRegister(unit->instruction->operandOne);
                Register* operandTwoRegister = getRegister(unit->instruction->operandTwo);

                if (unit->instruction->operation == "sw" || unit->instruction->operation == "lw") {
                    int offset = performOperation("add", stoi(unit->instruction->operandOne), operandTwoRegister->value);

                    if (unit->instruction->operation == "sw") {
                        cacheMemory[offset % cacheMemory.size()] = targetRegister->value;
                    } else if (unit->instruction->operation == "lw") {
                        targetRegister->value = cacheMemory[offset % cacheMemory.size()];
                    }
                } else {
                    targetRegister->value = performOperation(unit->instruction->operation, operandOneRegister->value, operandTwoRegister->value);

                    operandOneRegister->isReading = false;
                    operandOneRegister->instruction = nullptr;
                }

                // Free memory of aux registers
                targetRegister->isWriting = false;
                targetRegister->instruction = nullptr;

                if (rename.find(targetRegister->name) != rename.end()) {
                    renameInstructionsRegister(targetRegister, i);
                }

                operandTwoRegister->isReading = false;
                operandTwoRegister->instruction = nullptr;
            }
        }
    }

    /**
    * Responsible for renaming a register in instructions list.
    *
    * @param target rename target register.
    * @param index index of a given register.
    */
    void renameInstructionsRegister(Register* target, size_t index) {
        for (size_t i = 0; i < instructions.size(); i++) {
            if (instructions[i]->targetRegister == target->name) {
                instructions[i]->targetRegister = rename[target->name];
            }

            if (instructions[i]->operandOne == target->name) {
                instructions[i]->operandOne = rename[target->name];
            }

            if (instructions[i]->operandTwo == target->name) {
                instructions[i]->operandTwo = rename[target->name];
            }
        }

        // Erase renamed registers array
        rename.erase(target->name);
    }

    /**
    * Iterates through every FU seeking for an available one.
    *
    * @param units current FUs.
    * @return available FU or @nullptr if none is found.
    */
    FunctionalUnit* findAvailableFU(vector<FunctionalUnit*>& units) {
        for (size_t i = 0; i < units.size(); i++) {
            if (!units[i]->isBusy) {
                return units[i];
            }
        }

        return nullptr;
    }

    /**
    * Find a register by an string name.
    *
    * @param name name of a register.
    * @return available register or @nullptr if none is found.
    */
    Register* getRegister(const string& name) {
        for (size_t i = 0; i < registers.size(); i++) {
            if (registers[i]->name == name) {
                return registers[i];
            }
        }

        return nullptr;
    }

    /**
    * Process ALU operations.
    *
    * @param operation given math operation.
    * @param operandOne first number operand.
    * @param operandTwo second number operand.
    * @return result of a math operation or @0 if operation is not valid.
    */
    int performOperation(const string& operation, int operandOne, int operandTwo) {
        if (operation == "add") {
            return operandOne + operandTwo;
        } else if (operation == "sub") {
            return operandOne - operandTwo;
        } else if (operation == "mul") {
            return operandOne * operandTwo;
        } else if (operation == "div") {
            return operandOne / operandTwo;
        }
        
        return 0;
    }

    /**
    * Show status of the current cycle which is being executed.
    */
    void showCycleStatus() {
        cout << "\n@ Status: cycle " << currentCycle << "\n";
        cout << "\nInstructions read: ";
        for (size_t i = 0; i < instructions.size(); i++) {
            if (instructions[i]->isIssued) {
                cout << "\n" << instructions[i]->operation << " " << instructions[i]->targetRegister
                          << " " << instructions[i]->operandOne << " " << instructions[i]->operandTwo;
            }
        }

        cout << "\n\nInstructions executing:";
        for (size_t i = 0; i < instructions.size(); i++) {
            if (instructions[i]->isExecuting) {
                cout << "\n" << instructions[i]->operation << " " << instructions[i]->targetRegister
                          << " " << instructions[i]->operandOne << " " << instructions[i]->operandTwo;
            }
        }

        cout << "\n\nInstructions finished:";
        for (size_t i = 0; i < instructions.size(); i++) {
            if (instructions[i]->isDone) {
                cout << "\n" << instructions[i]->operation << " " << instructions[i]->targetRegister
                          << " " << instructions[i]->operandOne << " " << instructions[i]->operandTwo;
            }
        }

        cout << endl;
    }

    /**
    * Inits Tomasulo's algorithm.
    */
    void run() {
        while (!isFinished()) {
            issue();
            execute();
            handleWrite();
            showCycleStatus();
            currentCycle++;
        }
        cout << "\nFinished. Terminating program.\n";
    }
};

/**
* Free instruction alocated memory.
* @param instructions array of current alocated instructions.
*/
void freeInstructions(vector<Instruction*>& instructions) {
    for (const auto& instruction : instructions) {
        delete instruction;
    }
}

/**
* Head of the code. Begin and end of everything. Past here there are only dust and echoes.
*/
int main() {
    vector<Instruction*> instructions;
    ifstream inputFile("large_input.txt"); 

    if (!inputFile.is_open()) {
        cerr << "Failed to read file. Aborting program." << endl;
        return 1;
    }

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        string op, dest, src1, src2;
        iss >> op >> dest >> src1 >> src2;
        instructions.push_back(new Instruction(op, dest, src1, src2));
    }

    inputFile.close();

    // Custom values for instruction's latency, registers and FUs amount
    map<string, int> values = {
        {"addFULatency", 3},
        {"mulFULatency", 10},
        {"swFULatency", 2},
        {"addFUAgg", 3},
        {"mulFUAgg", 2},
        {"swFUAgg", 2},
        {"registerAgg", 16}
    };

    vector<int> cacheMem(32, 2); 

    Tomasulo tomasulo(instructions, values);
    tomasulo.run();
    freeInstructions(instructions); // Free alocated memory for pipeline instructions

    return 0;
}