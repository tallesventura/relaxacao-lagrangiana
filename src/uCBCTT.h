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
 // vetor de solu��o (vari�veis z)
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
void initRestJanHor(RestJanHor *rest);
void initVetJanHor();
void montaMatCoefXFO();
void montaCoefRestJanHor();
void montarModeloRelaxado(char *arq);
void initMultiplicadores();
int ehViavel(Solucao* s);
void getValSol(Solucao &s, CPXENVptr env, CPXLPptr lp);
//==============================================================================

#endif
