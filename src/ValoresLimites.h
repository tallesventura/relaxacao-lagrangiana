#ifndef ValoresLimitesH
#define ValoresLimitesH

// Valores limites baseados nas instâncias comp
#define MAX_DIS 135  // 131
#define MAX_TUR 150  // 150
#define MAX_PRO 135  // 135
#define MAX_SAL 20   // 20
#define MAX_DIA 6    // 6
#define MAX_PER 9    // 6
#define MAX_RES 1400 // 1368
#define MAX_COL 50000
#define MAX_VAR 71000 //70832

#define MAX_REST10  MAX_TUR * MAX_DIA * MAX_PER;
#define MAX_REST14  MAX_PER * MAX_DIA * MAX_SAL * MAX_DIS;
#define MAX_REST15  MAX_SAL * MAX_DIS;

int const PESOS[4] = { 1,2,5,1 };

#endif