#ifndef uCBCTTH
#define uCBCTTH

#include "Solucao.h"
#include "ValoresLimites.h"
#include "Instancia.h"

#include "..\lib\cplex\include\cplex.h"


#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define PATH_INST "instances\\"
#define NUM_INST 21 // n�mero de inst�ncias


//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tRestJanHor {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de janela de hor�rio
	int coefMatZ[MAX_TUR*MAX_DIA*MAX_PER];		   // matriz de coeficientes das vari�veis z de uma restri��o de janela de hor�rio
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de salas diferentes
	int coefMatY[MAX_SAL*MAX_DIS];			       // matriz de coeficientes das vari�veis y de uma restri��o de salas diferentes
}RestSalDif;
//------------------------------------------------------------------------------

//==============================================================================


//===================================== M�TODOS ================================
void montarModeloPLI(char *arq, Instancia* inst);

Solucao* execCpx(char *arq, Instancia* inst);

void execUma(char* nomeInst);
void execTodas();
void initRestJanHor(RestJanHor *rest, Instancia* inst);
RestJanHor* getVetJanHor(Instancia* inst, int numRest);
void initRestSalDif(RestSalDif *rest, Instancia* inst);
RestSalDif* getVetSalDif(Instancia *inst, int numRest);
void montaMatCoefXFO(Instancia* inst);
void montaCoefRestJanHor(Instancia* inst);
void montaCoefRestSalDif(Instancia* inst);
void montarModeloRelaxado(char *arq, Instancia* inst, double* vetAlpha, double* vetMultRes14, double* vetMultRes15);
void initMultiplicadores(double* vetMult, int tam, double val);
int getVetViabJanHor(Solucao* sol, double* vetViabJanHor, Instancia* inst);
int getViabSalDif14(Solucao* sol, double* vetViab, Instancia* inst);
int getViabSalDif15(Solucao* sol, double* vetViab, Instancia* inst);
void getValSol(Solucao *sol, CPXENVptr env, CPXLPptr lp, Instancia* inst);
void exportarCsv(Solucao* sol, char *arq, Instancia* inst);
//==============================================================================

#endif
