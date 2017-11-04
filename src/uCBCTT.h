#ifndef uCBCTTH
#define uCBCTTH

#include "..\lib\cplex\include\cplex.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

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
 int numPer_;    // n�mero de per�odos de oferta (n�mero de aulas)
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
typedef struct tRestJanHor {
	int coefMatX[MAX_PER*MAX_DIA][MAX_SAL][MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de janela de hor�rio
	int coefMatZ[MAX_TUR][MAX_DIA][MAX_PER];		 // matriz de coeficientes das vari�veis z de uma restri��o de janela de hor�rio
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int coefMatX[MAX_PER*MAX_DIA][MAX_SAL][MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de salas diferentes
	int coefMatY[MAX_SAL][MAX_DIS];			 // matriz de coeficientes das vari�veis y de uma restri��o de salas diferentes
}RestSalDif;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tSolucao
{
 // restri��es SOFT
 int capSal_;    // n�mero de aulas que violam a capacidade da sala
 int janHor_;    // n�mero de janelas no hor�rio das turmas
 int diaMin_;    // n�mero "dias m�nimos" n�o respeitados
 int salDif_;    // n�mero de salas diferentes usadas para as disciplinas

 // FO
 int funObj_;    // valor da fun��o objetivo

 // restri��es HARD 
 int vioNumAul_; // viola��es na restri��o 1 - n�mero de aulasA
 int vioAulSim_; // viola��es na restri��o 2 - aulas simultaneas
 int vioDisSim_; // viola��es na restri��o 3 - disciplinas simultaneas (Duas aulas da mesma disciplina em um mesmo per�odo)
 int vioProSim_; // viola��es na restri��o 4 - professores simultaneos
 int vioTurSim_; // viola��es na restri��o 5 - turmas simultaneas

 // CPLEX
 int numVar_;
 int numRes_;
 double* vetViab_;  // vetor que armazena se as restri��es foram satisfeitas
 double valSol_; // valor da solu��o obtida pelo CPLEX
 double bstNod_; // valor do melhor n� (lower bound)
 double tempo_;  // tempo de execu��o

 // vetor de solu��o (vari�veis x)
 double vetSol_[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS];
 // Matriz de solu��o (vari�veis z)
 double vetSolZ_[MAX_TUR*MAX_DIA*MAX_PER];
 // vetor de solu��o (vari�veis q)
 double vetSolQ_[MAX_DIS];
 // vetor de solu��o (vari�veis z)
 double vetSolY_[MAX_DIS*MAX_SAL];
 // vetor de solu��o (vari�veis v)
 double vetSolV_[MAX_DIS*MAX_DIA];

 // matriz de solu��o (per�odo x dia x sala)
 int matSolSal_[MAX_PER][MAX_DIA][MAX_SAL];

 // matriz de solu��o (per�odo x dia x turma)
 int matSolTur_[MAX_PER][MAX_DIA][MAX_TUR];
}Solucao;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tInstancia {
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
}Instancia;
//------------------------------------------------------------------------------

//==============================================================================


//===================================== M�TODOS ================================
void montarModeloPLI(char *arq, Instancia* inst);
Instancia* lerInstancia(char *arq);

Solucao* execCpx(char *arq, Instancia* inst);
void escreverSol(Solucao* s, char *arq, Instancia* inst);

void execUma(char* nomeInst);
void execTodas();
void initRestJanHor(RestJanHor *rest);
RestJanHor* getVetJanHor(Instancia* inst, int numRest);
void initRestSalDif(RestSalDif *rest);
RestSalDif* getVetSalDif(Instancia *inst, int numRest);
void montaMatCoefXFO(Instancia* inst);
void montaCoefRestJanHor(Instancia* inst);
void montaCoefRestSalDif(Instancia* inst);
void montarModeloRelaxado(char *arq, Instancia* inst, double* vetAlpha, double* vetMultRes14, double* vetMultRes15);
void initMultiplicadores(double* vetMult, int tam, double val);
int getVetViabJanHor(Solucao* sol, double* vetViabJanHor, Instancia* inst);
void getValSol(Solucao *sol, CPXENVptr env, CPXLPptr lp, Instancia* inst);
//==============================================================================

#endif
