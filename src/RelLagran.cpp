#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>

//------------------------------------------------------------------------------
Solucao* execRelLagran(char* arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	Solucao *solRel, *solViav;
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
	
	// parar quando eta < 0.001

	do {

		// Resolver o problema relaxado
		solRel = (Solucao*)execCpx(arq, inst);
		calculaFO(solRel, inst);

		if (solRel->funObj_ < FOAtual)
			itSemMelhora = 0;
		else
			itSemMelhora++;

		FOAtual = solRel->funObj_;

		// Viabilizar a solução
		solViav = clonarSolucao(solRel, inst);
		viabilizaSol(solViav, inst);
		calculaFO(solViav, inst);

		// Calcular os limitantes
		lb = MAX(lb, solRel->funObj_);
		ub = MIN(ub, solViav->funObj_);

		gap = ((solViav->funObj_ - solRel->funObj_) / solViav->funObj_) * 100;
		if (gap < 1)
			break;

		// Calcular os sub-gradientes
		getSubGradRest10(solRel, inst, subGradsRelRes10);
		getSubGradRest14(solRel, inst, subGradsRelRes14);
		getSubGradRest15(solRel, inst, subGradsRelRes15);

		// Calcular o passo
		double passoRes10 = calculaPasso(eta, lb, ub, subGradsRelRes10, numRest10);
		double passoRes14 = calculaPasso(eta, lb, ub, subGradsRelRes14, numRest14);
		double passoRes15 = calculaPasso(eta, lb, ub, subGradsRelRes15, numRest15);

		// Atualizar os multiplicadores
		getSubGradRest10(solViav, inst, subGradsViavRes10);
		atualizaMultMenIg(vetMultRes10, passoRes10, subGradsViavRes10, numRest10);
		getSubGradRest14(solViav, inst, subGradsViavRes14);
		atualizaMultMenIg(vetMultRes14, passoRes14, subGradsViavRes14, numRest14);
		getSubGradRest15(solViav, inst, subGradsViavRes15);
		atualizaMultMaiIg(vetMultRes15, passoRes15, subGradsViavRes15, numRest15);

		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}
		
	} while (eta > 0.001 && itSemMelhora < 50);

	return solViav;
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
				sol->vetViabJanHor_[pos] = soma <= z ? 0 : soma - z;
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

	return vetSubGrad;
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