#include "RelLagran.h"
#include "Colecoes.h"

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

//------------------------------------------------------------------------------
/*
paradaPorTempo {
	false: enquanto eta > 0.005
	true:  roda por 1 hora;
}
*/
Solucao* execRelLagran(char* arq, Instancia* instOrig, double* vetMult, MatRestCplex* rest, int paradaPorTempo, int ubFixo, double ub) {

	Solucao *solRel, *solViav, *bestSol;
	Instancia* instRel;
	solViav = bestSol = NULL;

	double firstLB, firstUB, tempo;
	double lb = -INFINITY;
	ub = ubFixo ? ub : INFINITY;
	double eta = 2.0;

	double gap, passo;
	int numRest10 = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numRest14 = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numRest15 = instOrig->numSal__ * instOrig->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;
	int numX = instOrig->numPerTot__ * instOrig->numSal__ * instOrig->numDis__;
	int numZ = instOrig->numTur__ * instOrig->numDia__ * instOrig->numPerDia__;
	int numY = instOrig->numSal__ * instOrig->numDis__;
	int numVar = numX + numY + numZ;
	int continua = 1;
	int itSemMelhora = 0;
	int it = 0;
	
	printf("\n");

	clock_t h, tCplex;
	h = clock();

	do {

		printf("Clonando instância\n");
		instRel = clonarInstancia(instOrig);

		// Resolver o problema relaxado
		printf("Relaxando o modelo\n");
		relaxarModelo(arq, instRel, vetMult, rest);
		printf("Executando CPX\n");
		tCplex = clock();
		solRel = (Solucao*)execCpx(arq, instRel);
		tCplex = clock() - tCplex;
		printf("Tempo de execucao CPLEX: %f\n", (double)(tCplex / CLOCKS_PER_SEC));

		if (!ubFixo) {
			// Viabilizar a solução
			printf("Clonando solucao\n");
			solViav = clonarSolucao(solRel, instRel);
			printf("Viabilizando solucao\n");
			viabilizaSol(solViav, instOrig);
			calculaFO(solViav, instOrig);
		}

		if (solRel->funObj_ > lb) {
			itSemMelhora = 0;
			bestSol = clonarSolucao(solRel, instOrig);
		}
		else {
			itSemMelhora++;
		}

		// Calcular os limitantes
		double auxLB = lb;
		lb = MAX(auxLB, solRel->funObj_);
		
		if (!ubFixo) {
			double auxUB = ub;
			ub = MIN(auxUB, solViav->funObj_);
		}

		printf("LB = %f\n", lb);
		printf("UB = %f\n", ub);

		if (it == 0) {
			firstLB = lb;
			firstUB = ub;
		}

		gap = ub != 0 ? (fabs((lb - ub) / ub) * 100) : INFINITY;
		printf("GAP: %f%%\n", fabs((lb - ub) / ub) * 100);

		if (ub - lb < 1) {
			break;
		}

		// Calcular os sub-gradientes
		printf("Calculando os sub-gradientes\n");
		double* subGrads = getSubGrads(solRel, instRel, rest);

		if (itSemMelhora != 0 && itSemMelhora % 30 == 0) {
			eta /= 2;
		}

		printf("calculando o passo\n");
		// Calcular o passo
		passo = calculaPasso(eta, lb, ub, subGrads, instOrig);

		// Atualizar os multiplicadores
		printf("atualizando multiplicadores\n");
		atualizaMultiplicadores(instOrig, vetMult, passo, subGrads);

		printf("FO solRel = %f\n", solRel->funObj_);
		if (!ubFixo) {
			printf("FO solViav = %f\n", solViav->funObj_);
		}
		printf("gap = %f\n", gap);
		printf("eta = %f\n", eta);
		printf("passo = %f\n", passo);
		printf("it = %d\n", it);
		printf("itSemMelhora = %d\n", itSemMelhora);
		printf("\n");
		printf("\n============================ FIM =====================================\n");

		desalocaIntancia(instRel);
		desalocaSolucao(solRel);
		free(subGrads);

		if (solViav != NULL) {
			desalocaSolucao(solViav);
		}

		it++;

		if (paradaPorTempo) {
			h = clock() - h;
			tempo = (double) (h / CLOCKS_PER_SEC);

			continua = tempo < 3600 ? 1 : 0;
		} else {
			continua = eta > 0.005 ? 1 : 0;
		}

		continua = continua && passo != 0;

	} while (continua);

	h = clock() - h;
	tempo = (double)(h / CLOCKS_PER_SEC);
	printf("\n=============================\n");
	printf("TEMPO DE EXECUCAO: %f\n", (double) h / CLOCKS_PER_SEC);
	
	montaResultado(bestSol, gap, firstLB, lb, firstUB, ub, tempo);

	return bestSol;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void relaxarModelo(char *arq, Instancia* inst, double* vetMult, MatRestCplex* rest) {

	clock_t tempo;
	printf("Calculando coeficientes\n");
	tempo = clock();
	montaVetCoefsFO(inst, vetMult, rest);
	tempo = clock() - tempo;
	printf("Tempo do calculo dos coeficientes: %f\n", (double)(tempo / CLOCKS_PER_SEC));
	printf("Escrevendo LP\n");
	montarModeloRelaxado(arq, inst);
}
//------------------------------------------------------------------------------

double* getSubGrads(Solucao* sol, Instancia* inst, MatRestCplex* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ * inst->numDis__;
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;
	int ini, fim, pos, lin;
	double soma = 0;


	double* vetSubGrads = (double*)malloc((numRest10 + numRest14 + numRest15) * sizeof(double));

	for (int i = 0; i < numRest; i++) {
		vetSubGrads[i] = 0;
	}

	for (int i = 0; i < rest->numLin; i++) {
		ini = rest->matbeg[i];
		fim = ini + rest->matcnt[i];
		soma = 0;

		for (int j = ini; j < fim; j++) {
			//printf("%d\n", j);
			// restrição 10
			if (i < numRest10) {
				// coeficiente de Z
				if (j == fim - 1) {
					pos = rest->matind[j] - numX;
					soma += rest->matval[j] * sol->vetSolZ_[pos];
				} else { // coeficiente de X
					soma += rest->matval[j] * sol->vetSol_[rest->matind[j]];
				}	
			} else { // Restrições 14 e 15
				// coeficiente de Y
				if (j == fim - 1) {
					pos = rest->matind[j] - numX - numZ;
					soma += rest->matval[j] * sol->vetSolY_[pos];
				} else { // coeficiente de X
					soma += rest->matval[j] * sol->vetSol_[rest->matind[j]];
				}
			}
		}
		vetSubGrads[i] = soma;
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
		printf("%.3f; ", vet[i]);
	}
}

void printSubgrads(double* vetSubgrads, Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;

	for (int i = 0; i < numRest; i++) {
		printf("%.3f; ", vetSubgrads[i]);
	}
	printf("\n");
}