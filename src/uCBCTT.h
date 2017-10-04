#ifndef uCBCTTH
#define uCBCTTH

#define PATH_INST "instances\\"
#define NUM_INST 21 // n�mero de inst�ncias

// Valores limites baseados nas inst�ncias comp
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
 int numPer_;    // n�mero de per�odos de oferta
 int diaMin_;    // dias m�nimos para distribui��o
 int numAlu_;    // n�mero de alunos
}Disciplina;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tTurma
{
 char nome_[50];       // nome da turma
 int numDis_;          // n�mero de disciplinas
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
 int periodo_;    // id do per�odo
}Restricao;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tSolucao
{
 // FO
 int capSal_;    // n�mero de aulas que violam a capacidade da sala
 int janHor_;    // n�mero de janelas no hor�rio das turmas
 int diaMin_;    // n�mero "dias m�nimos" n�o respeitados
 int salDif_;    // n�mero de salas diferentes usadas para as disciplinas
 int funObj_;    // valor da fun��o objetivo

 // restri��es SOFT 
 int vioNumAul_; // viola��es na restri��o 1 - n�mero de aulasA
 int vioAulSim_; // viola��es na restri��o 2 - aulas simultaneas
 int vioDisSim_; // viola��es na restri��o 3 - disciplinas simultaneas
 int vioProSim_; // viola��es na restri��o 4 - professores simultaneos
 int vioTurSim_; // viola��es na restri��o 5 - turmas simultaneas

 // CPLEX
 int numVar_;
 int numRes_;
 double valSol_; // valor da solu��o obtida pelo CPLEX
 double bstNod_; // valor do melhor n� (lower bound)
 double tempo_;  // tempo de execu��o

 // vetor de solu��o (vari�veis x)
 double vetSol_[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS];

 // matriz de solu��o (per�odo x dia x sala)
 int matSolSal_[MAX_PER][MAX_DIA][MAX_SAL];

 // matriz de solu��o (per�odo x dia x turma)
 int matSolTur_[MAX_PER][MAX_DIA][MAX_TUR];
}Solucao;
//------------------------------------------------------------------------------

//==============================================================================



//============================= VARI�VEIS GLOBAIS ==============================

// ------------ Dados de entrada
char nomInst__[150]; // nome da inst�ncia
int numDis__;        // n�mero de disciplinas 
int numTur__;        // n�mero de turmas
int numPro__;        // n�mero de professores
int numSal__;        // n�mero de salas
int numDia__;        // n�mero de dias
int numPerDia__;     // n�mero de per�odos por dia
int numPerTot__;     // n�mero de per�odos total
int numRes__;        // n�mero de restri��es
int numSol__;        // n�mero de colunas
int numVar__;        // n�mero de vari�veis
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



//===================================== M�TODOS ================================
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
