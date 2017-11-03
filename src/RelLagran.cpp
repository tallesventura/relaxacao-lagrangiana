#include "RelLagran.h"
//#include "uCBCTT.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <math.h>

#include <algorithm>

#include "..\lib\cplex\include\cplex.h"

// coefs originais da FO, 
Solucao* execRelLagran(char* arq, double* vetAlpha, int tamAlpha, Instancia* inst, RestJanHor *vetRestJanHor, int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]) {

	Solucao *solRel, *bestSol;
	double lb = -INFINITY;
	double ub = INFINITY;
	double eta = 2.0;
	double gap = 100;
	// parar quando eta < 0.001

	// subgradientes
	while (gap > 2.0) {
		solRel = (Solucao*) execCpx(arq, inst);

		// Gerar solucao vi�vel

		lb = std::max(lb, solRel->valSol_);

		if (eta <= 0.001)
			break;
	}


	return bestSol;
}
	
