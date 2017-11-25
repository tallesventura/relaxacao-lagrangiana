#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

//------------------------------------------------------------------------------
Solucao* execRelLagran(char* arq, Instancia* instOrig, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	Solucao *solRel, *solViav, *bestSol;
	Instancia* instRel;
	double lb = -INFINITY;
	double ub = INFINITY;
	double eta = 2.0;
	double lastFO = -INFINITY;

	double gap;
	int numRest10 = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numRest14 = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numRest15 = instOrig->numSal__ * instOrig->numDis__;
	printf("%d, %d, %d", numRest10, numRest14, numRest15);

	int itSemMelhora = 0;
	int it = 0;
	
	printf("\n");

	do {

		printf("Clonando instância\n");
		instRel = clonarInstancia(instOrig);

		// Resolver o problema relaxado
		printf("Relaxando o modelo\n");
		relaxarModelo(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15);
		printf("Executando CPX\n");
		solRel = (Solucao*)execCpx(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15);
		printf("Calculando FO\n");
		calculaFO(solRel, instRel);

		// Viabilizar a solução
		printf("Clonando solucao\n");
		solViav = clonarSolucao(solRel, instRel);
		printf("Viabilizando solucao\n");
		viabilizaSol(solViav, instOrig);
		calculaFO(solViav, instOrig);

		if (solRel->funObj_ > lb) {
			itSemMelhora = 0;
		}
		else {
			itSemMelhora++;
		}

		if (solRel->funObj_ > lb) {
			bestSol = clonarSolucao(solViav, instOrig);
		}

		// Calcular os limitantes
		double auxLB = lb;
		lb = MAX(auxLB, solRel->funObj_);
		double auxUB = ub;
		ub = MIN(auxUB, solViav->funObj_);

		printf("lb: MAX(%f, %f) = %f\n", auxLB, solRel->funObj_, lb);
		printf("ub: MIN(%f, %f) = %f\n", auxUB, solViav->funObj_, ub);

		gap = fabs(ub - lb);
		printf("gap: %f - %f = %f\n", ub, lb, gap);
		if (gap < 1.0) {
			break;
		}

		// Calcular os sub-gradientes
		printf("Calculando os sub-gradientes\n");
		double* subGradsRes10 = (double*) getSubGradRest10(solRel, instRel);
		double* subGradsRes14 = (double*) getSubGradRest14(solRel, instRel);
		double* subGradsRes15 = (double*) getSubGradRest15(solRel, instRel);

		printf("juntando subgradientes\n");
		double* vetSubGrads = juntaVetsSubGrad(subGradsRes10, subGradsRes14, subGradsRes15, numRest10, numRest14, numRest15);

		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}

		printf("calculando o passo\n");
		// Calcular o passo
		double passo = calculaPasso(eta, lb, ub, vetSubGrads, numRest10 + numRest14 + numRest15);

		printf("atualizando multiplicadores\n");
		// Atualizar os multiplicadores
		atualizaMultMenIg(vetMultRes10, passo, subGradsRes10, numRest10);
		atualizaMultMenIg(vetMultRes14, passo, subGradsRes14, numRest14);
		atualizaMultMaiIg(vetMultRes15, passo, subGradsRes15, numRest15);

		printf("FO solRel = %f\n", solRel->funObj_);
		printf("FO solViav = %f\n", solViav->funObj_);
		printf("gap = %f\n", gap);
		printf("eta = %f\n", eta);
		printf("passo = %f\n", passo);
		printf("itSemMelhora = %d\n", itSemMelhora);
		printf("-------------------------------------------\n");

		char dest[50];
		char* aux = "toy3_debug_it-";
		strcpy_s(dest, aux);
		char nomeit[10];
		strcat_s(dest, itoa(it, nomeit, 10));
		strcat_s(dest, ".csv");
		printf("%s\n", dest);

		printf("========================================================\n");
		printCoefsFO(instOrig);
		printf("========================================================\n");

		double** matD = montaMatD(instRel);
		double* vetD = montaVetD(instRel);
		escreveCSVDebugCoefs(dest, instRel, matD, vetD);
	
		free(vetD);

		lastFO = solRel->funObj_;

		desalocaIntancia(instRel);
		desalocaSolucao(solRel);
		free(subGradsRes10);
		free(subGradsRes14);
		free(subGradsRes15);
		free(vetSubGrads);

		it++;

	} while (eta > 0.005 && it < 3);

	return bestSol;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void relaxarModelo(char *arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	// Conferir
	montaVetCoefsFO(inst, vetMultRes10, vetMultRes14, vetMultRes15);
	montarModeloRelaxado(arq, inst, vetMultRes10, vetMultRes14, vetMultRes15);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest10(Solucao* sol, Instancia* inst) {

	int numRest10 = inst->numTur__*inst->numDia__*inst->numPerDia__;
	double* vetSubGrad = (double*)malloc(numRest10 * sizeof(double));

	double soma;
	int pos = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			double soma = 0;
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						int prim = d*inst->numPerDia__;
						int seg = (d*inst->numPerDia__) + 1;
						soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
					}
				}
			double z = sol->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)];
			vetSubGrad[pos] = soma - z;

			pos++;
		}
	}

	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			double soma = 0;
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						int prim = (d*inst->numPerDia__) + inst->numPerDia__ - 1;
						int seg = (d*inst->numPerDia__) + inst->numPerDia__ - 2;
						soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
					}
				}
			double z = sol->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)];
			vetSubGrad[pos] = soma - z;

			pos++;
		}
	}

	for (int s = 2; s < inst->numPerDia__; s++)
	{
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				double soma = 0;
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++) {
							int prim = (d*inst->numPerDia__) + s - 1;
							int seg = (d*inst->numPerDia__) + s - 2;
							int ter = (d*inst->numPerDia__) + s;
							soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)] -
								sol->vetSol_[offset3D(r, ter, c, inst->numPerTot__, inst->numDis__)];
						}
					}
				double z = sol->vetSolZ_[offset3D(u, d, s, inst->numDia__, inst->numPerDia__)];
				vetSubGrad[pos] = soma - z;

				pos++;
			}
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest14(Solucao* sol, Instancia* inst) {

	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	double* vetSubGrad = (double*)malloc(numRest14 * sizeof(double));

	int pos = 0;
	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {
				double x = sol->vetSol_[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)];
				double y = sol->vetSolY_[offset2D(c, r, inst->numSal__)];
				vetSubGrad[pos] = x - y;
				pos++;
			}
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest15(Solucao* sol, Instancia* inst) {

	int numRest15 = inst->numSal__ * inst->numDis__;
	double* vetSubGrad = (double*)malloc(numRest15 * sizeof(double));

	int pos = 0;
	for (int r = 0; r < inst->numSal__; r++) {
		for (int c = 0; c < inst->numDis__; c++) {

			double soma = 0;
			for (int p = 0; p < inst->numPerTot__; p++) {
				soma += sol->vetSol_[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)];
			}

			double y = sol->vetSolY_[offset2D(c, r, inst->numSal__)];
			vetSubGrad[pos] = soma - y;

			pos++;
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double calculaPasso(double eta, double lb, double ub, double* vetSubGrad, int tamVet) {

	double modulo = 0;
	for (int i = 0; i < tamVet; i++) {
		modulo += vetSubGrad[i] * vetSubGrad[i];
	}
	//printf("MODULO: %f\n", modulo);
	return (eta * (ub - lb)) / modulo;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atualizaMultMenIg(double* vetMult, double passo, double* subGrad, int tamVet) {

	double val;
	for (int i = 0; i < tamVet; i++) {
		val = vetMult[i];
		vetMult[i] = MIN(0, val - (passo * subGrad[i]));
		//printf("MIN(0, %.3f + (%.3f * %.3f))\n", val, passo, subGrad[i]);
		if (vetMult[i] > val) {
			//printf("MAIOR\n");
			//printf("MENOR\n");
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atualizaMultMaiIg(double* vetMult, double passo, double* subGrad, int tamVet) {

	double val;
	for (int i = 0; i < tamVet; i++) {
		val = vetMult[i];
		vetMult[i] = MAX(0, val - (passo * subGrad[i]));
		//printf("MAX(0, %.3f + (%.3f * %.3f))\n", val, passo, subGrad[i]);
		if (vetMult[i] < val) {
			//printf("MENOR\n");
			//printf("MAIOR\n");
		}
	}
}
//------------------------------------------------------------------------------

double* juntaVetsSubGrad(double* vetSubGrad10, double* vetSubGrad14, double* vetSubGrad15, int tamSubGrad10, int tamSubGrad14, int tamSubGrad15) {

	int pos = 0;
	double* vetFinal = (double*)malloc((tamSubGrad10 + tamSubGrad14 + tamSubGrad15) * sizeof(double));

	// Restrição 10
	for (int i = 0; i < tamSubGrad10; i++) {
		vetFinal[pos] = vetSubGrad10[i];
		pos++;
	}

	// Restrição 14
	for (int i = 0; i < tamSubGrad14; i++) {
		vetFinal[pos] = vetSubGrad14[i];
		pos++;
	}

	// Restrição 15
	for (int i = 0; i < tamSubGrad15; i++) {
		vetFinal[pos] = vetSubGrad15[i];
		pos++;
	}

	return vetFinal;
}

