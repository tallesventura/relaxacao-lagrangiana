#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>

//------------------------------------------------------------------------------
Solucao* execRelLagran(char* arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	Solucao *solRel, *solViav;
	Instancia *instRel, *instViav;
	double lb = -INFINITY;
	double ub = INFINITY;
	double FOAtual = INFINITY;
	double eta = 2.0;
	double gap;
	int numRest10 = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	double* subGradsRelRes10 = (double*)malloc(numRest10 * sizeof(double));
	double* subGradsRelRes14 = (double*)malloc(numRest14 * sizeof(double));
	double* subGradsRelRes15 = (double*)malloc(numRest15 * sizeof(double));
	double* subGradsViavRes10 = (double*)malloc(numRest10 * sizeof(double));
	double* subGradsViavRes14 = (double*)malloc(numRest14 * sizeof(double));
	double* subGradsViavRes15 = (double*)malloc(numRest15 * sizeof(double));
	int itSemMelhora = 0;
	int it = 1;
	
	printf("\n");

	do {

		instRel = clonarInstancia(inst);
		instViav = clonarInstancia(inst);

		// Resolver o problema relaxado
		//printf("Relaxando o modelo\n");
		relaxarModelo(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15);
		solRel = (Solucao*)execCpx(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15);
		calculaFO(solRel, instRel);

		if (solRel->funObj_ < FOAtual)
			itSemMelhora = 0;
		else
			itSemMelhora++;

		FOAtual = solRel->funObj_;

		// Viabilizar a solução
		solViav = clonarSolucao(solRel, instViav);
		viabilizaSol(solViav, instViav);
		calculaFO(solViav, instViav);

		// Calcular os limitantes
		double auxLB = lb;
		lb = MAX(auxLB, solRel->funObj_);
		double auxUB = ub;
		ub = MIN(auxUB, solViav->funObj_);

		printf("lb: MAX(%f, %f) = %f\n", auxLB, solRel->funObj_, lb);
		printf("ub: MIN(%f, %f) = %f\n", auxUB, solViav->funObj_, ub);

		gap = ub - lb;
		printf("gap: %f - %f = %f\n", ub, lb, gap);
		if (gap < 1.0) {
			break;
		}
			

		// Calcular os sub-gradientes
		getSubGradRest10(solRel, instRel, subGradsRelRes10);
		getSubGradRest14(solRel, instRel, subGradsRelRes14);
		getSubGradRest15(solRel, instRel, subGradsRelRes15);

		// Calcular o passo
		double passoRes10 = calculaPasso(eta, lb, ub, subGradsRelRes10, numRest10);
		double passoRes14 = calculaPasso(eta, lb, ub, subGradsRelRes14, numRest14);
		double passoRes15 = calculaPasso(eta, lb, ub, subGradsRelRes15, numRest15);

		// Atualizar os multiplicadores
		getSubGradRest10(solViav, instViav, subGradsViavRes10);
		atualizaMultMenIg(vetMultRes10, passoRes10, subGradsViavRes10, numRest10);
		getSubGradRest14(solViav, instViav, subGradsViavRes14);
		atualizaMultMenIg(vetMultRes14, passoRes14, subGradsViavRes14, numRest14);
		getSubGradRest15(solViav, instViav, subGradsViavRes15);
		atualizaMultMaiIg(vetMultRes15, passoRes15, subGradsViavRes15, numRest15);

		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}

		printf("Iteracao %d:\n", it);
		printf("FO solRel = %f\n", solRel->funObj_);
		printf("FO solViav = %f\n", solViav->funObj_);
		printf("gap = %f\n", gap);
		printf("eta = %f\n", eta);
		printf("itSemMelhora = %d\n", itSemMelhora);
		printf("-------------------------------------------\n");

		desalocaIntancia(instViav);
		printf("desalocou instViav\n");
		desalocaIntancia(instRel);
		printf("desalocou instRel\n");
		//desalocaSolucao(solRel);
		
	} while (eta > 0.005 && itSemMelhora < 50);

	return solViav;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void relaxarModelo(char *arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	montaVetCoefsFO(inst, vetMultRes10, vetMultRes14, vetMultRes15);
	montarModeloRelaxado(arq, inst, vetMultRes10, vetMultRes14, vetMultRes15);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void getSubGradRest10(Solucao* sol, Instancia* inst, double* vetSubGrad) {

	double soma;
	int pos = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			double soma = 0;
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[c][u] == 1)
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
				if (inst->matDisTur__[c][u] == 1)
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
					if (inst->matDisTur__[c][u] == 1)
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
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void getSubGradRest14(Solucao* sol, Instancia* inst, double* vetSubGrad) {

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
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void getSubGradRest15(Solucao* sol, Instancia* inst, double* vetSubGrad) {

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
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double calculaPasso(double eta, double lb, double ub, double* vetSubGrad, int tamVet) {

	double modulo = 0;
	for (int i = 0; i < tamVet; i++) {
		modulo += vetSubGrad[i] * vetSubGrad[i];
	}

	return (eta * (ub - lb)) / modulo;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atualizaMultMenIg(double* vetMult, double passo, double* subGrad, int tamVet) {

	double val;
	for (int i = 0; i < tamVet; i++) {
		val = vetMult[i];
		vetMult[i] = MIN(0, val + (passo * subGrad[i]));
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atualizaMultMaiIg(double* vetMult, double passo, double* subGrad, int tamVet) {

	double val;
	for (int i = 0; i < tamVet; i++) {
		val = vetMult[i];
		vetMult[i] = MAX(0, val + (passo * subGrad[i]));
	}
}
//------------------------------------------------------------------------------