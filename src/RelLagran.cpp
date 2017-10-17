#include "RelLagran.h"
#include "uCBCTT.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include "..\lib\cplex\include\cplex.h"

double *vetAlpha;	// Vetor com os multiplicadores de Lagrange

void initMultiplicadores(double* mult, int numRest) {

	for (int i = 0; i < numRest; i++) {
		mult[i] = 0.0;
	}
}

void montarModeloRelaxado(char *arq, int numRest, int coefMatXFO, RestJanHor *vetRestJanHor) {


}

	
