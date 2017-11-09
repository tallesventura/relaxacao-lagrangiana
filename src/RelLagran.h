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

Solucao* execRelLagran(char* arq, double* vetAlpha, int tamAlpha, Instancia* inst, RestJanHor *vetRestJanHor, int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]);

#endif // RelLagran