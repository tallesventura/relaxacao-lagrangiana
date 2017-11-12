#ifndef RestricoesModeloH
#define RestricoesModeloH

#include "ValoresLimites.h"

//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tRestJanHor {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de janela de hor�rio
	int coefMatZ[MAX_TUR*MAX_DIA*MAX_PER];		   // matriz de coeficientes das vari�veis z de uma restri��o de janela de hor�rio (lado esquerdo)
	int coefZ = 1;                                 // Valor do coeficiente de Z da restri��o (lado direito da inequa��o)
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int coefMatX[MAX_PER*MAX_DIA*MAX_SAL*MAX_DIS]; // matriz de coeficientes das vari�veis x de uma restri��o de salas diferentes
	int coefMatY[MAX_SAL*MAX_DIS];			       // matriz de coeficientes das vari�veis y de uma restri��o de salas diferentes (lado esquerdo)
	int coefY = 1;								   // Valor do coeficiente de Y da restri��o (lado direito da inequa��o)
}RestSalDif;
//------------------------------------------------------------------------------

//==============================================================================

#endif