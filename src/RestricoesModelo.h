#ifndef RestricoesModeloH
#define RestricoesModeloH

#include "ValoresLimites.h"

//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tRestJanHor {
	int* coefMatX;		// matriz de coeficientes das variáveis x de uma restrição de janela de horário
	int* coefMatZ;		// matriz de coeficientes das variáveis z de uma restrição de janela de horário 
	int numCoefsNaoNulos;
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int* coefMatX;		// matriz de coeficientes das variáveis x de uma restrição de salas diferentes
	int* coefMatY;		// matriz de coeficientes das variáveis y de uma restrição de salas diferentes
	int numCoefsNaoNulos;
}RestSalDif;
//------------------------------------------------------------------------------

//==============================================================================

#endif