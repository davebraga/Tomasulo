#Simulador do Algoritmo de Tomasulo
##Sobre
Este simulador na linguagem C++ apresenta 4 classes principais:
### Instruction
Responsável pelos dados de instrução, bem como verificar se estes estão em execução ou já foram executados, bem como armazenar o número de ciclos restantes.
### Functional Unit
Como propriamente dito, se trata da simulação de uma Unidade Funcional (ou FU)de um hardware, onde cada FU representa uma unidade específica.
### Register
Armazena os valores temporários gerados durante a execução do algoritmo, bem como o status de ocupação durante a leitura ou escrita.
### Tomasulo
Esta classe é responsável por contruir as três classes apresentasas acima e executar o algoritmo com os comandos oferecidos no arquivo .txt do input

Além da main(), é possivel encontrar outras funções  
##Inicializando
Para a execução do algoritmo, é preciso inicialmente compilar o projeto, no qual pode ser feito executando o seguinte comando no terminal:

g++ tomasulo.cpp -o tomasulo 

Uma vez compilado, você pode abrir o executavel gerado clicando no arquivo ou através do terminal.

Ao abrir o arquivo, este irá aguardar a seleção de um arquivo .txt com o comando em Assembly separados por espaços para poder ser executado.**Os comandos aceitos são add, mul, sub e sw**, conforme apresentado no exemplo abaixo:

text 
add F1 F2 F3
sw F1 670 R1
sub F1 F3 F2
mul F1 F2 F3

Junto do código há exemplos de inputs a serem utilizados para teste

#Configurações
É possivel alterar as configurações de parametros do simulador como o número de unidades funcionais e latência de cada fução aceita dentro da main() no arquivo tomasulo.cpp , conforme mostrado a seguir:

cpp 
   map<string, int> values = {
        {"addFULatency", 3},
        {"mulFULatency", 10},
        {"swFULatency", 2},
        {"addFUAgg", 3},
        {"mulFUAgg", 2},
        {"swFUAgg", 2},
        {"registerAgg", 16}
    };


#Autores

- [Daniel] (https://github.com/Daniellucasm)
- [Davi] (https://github.com/davebraga)
- [Gabriel] (https://github.com/Gufdoor)