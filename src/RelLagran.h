#ifndef RelLagranH
#define RelLagranH

#include "uCBCTT.h"
#include "Solucao.h"
#include "ValoresLimites.h"

//#define REL_CAP_SAL // Relaxar restrição soft de capacidade das salas
#define	REL_JAN_HOR // Relaxar restrição soft de janelas de horários
//#define REL_DIA_MIN // Relaxar restrição soft de dias mínimos
#define REL_SAL_DIF // Relaxar restrição soft de salas diferentes

#define VAL_INIT_ALPHA 0  // Valor usado para inicializar os valores dos multiplicadores alpha de lagrange (JanHor) (<= 0)
#define VAL_INIT_RES_14 0 // Valor usado para inicializar os valores dos multiplicadores de lagrange da restrição 14 (SalDif) (<= 0)
#define VAL_INIT_RES_15 0 // Valor usado para inicializar os valores dos multiplicadores de lagrange da restrição 15 (SalDif) (>= 0)


// MÉTODOS ===================================================================================

Solucao* execRelLagran(char* arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15);
void getSubGradRest10(Solucao* sol, Instancia* inst, double* vetSubGrad);
void getSubGradRest14(Solucao* sol, Instancia* inst, double* vetSubGrad);
void getSubGradRest15(Solucao* sol, Instancia* inst, double* vetSubGrad);
double calculaPasso(double eta, double lb, double ub, double* vetSubGrad, int tamVet);
void atualizaMultMenIg(double* vetMult, double passo, double* subGrad, int tamVet);
void atualizaMultMaiIg(double* vetMult, double passo, double* subGrad, int tamVet);

#endif // RelLagran