#ifndef uCBCTTH
#define uCBCTTH

#define PATH_INST "instances\\"
#define NUM_INST 21 // número de instâncias

// Valores limites baseados nas instâncias comp
#define MAX_DIS 135  // 131
#define MAX_TUR 150  // 150
#define MAX_PRO 135  // 135
#define MAX_SAL 20   // 20
#define MAX_DIA 6    // 6
#define MAX_PER 6    // 6
#define MAX_RES 1400 // 1368
#define MAX_COL 50000
#define MAX_VAR 71000 //70832

//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tDisciplina
{
 char nome_[50]; // nome da disciplina
 int professor_; // id do professor
 int numPer_;    // número de períodos de oferta
 int diaMin_;    // dias mínimos para distribuição
 int numAlu_;    // número de alunos
}Disciplina;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tTurma
{
 char nome_[50];       // nome da turma
 int numDis_;          // número de disciplinas
 int vetDis_[MAX_DIS]; // vetor com as disciplinas (ids)
}Turma;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tProfessor
{
 char nome_[50];       // nome da turma
}Professor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tSala
{
 char nome_[50];  // nome da sala
 int capacidade_; // capacidade
}Sala;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestricao
{
 int disciplina_; // id da disciplina
 int periodo_;    // id do período
}Restricao;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tSolucao
{
 // FO
 int capSal_;    // número de aulas que violam a capacidade da sala
 int janHor_;    // número de janelas no horário das turmas
 int diaMin_;    // número "dias mínimos" não respeitados
 int salDif_;    // número de salas diferentes usadas para as disciplinas
 int funObj_;    // valor da função objetivo

 // restrições SOFT 
 int vioNumAul_; // violações na restrição 1 - número de aulasA
 int vioAulSim_; // violações na restrição 2 - aulas simultaneas
 int vioDisSim_; // violações na restrição 3 - disciplinas simultaneas
 int vioProSim_; // violações na restrição 4 - professores simultaneos
 int vioTurSim_; // violações na restrição 5 - turmas simultaneas

 // CPLEX
 int numVar_;
 int numRes_;
 double valSol_; // valor da solução obtida pelo CPLEX
 double bstNod_; // valor do melhor nó (lower bound)
 double tempo_;  // tempo de execução

 // vetor de solução (variáveis x)
 double vetSol_[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS];

 // matriz de solução (período x dia x sala)
 int matSolSal_[MAX_PER][MAX_DIA][MAX_SAL];

 // matriz de solução (período x dia x turma)
 int matSolTur_[MAX_PER][MAX_DIA][MAX_TUR];
}Solucao;
//------------------------------------------------------------------------------

//==============================================================================



//============================= VARIÁVEIS GLOBAIS ==============================

// ------------ Dados de entrada
char nomInst__[150]; // nome da instância
int numDis__;        // número de disciplinas 
int numTur__;        // número de turmas
int numPro__;        // número de professores
int numSal__;        // número de salas
int numDia__;        // número de dias
int numPerDia__;     // número de períodos por dia
int numPerTot__;     // número de períodos total
int numRes__;        // número de restrições
int numSol__;        // número de colunas
int numVar__;        // número de variáveis
Disciplina vetDisciplinas__[MAX_DIS];
Turma vetTurmas__[MAX_TUR];
Professor vetProfessores__[MAX_PRO];
Sala vetSalas__[MAX_SAL];
Restricao vetRestricoes__[MAX_RES];

// ------------ CPLEX


// ------------ Auxiliares
int matDisTur__[MAX_DIS][MAX_TUR];
int **matGCInicial__;

//==============================================================================



//===================================== MÉTODOS ================================
void montarModeloPLI(char *arq);
void lerInstancia(char *arq);
void lerSolucoesIniciais();
void desaloca();
void testarEntrada();

void execCpx(Solucao &s,char *arq);
void escreverSol(Solucao &s,char *arq);

void execUma(char* nomeInst);
void execTodas();


//==============================================================================

#endif
