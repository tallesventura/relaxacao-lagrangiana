#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

//------------------------------------------------------------------------------
Solucao* execRelLagran(char* arq, Instancia* instOrig, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest) {

	Solucao *solRel, *solViav, *bestSol;
	Instancia* instRel;
	double lb = -INFINITY;
	double ub = INFINITY;
	double eta = 2.0;

	double gap, passo;
	int numRest10 = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numRest14 = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numRest15 = instOrig->numSal__ * instOrig->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;
	printf("%d, %d, %d", numRest10, numRest14, numRest15);

	int itSemMelhora = 0;
	int it = 0;
	
	printf("\n");

	clock_t h;
	h = clock();

	do {

		printf("Clonando inst�ncia\n");
		instRel = clonarInstancia(instOrig);

		// Resolver o problema relaxado
		printf("Relaxando o modelo\n");
		relaxarModelo(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15, rest);
		printf("Executando CPX\n");
		solRel = (Solucao*)execCpx(arq, instRel, vetMultRes10, vetMultRes14, vetMultRes15);
		printf("\n");

		// ==================== DEBUG =============================================================
		/*printf("========================================================\n");
		//printCoefsFO(instRel);
		printf("\n========================================================\n");
		char dest[50];
		char* aux = "toy3_debug_it-";
		strcpy_s(dest, aux);
		char nomeit[10];
		strcat_s(dest, itoa(it, nomeit, 10));
		strcat_s(dest, ".csv");
		printf("%s\n", dest);

		debugaCoeficientes(dest, instRel, solRel, vetMultRes10, vetMultRes14, vetMultRes15);*/
		// ==================== DEBUG =============================================================

		// Viabilizar a solu��o
		printf("Clonando solucao\n");
		solViav = clonarSolucao(solRel, instRel);
		printf("Viabilizando solucao\n");
		viabilizaSol(solViav, instOrig);
		calculaFO(solViav, instOrig);

		if (solRel->funObj_ > lb) {
			itSemMelhora = 0;
			bestSol = clonarSolucao(solViav, instOrig);
		}
		else {
			itSemMelhora++;
		}

		// Calcular os limitantes
		double auxLB = lb;
		lb = MAX(auxLB, solRel->funObj_);
		double auxUB = ub;
		ub = MIN(auxUB, solViav->funObj_);
		//ub = 5;

		printf("lb: MAX(%f, %f) = %f\n", auxLB, solRel->funObj_, lb);
		printf("ub: MIN(%f, %f) = %f\n", auxUB, solViav->funObj_, ub);

		gap = fabs(ub - lb);
		printf("GAP PERCENTUAL: %f%%\n", fabs((lb - ub) / ub) * 100);
		printf("GAP ESCALAR (ub - lb): %f - %f = %.2f\n", ub, lb, gap);
		if (gap < 1.0) {
			break;
		}

		// Calcular os sub-gradientes
		printf("Calculando os sub-gradientes\n");
		double* subGradsRes10 = getSubGradRest10(solRel, instRel, rest);
		double* subGradsRes14 = getSubGradRest14(solRel, instRel, rest);
		double* subGradsRes15 = getSubGradRest15(solRel, instRel, rest);


		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}

		printf("calculando o passo\n");
		// Calcular o passo
		passo = calculaPasso(eta, lb, ub, subGradsRes10, subGradsRes14, subGradsRes15, instOrig);

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
		printf("it = %d\n", it);
		printf("itSemMelhora = %d\n", itSemMelhora);
		printf("\n");
		printf("----------------------------------------------------\n");

		desalocaIntancia(instRel);
		desalocaSolucao(solRel);
		desalocaSolucao(solViav);
		free(subGradsRes10);
		free(subGradsRes14);
		free(subGradsRes15);

		it++;
	} while (eta > 0.005);
	//} while (eta > 0.005 && it < 3);

	h = clock() - h;
	printf("\n=============================\n");
	printf("TEMPO DE EXECUCAO: %f\n", (double) h / CLOCKS_PER_SEC);

	return bestSol;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void relaxarModelo(char *arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest) {

	printf("Calculando coeficientes\n");
	montaVetCoefsFO(inst, vetMultRes10, vetMultRes14, vetMultRes15, rest);
	printf("Escrevendo LP\n");
	montarModeloRelaxado(arq, inst, vetMultRes10, vetMultRes14, vetMultRes15);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest10(Solucao* sol, Instancia* inst, RestricoesRelaxadas* rest) {

	int numRest10 = inst->numTur__*inst->numDia__*inst->numPerDia__;
	double* vetSubGrad = (double*)malloc(numRest10 * sizeof(double));

	double soma;
	double x1, x2, x3, coefX1, coefX2, coefX3;
	int posX1, posX2, posX3, posZ;
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
						posX1 = offset3D(r, prim, c, inst->numPerTot__, inst->numDis__);
						posX2 = offset3D(r, seg, c, inst->numPerTot__, inst->numDis__);
						x1 = sol->vetSol_[posX1];
						x2 = sol->vetSol_[posX2];
						coefX1 = rest->vetRestJanHor__[pos].coefMatX[posX1];
						coefX2 = rest->vetRestJanHor__[pos].coefMatX[posX2];
						soma += (coefX1 * x1) + (coefX2 * x2);
					}
				}
			posZ = offset3D(u, d, 0, inst->numDia__, inst->numPerDia__);
			double z = sol->vetSolZ_[posZ];
			double coefZ = rest->vetRestJanHor__[pos].coefMatZ[posZ];
			vetSubGrad[pos] = soma + (coefZ * z);
			//printf("R10 pos %d: %f + (%.4f * %.4f) = %.4f\n", pos, soma, coefZ, z, vetSubGrad[pos]);

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
						posX1 = offset3D(r, prim, c, inst->numPerTot__, inst->numDis__);
						posX2 = offset3D(r, seg, c, inst->numPerTot__, inst->numDis__);
						x1 = sol->vetSol_[posX1];
						x2 = sol->vetSol_[posX2];
						coefX1 = rest->vetRestJanHor__[pos].coefMatX[posX1];
						coefX2 = rest->vetRestJanHor__[pos].coefMatX[posX2];
						soma += (coefX1 * x1) + (coefX2 * x2);
					}
				}
			posZ = offset3D(u, d, 1, inst->numDia__, inst->numPerDia__);
			double z = sol->vetSolZ_[posZ];
			double coefZ = rest->vetRestJanHor__[pos].coefMatZ[posZ];
			vetSubGrad[pos] = soma + (coefZ * z);
			//printf("R10 pos %d: %f + (%.4f * %.4f) = %.4f\n", pos, soma, coefZ, z, vetSubGrad[pos]);
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
							posX1 = offset3D(r, prim, c, inst->numPerTot__, inst->numDis__);
							posX2 = offset3D(r, seg, c, inst->numPerTot__, inst->numDis__);
							posX3 = offset3D(r, ter, c, inst->numPerTot__, inst->numDis__);
							x1 = sol->vetSol_[posX1];
							x2 = sol->vetSol_[posX2];
							x3 = sol->vetSol_[posX3];
							coefX1 = rest->vetRestJanHor__[pos].coefMatX[posX1];
							coefX2 = rest->vetRestJanHor__[pos].coefMatX[posX2];
							coefX3 = rest->vetRestJanHor__[pos].coefMatX[posX3];
							soma += (coefX1 * x1) + (coefX2 * x2) + (coefX3 * x3);
						}
					}
				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				double z = sol->vetSolZ_[posZ];
				double coefZ = rest->vetRestJanHor__[pos].coefMatZ[posZ];
				vetSubGrad[pos] = soma + (coefZ * z);
				//printf("R10 pos %d: %f + (%.4f * %.4f) = %.4f\n", pos, soma, coefZ, z, vetSubGrad[pos]);
				pos++;
			}
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest14(Solucao* sol, Instancia* inst, RestricoesRelaxadas* rest) {

	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	double* vetSubGrad = (double*)malloc(numRest14 * sizeof(double));

	int pos = 0;
	int posX, posY;
	double coefX, coefY;
	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {
				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				posY = offset2D(c, r, inst->numSal__);
				coefX = rest->vetRest14__[pos].coefMatX[posX];
				coefY = rest->vetRest14__[pos].coefMatY[posY];
				double x = sol->vetSol_[posX];
				double y = sol->vetSolY_[posY];
				vetSubGrad[pos] = (coefX * x) + (coefY * y);
				//printf("R14 pos %d: (%.4f * %.4f) + (%.4f * %.4f) = %.4f\n", pos, coefX, x, coefY, y, vetSubGrad[pos]);
				pos++;
			}
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getSubGradRest15(Solucao* sol, Instancia* inst, RestricoesRelaxadas* rest) {

	int numRest15 = inst->numSal__ * inst->numDis__;
	double* vetSubGrad = (double*)malloc(numRest15 * sizeof(double));

	int pos = 0;
	int posX, posY;
	double coefX, coefY;
	for (int r = 0; r < inst->numSal__; r++) {
		for (int c = 0; c < inst->numDis__; c++) {

			posY = offset2D(c, r, inst->numSal__);
			double soma = 0;
			for (int p = 0; p < inst->numPerTot__; p++) {
				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				coefX = rest->vetRest15__[pos].coefMatX[posX];
				soma += coefX * sol->vetSol_[posX];
			}

			double y = sol->vetSolY_[posY];
			coefY = rest->vetRest15__[pos].coefMatY[posY];
			vetSubGrad[pos] = soma + (coefY * y);
			//printf("R15 pos %d: %.4f + (%.4f * %.4f) = %.4f\n", pos, soma, coefY, y, vetSubGrad[pos]);

			pos++;
		}
	}

	return vetSubGrad;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double calculaPasso(double eta, double lb, double ub, double* subGradsRes10, double* subGradsRes14, double* subGradsRes15, Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;

	double modulo = 0;
	for (int i = 0; i < numRest10; i++) {
		modulo += subGradsRes10[i] * subGradsRes10[i];
	}

	for (int i = 0; i < numRest14; i++) {
		modulo += subGradsRes14[i] * subGradsRes14[i];
	}

	for (int i = 0; i < numRest15; i++) {
		modulo += subGradsRes15[i] * subGradsRes15[i];
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
		/*printf("MIN(0, %.3f + (%.3f * %.3f))\n", val, passo, subGrad[i]);
		if (vetMult[i] > val) {
			printf("MAIOR\n");
			printf("MENOR\n");
		}*/
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atualizaMultMaiIg(double* vetMult, double passo, double* subGrad, int tamVet) {

	double val;
	for (int i = 0; i < tamVet; i++) {
		val = vetMult[i];
		vetMult[i] = MAX(0, val - (passo * subGrad[i]));
		/*printf("MAX(0, %.3f + (%.3f * %.3f))\n", val, passo, subGrad[i]);
		if (vetMult[i] < val) {
			printf("MENOR\n");
			printf("MAIOR\n");
		}*/
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

double* juntaVetsSubGrad(double* vetSubGrad10, double* vetSubGrad14, double* vetSubGrad15, int tamSubGrad10, int tamSubGrad14, int tamSubGrad15) {

	int pos = 0;
	double* vetFinal = (double*)malloc((tamSubGrad10 + tamSubGrad14 + tamSubGrad15) * sizeof(double));

	// Restri��o 10
	for (int i = 0; i < tamSubGrad10; i++) {
		vetFinal[pos] = vetSubGrad10[i];
		pos++;
	}

	// Restri��o 14
	for (int i = 0; i < tamSubGrad14; i++) {
		vetFinal[pos] = vetSubGrad14[i];
		pos++;
	}

	// Restri��o 15
	for (int i = 0; i < tamSubGrad15; i++) {
		vetFinal[pos] = vetSubGrad15[i];
		pos++;
	}

	return vetFinal;
}

void debugaCoeficientes(char* arq, Instancia* instRel, Solucao* sol, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest) {
	int numX = instRel->numPerTot__ * instRel->numSal__ * instRel->numDis__;
	int numZ = instRel->numTur__ * instRel->numDia__ * instRel->numPerDia__;
	int numY = instRel->numSal__ * instRel->numDis__;
	int numVar = numX + numY + numZ;

	double** matD = montaMatD(instRel, rest);
	double* vetD = montaVetD(instRel, sol, rest);
	printf("\n");
	printVetD(instRel, vetD);
	escreveCSVDebugCoefs(arq, instRel, sol, matD, vetD, vetMultRes10, vetMultRes14, vetMultRes15);

	free(vetD);
	desalocaMatD(matD, numVar);
}

void printMultiplicadores(double* vet, int tam) {

	for (int i = 0; i < tam; i++) {
		printf("%.3f, ", vet[i]);
	}
}