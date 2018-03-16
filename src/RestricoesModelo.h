#ifndef RestricoesModeloH
#define RestricoesModeloH

#include "ValoresLimites.h"

//================================ ESTRUTURAS ================================\\

//------------------------------------------------------------------------------
typedef struct tRestJanHor {
	int* coefMatX;		// matriz de coeficientes das vari�veis x de uma restri��o de janela de hor�rio
	int* coefMatZ;		// matriz de coeficientes das vari�veis z de uma restri��o de janela de hor�rio 
	int numCoefsNaoNulos;
}RestJanHor;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef struct tRestSalDif {
	int* coefMatX;		// matriz de coeficientes das vari�veis x de uma restri��o de salas diferentes
	int* coefMatY;		// matriz de coeficientes das vari�veis y de uma restri��o de salas diferentes
	int numCoefsNaoNulos;
}RestSalDif;
//------------------------------------------------------------------------------

//==============================================================================

#endif