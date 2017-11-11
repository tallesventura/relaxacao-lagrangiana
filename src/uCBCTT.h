#ifndef uCBCTTH
#define uCBCTTH

#include "Solucao.h"
#include "ValoresLimites.h"
#include "Instancia.h"
#include "RestricoesModelo.h"

#include "..\lib\cplex\include\cplex.h"


#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define PATH_INST "instances\\"
#define NUM_INST 21 // número de instâncias

//===================================== MÉTODOS ================================
void montarModeloPLI(char *arq, Instancia* inst);

Solucao* execCpx(char *arq, Instancia* inst);

void execUma(char* nomeInst);
void execTodas();
void montarModeloPLI(char *arq, Instancia* inst);
void montarModeloRelaxado(char *arq, Instancia* inst, double* vetAlpha, double* vetMultRes14, double* vetMultRes15);
void initMultiplicadores(double* vetMult, int tam, double val);
int getVetViabJanHor(Solucao* sol, Instancia* inst);
int getViabSalDif14(Solucao* sol, Instancia* inst);
int getViabSalDif15(Solucao* sol, Instancia* inst);
void getValSol(Solucao *sol, CPXENVptr env, CPXLPptr lp, Instancia* inst);
void exportarCsv(Solucao* sol, char *arq, Instancia* inst);
//==============================================================================

#endif
