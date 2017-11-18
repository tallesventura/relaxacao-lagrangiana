#ifndef RelLagranH
#define RelLagranH

#include "uCBCTT.h"
#include "Solucao.h"
#include "ValoresLimites.h"

//#define REL_CAP_SAL // Relaxar restri��o soft de capacidade das salas
#define	REL_JAN_HOR // Relaxar restri��o soft de janelas de hor�rios
//#define REL_DIA_MIN // Relaxar restri��o soft de dias m�nimos
#define REL_SAL_DIF // Relaxar restri��o soft de salas diferentes

#define VAL_INIT_ALPHA 0  // Valor usado para inicializar os valores dos multiplicadores alpha de lagrange (JanHor) (<= 0)
#define VAL_INIT_RES_14 0 // Valor usado para inicializar os valores dos multiplicadores de lagrange da restri��o 14 (SalDif) (<= 0)
#define VAL_INIT_RES_15 0 // Valor usado para inicializar os valores dos multiplicadores de lagrange da restri��o 15 (SalDif) (>= 0)


// M�TODOS ===================================================================================

Solucao* execRelLagran(char* arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15);
void relaxarModelo(char *arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15);
double* getSubGradRest10(Solucao* sol, Instancia* inst);
double* getSubGradRest14(Solucao* sol, Instancia* inst);
double* getSubGradRest15(Solucao* sol, Instancia* inst);
double calculaPasso(double eta, double lb, double ub, double* vetSubGrad, int tamVet);
void atualizaMultMenIg(double* vetMult, double passo, double* subGrad, int tamVet);
void atualizaMultMaiIg(double* vetMult, double passo, double* subGrad, int tamVet);
double* juntaVetsSubGrad(double* vetSubGrad10, double* vetSubGrad14, double* vetSubGrad15, int tamSubGra10, int tamSubGra14, int tamSubGra15);

#endif // RelLagran