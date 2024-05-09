//Simulador - Algoritmo de Tomasulo
//Arquitetura de Computadores III
//Grupo: Daniel M.,Davi B., Gabriel L. e Lucas A.
//Puc Minas - Praca da Liberdade
//2024-01
/*Adaptado de:
LIU, Kan
SATAPATHY, Piyush Ranjan
SETHI, Ricky J.
Universidade da California, Riverside, 2004 
http://alumni.cs.ucr.edu/~piyush/cs203Project.pdf*/

#include <deque>
#include <string>

using namespace ::std;

#define NUM_REG 32 //Numero de registradores (tanto int quanto float)
#define NUM_FU 5 //O modelo apresenta 5 Unidades de Funcao (FU)
#define TAM_MEM 32 //Tamanho da memoria (ou numero de palavras)
#define CICLOS 1 //Numero de ciclos para realizar uma instrucao do tipo INT

#define EMPTY -1
#define ZERO 0.0

//No codigo original, implementou-se 11 instrucoes sendo 5 inteiras e 6 com numeros naturais(floating-point) 
enum Instrs {LD,SD,BEQ,BNE,ADD,ADD_D,ADDI,SUB,SUB_D,MULT_D,DIV_D};

//Unidades de Funcao (ou Function Unit)
enum FU {READY=-1,ADDER=0,MULTIPLIER,DIVIDER,LOAD,STORE};

enum StatusReserva{Processando,Completado,Nao_Executado,};

struct Instrucao{
    Instrs operacao;
    int label;
    int rd;
    int rs;
    int rt;
};

struct StatusInstrucao{
    Instrs operacao;
    int rd;
    int rs;
    int rt;
    int fetch;
    int issue;
    int exeStart;
    int exeCompl;
    int wriBack;
};

struct EstacaoReserva{
    Instrs operacao;
    bool ocupado;
    float vj;
    float vk;
    struct Indice qj;
    struct Indice qk;
    int des;
    StatusReserva status;
    int exe;
    struct StatusInstrucao* statusAtual;
};

//Indice da Unidade de Funcao 
struct Indice{
    int indexFU;
    int indexER;
};

//Indice da Estacao de Reserva
struct IndiceER{
    FU tipo;
    int indexFU;
    int indexER;
};

class Tomasulo{
    private:
        int registrINT[NUM_REG]; 
        float registrFLOAT[NUM_REG];
        float memoria[TAM_MEM];
        int numFU[NUM_FU];//qtde de Unidades de Funcao 
        int *numES[NUM_FU];//qtde de estacoes de reserva para cada FU
        int *ciclosExec[NUM_FU];//ciclos de execucao para cada FU
        int pc;
        int ciclo;

        string nomeArquivo;

        struct EstacaoReserva **estacoesDeReserva[NUM_FU];
        struct IndiceER statusDoRegistro[NUM_REG];
        struct StatusInstrucao *registradorInst; //aponta para a instrucao no aguardo para processo

        bool *statusDasFUs[NUM_FU];
        bool memoriaOcup; //Indica se ja possui uma instrucao na memoria
        bool *divOcup; //Indica se tem uma div usando a Unidade de Funcao

        //Filas
        deque <struct Instrucao> filaInstrucao;
        deque <struct StatusInstrucao> filaStatusInstr;
        deque <struct IndiceER> filaLoad;
        deque <struct IndiceER> filaStore;

    public:
        //Construtor
        Tomasulo(); 

        //Funcoes de inicializacao
        int initRegistrINT();
        int initRegistrFLOAT();
        int initMem();
        bool initArquivo(const char*); //Inicializa os registradores a partir de um arquivo
        int inicializar(); //Inicializa a simulacao do algoritmo

        //Display
        void printStatus(); //Mostra res,sta,stats, status da instancia e result de cada ciclo
        void printResultados();
        void printStatusLocal();
        void printMem();
        void printRegistrINT();
        void printRegistrFLOAT();
        void printInstrFila(ostream & out); //Mostra as instrucoes presentes na fila

        //Checagem das funcoes
        void checkInstr(struct Instrucao i); //checa o numero de instrucoes presentes no registrador
        bool checkIssue(); //checa as instrucoes
        bool checkER(int ty,int fu); //checa se a estacao de reserva pode iniciar a execucao
        bool checkERFinalizada(int ty, int fu, int in);
        //No codigo original, as duas funcoes a seguir servem como "bookkeeping", ou seja, contabilizam as instrucoes
        void bookkeepINT();
        void bookkeepFLOAT();

        //Acesso a memoria
        bool calculaEndereco(int ty, int fu, int in); //Calcula o endereco de memoria para realizar o load e o store
        bool acessaMemoria();

        bool parseInput(const char*);
        bool loadIssue();
        bool storeIssue();
        bool floatIssue(FU ty);
        bool result(int ty,int fu,int in);
        bool instructionFetch(int p);
};
