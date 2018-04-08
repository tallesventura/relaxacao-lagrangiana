#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

//------------------------------------------------------------------------------
Solucao* execRelLagran(char* arq, Instancia* instOrig, double* vetMult, MatRestCplex* rest) {

	Solucao *solRel, *solViav, *bestSol;
	Instancia* instRel;
	double lb = -INFINITY;
	double ub = INFINITY;
	double previousFO = -INFINITY;
	double eta = 2.0;

	double gap, passo;
	int numRest10 = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numRest14 = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numRest15 = instOrig->numSal__ * instOrig->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;
	int numX = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numZ = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numY = instOrig->numSal__ * instOrig->numDis__;
	printf("%d, %d, %d", numRest10, numRest14, numRest15);

	int itSemMelhora = 0;
	int it = 0;
	
	printf("\n");

	clock_t h;
	h = clock();

	do {

		printf("Clonando instância\n");
		instRel = clonarInstancia(instOrig);

		// Resolver o problema relaxado
		//printf("Relaxando o modelo\n");
		relaxarModelo(arq, instRel, vetMult, rest);
		//imprimeMatRestCplex(rest, instRel);
		//printf("------------------------------COEFS FO-------------------------------------\n");
		//printCoefsFO(instRel);
		//printf("\n");
		printf("Executando CPX\n");
		solRel = (Solucao*)execCpx(arq, instRel);
		//printf("\n");
		/*imprimeX(solRel, instOrig);
		printf("\n");
		imprimeZ(solRel, instOrig);
		printf("\n");
		imprimeY(solRel, instOrig);*/
		/*printf("\n----------------------MULTIPLICADORES-----------------------------------------------\n");
		printMultiplicadores(vetMult, numRes);
		printf("\n---------------------------------------------------------------------\n");*/

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

		// Viabilizar a solução
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
		if (gap == 0) {
			break;
		}

		// Calcular os sub-gradientes
		printf("Calculando os sub-gradientes\n");
		double* subGrads = getSubGrads(solRel, instRel, rest);

		/*printf("----------------------SUBGRADS-----------------------------------------------\n");
		printSubgrads(subGrads, instOrig);
		printf("\n");*/

		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}

		/*for (int i = 0; i < numX; i++) {
			printf("%f;", solRel->vetSol_[i]);
		}
		for (int i = 0; i < numZ; i++) {
			printf("%f;", solRel->vetSolZ_[i]);
		}
		for (int i = 0; i < numY; i++) {
			printf("%f;", solRel->vetSolY_[i]);
		}
		printf("\n");*/

		printf("calculando o passo\n");
		// Calcular o passo
		passo = calculaPasso(eta, lb, ub, subGrads, instOrig);

		if (passo == 0) {
			break;
		}

		printf("atualizando multiplicadores\n");
		// Atualizar os multiplicadores
		atualizaMultiplicadores(instOrig, vetMult, passo, subGrads);
		//printMultiplicadores(vetMult, numRes);

		previousFO = solRel->funObj_;

		printf("FO solRel = %f\n", solRel->funObj_);
		printf("FO solViav = %f\n", solViav->funObj_);
		printf("gap = %f\n", gap);
		printf("eta = %f\n", eta);
		printf("passo = %f\n", passo);
		printf("it = %d\n", it);
		printf("itSemMelhora = %d\n", itSemMelhora);
		printf("\n");
		printf("\n============================ FIM =====================================\n");

		desalocaIntancia(instRel);
		desalocaSolucao(solRel);
		desalocaSolucao(solViav);
		free(subGrads);

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
void relaxarModelo(char *arq, Instancia* inst, double* vetMult, MatRestCplex* rest) {

	printf("Calculando coeficientes\n");
	montaVetCoefsFO(inst, vetMult, rest);
	printf("Escrevendo LP\n");
	montarModeloRelaxado(arq, inst);
}
//------------------------------------------------------------------------------

double* getSubGrads(Solucao* sol, Instancia* inst, MatRestCplex* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ + inst->numDis__;
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;
	int col, posZ, posY;
	double soma = 0;

	double* vetSubGrads = (double*)malloc((numRest10 + numRest14 + numRest15) * sizeof(double));

	for (int i = 0; i < numRest; i++) {
		vetSubGrads[i] = 0;
	}

	for (int lin = 0; lin < numRest; lin++) {
		soma = 0;
		for (int i = 0; i < rest->numCoefsTotal; i++) {
			if (rest->matind[i] == lin) {
				col = findCol(i, rest);
				if (col < numX) {
					//printf("%f + (%d * %f)\n", vetSubGrads[lin], rest->matval[i], sol->vetSol_[col]);
					//printf("%f + ", rest->matval[i] * sol->vetSol_[col]);
					soma += rest->matval[i] * sol->vetSol_[col];
					//printf("(%d * %f) + ", rest->matval[i], sol->vetSol_[col]);
				}
				else if (col < numX + numZ) {
					posZ = col - numX;
					//printf("%f + (%d * %f)\n", vetSubGrads[lin], rest->matval[i], sol->vetSolZ_[posZ]);
					//printf("%f + ", rest->matval[i] * sol->vetSolZ_[posZ]);
					soma += rest->matval[i] * sol->vetSolZ_[posZ];
					//printf("(%d * %f) + ", rest->matval[i], sol->vetSolZ_[posZ]);
				}
				else {
					posY = col - numX - numZ;
					//printf("%f + (%d * %f)\n", vetSubGrads[lin], rest->matval[i], sol->vetSolY_[posY]);
					//printf("%f + ", rest->matval[i] * sol->vetSolY_[posY]);
					soma += rest->matval[i] * sol->vetSolY_[posY];
					//printf("(%d * %f) + ", rest->matval[i], sol->vetSolY_[posY]);
				}
			}
		}
		vetSubGrads[lin] = soma;
		//printf("= %f\n", soma);
		//printf("vetSubGrads[%d] = %f\n", lin, soma);
	}

	return vetSubGrads;
}

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
double calculaPasso(double eta, double lb, double ub, double* subGrads, Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;

	double modulo = 0;
	for (int i = 0; i < numRest; i++) {
		modulo += subGrads[i] * subGrads[i];
		//printf("(%f * %f) + ", subGrads[i], subGrads[i]);
	}
	//printf("= %f\n", modulo);

	//printf("(eta * (ub - lb)) / modulo => (%f * (%f - %f)) / %f\n", eta, ub, lb, modulo);
	printf("MODULO: %f\n", modulo);

	if (isnan(modulo) || modulo == 0) {
		return 0;
	}

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

void atualizaMultiplicadores(Instancia* inst, double* vetMult, double passo, double* subGrads) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;

	double val;

	for (int i = 0; i < numRest10; i++) {
		val = vetMult[i];
		vetMult[i] = MIN(0, val - (passo * subGrads[i]));
	}

	for (int i = numRest10; i < numRest10 + numRest14; i++) {
		val = vetMult[i];
		vetMult[i] = MIN(0, val - (passo * subGrads[i]));
	}

	for (int i = numRest10 + numRest14; i < numRest; i++) {
		val = vetMult[i];
		vetMult[i] = MAX(0, val - (passo * subGrads[i]));
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

void printSubgrads(double* vetSubgrads, Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;

	for (int i = 0; i < numRest; i++) {
		printf("%.3f, ", vetSubgrads[i]);
	}
	printf("\n");
}