#ifndef RestricoesModeloH
#define RestricoesModeloH

#include "ValoresLimites.h"

//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tRestJanHor {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das variáveis x de uma restrição de janela de horário
	int coefMatZ[MAX_TUR*MAX_DIA*MAX_PER];		   // matriz de coeficientes das variáveis z de uma restrição de janela de horário (lado esquerdo)
	int coefZ = 1;                                 // Valor do coeficiente de Z da restrição (lado direito da inequação)
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das variáveis x de uma restrição de salas diferentes
	int coefMatY[MAX_SAL*MAX_DIS];			       // matriz de coeficientes das variáveis y de uma restrição de salas diferentes (lado esquerdo)
	int coefY = 1;								   // Valor do coeficiente de Y da restrição (lado direito da inequação)
}RestSalDif;
//------------------------------------------------------------------------------

//==============================================================================

#endif