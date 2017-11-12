#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>

 // coefs originais da FO, 
Solucao* execRelLagran(char* arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	Solucao *solRel, *bestSol;
	double lb = -INFINITY;
	double ub = INFINITY;
	double eta = 2.0;
	double gap = 100;
	int numRestJanHor = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int it = 1;
	
	// parar quando eta < 0.001

	// subgradientes
	while (gap > 2.0) {
		solRel = (Solucao*)execCpx(arq, inst);

		Solucao *solViav = clonarSolucao(solRel, inst);
		viabilizaSol(solViav, inst);
		calculaFO(solViav, inst);

		lb = MAX(lb, solRel->funObj_);
		ub = MIN(ub, solViav->funObj_);


		// Gerar solucao viável

		lb = MAX(lb, solRel->valSol_);

		if (eta <= 0.001)
			break;
	}


	return bestSol;
}

double* getSubGradRest10(Solucao* sol, Instancia* inst) {

	int numRestJanHor = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int posX;
	double* vetSubGrad = (double*)malloc(numRestJanHor * sizeof(double));

	double soma;
	for (int i = 0; i < numRestJanHor; i++) {
		soma = 0;
		
	}


}