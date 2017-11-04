#ifndef RelLagranH
#define RelLagranH

#include "uCBCTT.h"

#define VAL_INIT_ALPHA 0 // Valor usado para inicializar os valores dos multiplicadores de lagrange


// MÉTODOS ===================================================================================

Solucao* execRelLagran(char* arq, double* vetAlpha, int tamAlpha, Instancia* inst, RestJanHor *vetRestJanHor, int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]);

#endif // RelLagran