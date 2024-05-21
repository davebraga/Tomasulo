#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

// Declaring standard namespaces
using std::string;
using std::vector;
using std::map;
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
    }


    void run() {}
};

int main() {
    vector<Instruction*> instructions;
    ifstream inputFile("input.txt"); 

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

    return 0;
}