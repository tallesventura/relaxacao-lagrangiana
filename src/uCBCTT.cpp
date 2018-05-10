#include "uCBCTT.h"
#include "Colecoes.h"
#include "RelLagran.h"
#include "Solucao.h"
#include "ValoresLimites.h"
//#include "..\lib\cplex\include\cplex.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>


#define RES_SOFT
#define RES_CAP_SAL // Restrição soft de capacidade das salas
#define	RES_JAN_HOR // Restrição soft de janelas de horários
#define RES_DIA_MIN // Restrição soft de dias mínimos
#define RES_SAL_DIF // Restrição soft de salas diferentes

#define RELAXAR
//#define ESCREVE_CSV

char INST[50] = "comp";
//char INST[50] = "toy";

char* NOME_INSTANCIAS[] = { "comp01", "comp02", "comp03", "comp04", "comp05", "comp06", "comp07", "comp08", "comp09", "comp10",
"comp11", "comp12", "comp13", "comp14", "comp15", "comp16", "comp17", "comp18", "comp19", "comp20",
"comp21" };

int UBs[] = {5, 24, 52, 35, 211, 27, 6, 37, 96, 4, 0, 147, 59, 51, 52, 18, 56, 61, 57, 4, 74};

//==============================================================================

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	srand(time(NULL));
	char nomeInst[10];
	strcpy_s(nomeInst, INST);
	strcat_s(nomeInst, "02");

	//execUma(nomeInst);
	execTodas();

	printf("\n\n>>> Pressione ENTER para encerrar: ");
	_getch();
	return 0;
}
//------------------------------------------------------------------------------

void execUma(char* nomeInst, char* pathResult, int paradaPorTempo, int ubFixo, double ub) {
	char aux[150];
	Solucao* sol;

	printf("RESULTADO EM: %s\n", pathResult);

	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
	strcat_s(aux, ".ctt");
	printf("%s\n", aux);
	
	Instancia* inst = lerInstancia(aux);
	initCoefsFO(inst);

	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
	strcat_s(aux, ".lp");
#ifdef RELAXAR
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__*inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;
	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ * inst->numDis__;
	int numVar = numX + numZ + numY;
	printf("numX = %d; numZ = %d; numY = %d; total = %d\n", numX, numZ, numY, numX + numZ + numY);

	printf("Montando as matrizes de coeficientes do CPLEX\n");
	MatRestCplex* matRestCplex = montaMatRestCplex(inst, numVar, numRest);

	printf("Inicializando vetor de multiplicadores\n");
	double* vetMult = (double*)malloc(numRest * sizeof(double));
	initMultiplicadores(vetMult, numRest, VAL_INIT_ALPHA);
#else
	montarModeloPLI(aux, inst);
#endif
	printf("Executando Relaxacao Lagrangiana\n");
	sol = execRelLagran(aux, inst, vetMult, matRestCplex, 0, 0, 0);
	montaSolucao(sol, inst);
	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
#ifdef ESCREVE_CSV
	char csv[50];
	strcpy_s(csv, aux);
	strcat_s(csv, ".csv");
	exportarCsv(sol, csv, inst);
#endif
	strcat_s(aux, ".sol");
	printf("Escrevendo Resultados\n");
	escreverResultadosCSV(pathResult, sol);
	escreverSol(sol, aux, inst);
	printf("Escrevendo Solucao\n");
	escreverSol(sol, aux, inst);
	desalocaMatRestCplex(matRestCplex);
	desalocaIntancia(inst);
	desalocaSolucao(sol);
	free(vetMult);
}

void execTodas() {

	// ETA e viabilização
	// TODO: rodar para todas instancias
	for (int i = 0; i < 1; i++) {
		char nomeArq[20];
		char pathArq[150];

		strcpy_s(nomeArq, NOME_INSTANCIAS[i]);
		strcat_s(nomeArq, "_ETA_viab");
		strcpy_s(pathArq, PATH_RESULTADOS);
		strcat_s(pathArq, nomeArq);
		strcat_s(pathArq, ".csv");

		execUma(NOME_INSTANCIAS[i], pathArq, 0, 0 , UBs[i]);
	}

	// ETA e UB fixo
	// TODO: rodar para todas instancias
	for (int i = 0; i < 1; i++) {
		char nomeArq[20];
		char pathArq[150];

		strcpy_s(nomeArq, NOME_INSTANCIAS[i]);
		strcat_s(nomeArq, "_ETA_UB_fixo");
		strcpy_s(pathArq, PATH_RESULTADOS);
		strcat_s(pathArq, nomeArq);
		strcat_s(pathArq, ".csv");

		execUma(NOME_INSTANCIAS[i], pathArq, 0, 1, UBs[i]);
	}

	// Tempo e viabilização
	/*for (int i = 0; i < NUM_INST; i++) {
		char nomeArq[20];
		char pathArq[150];

		strcpy_s(nomeArq, NOME_INSTANCIAS[i]);
		strcat_s(nomeArq, "_tempo_viab");
		strcpy_s(pathArq, PATH_RESULTADOS);
		strcat_s(pathArq, nomeArq);
		strcat_s(pathArq, ".csv");

		execUma(NOME_INSTANCIAS[i], pathArq, 1, 0, UBs[i]);
	}*/

	// Tempo e UB fixo
	/*for (int i = 0; i < NUM_INST; i++) {
		char nomeArq[20];
		char pathArq[150];

		strcpy_s(nomeArq, NOME_INSTANCIAS[i]);
		strcat_s(nomeArq, "_tempo_UB_fixo");
		strcpy_s(pathArq, PATH_RESULTADOS);
		strcat_s(pathArq, nomeArq);
		strcat_s(pathArq, ".csv");
		execUma(NOME_INSTANCIAS[i], pathArq, 1, 1, UBs[i]);
	}*/
}


//------------------------------------------------------------------------------
void getValSol(Solucao *sol, CPXENVptr env, CPXLPptr lp, Instancia* inst) {

	int sts, iniX, fimX, iniZ, fimZ, iniQ, fimQ, iniY, fimY, iniV, fimV;

	iniX = 0;
	fimX = inst->numPerTot__*inst->numSal__*inst->numDis__ - 1;
	iniZ = fimX + 1;
	fimZ = iniZ + (inst->numTur__*inst->numDia__*inst->numPerDia__) - 1;
	iniQ = fimZ + 1;
	fimQ = iniQ + inst->numDis__ - 1;
	iniY = fimQ + 1;
	fimY = iniY + (inst->numSal__*inst->numDis__) - 1;
	iniV = fimY + 2;
	fimV = iniV + (inst->numDia__*inst->numDis__) - 1;

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;
	int numV = inst->numDia__ * inst->numDis__;

	sol->vetSol_ = (double*)malloc(numX * sizeof(double));
	sol->vetSolZ_ = (double*)malloc(numZ * sizeof(double));
	sol->vetSolQ_ = (double*)malloc(numQ * sizeof(double));
	sol->vetSolY_ = (double*)malloc(numY * sizeof(double));
	sol->vetSolV_ = (double*)malloc(numV * sizeof(double));

	// Variáveis x
	sts = CPXgetmipx(env, lp, sol->vetSol_, iniX, fimX);
	// Variáveis z
	sts = CPXgetmipx(env, lp, sol->vetSolZ_, iniZ, fimZ);
	// Variáveis q
	sts = CPXgetmipx(env, lp, sol->vetSolQ_, iniQ, fimQ);
	// Variáveis y
	sts = CPXgetmipx(env, lp, sol->vetSolY_, iniY, fimY);
	// Variáveis v
	sts = CPXgetmipx(env, lp, sol->vetSolV_, iniV, fimV);

	/*for (int i = 0; i < numX; i++) {
		printf("%.6f, ", sol->vetSol_[i]);
	}*/
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Solucao* execCpx(char *arq, Instancia* inst)
{
	int sts;
	Solucao* s = (Solucao*) malloc(sizeof(Solucao));
	clock_t h;
	CPXENVptr env;
	CPXLPptr lp;
	env = CPXopenCPLEX(&sts);
	sts = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);

	// 1 quando tem combinação de valores grandes e pequenos para os coeficientes na matriz de restrições
	//sts = CPXsetintparam(env, CPXPARAM_Read_Scale, 1);
	//sts = CPXsetdblparam(env, CPX_PARAM_EPMRK, 0.90);
	lp = CPXcreateprob(env, &sts, "");
	sts = CPXreadcopyprob(env, lp, arq, NULL);
	s->numVar_ = CPXgetnumcols(env, lp);
	s->numRes_ = CPXgetnumrows(env, lp);
	h = clock();
	sts = CPXmipopt(env, lp);
	h = clock() - h;
	s->tempo_ = (double)h / CLOCKS_PER_SEC;
	sts = CPXgetmipobjval(env, lp, &(s->valSol_));
	sts = CPXgetbestobjval(env, lp, &(s->bstNod_));

	// Pegando os valores das variáveis
	getValSol(s, env, lp, inst);

	// Inicializando as matrizes de solucao
	montaSolucao(s, inst);

	sts = CPXfreeprob(env, &lp);
	sts = CPXcloseCPLEX(&env);

	return s;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloPLI(char *arq, Instancia* inst)
{
	int val;
	FILE *f = fopen(arq, "w");
	// ------------------ FO
	fprintf(f, "MIN\n");
#ifndef RES_SOFT
	fprintf(f, "\n\\Auxiliar\n");
	for (int r = 0; r < numSal__; r++)
	{
		for (int p = 0; p < numPerTot__; p++)
		{
			for (int c = 0; c < numDis__; c++)
				fprintf(f, "+ 0 x_%d_%d_%d ", p, r, c);
			fprintf(f, "\n");
		}
	}
	fprintf(f, "\nST\n");
#else
	fprintf(f, "\n\\Capacidade das salas\n");
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int p = 0; p < inst->numPerTot__; p++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->vetDisciplinas__[c].numAlu_ > inst->vetSalas__[r].capacidade_)
					fprintf(f, "+ %d x_%d_%d_%d ", PESOS[0] * (inst->vetDisciplinas__[c].numAlu_ - inst->vetSalas__[r].capacidade_), p, r, c);
				else
					fprintf(f, "+ 0 x_%d_%d_%d ", p, r, c);
			fprintf(f, "\n");
		}
	}

	fprintf(f, "\n\n\\Janelas de horários\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int s = 0; s < inst->numPerDia__; s++)
				fprintf(f, "+ %d z_%d_%d_%d ", PESOS[1], u, d, s);
			fprintf(f, "\n");
		}
	}

	fprintf(f, "\n\\Dias mínimos\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "+ %d q_%d ", PESOS[2], c);

	fprintf(f, "\n\n\\Salas diferentes\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			fprintf(f, "+ %d y_%d_%d ", PESOS[3], r, c);
		fprintf(f, "\n");
	}

	val = PESOS[3] * inst->numDis__;
	fprintf(f, "- val\n");
	fprintf(f, "\nST\n");
	fprintf(f, "\n\\Valor constante\n");
	fprintf(f, "val = %d\n", val);
#endif
	fprintf(f, "\n\\ ------------------------------------ HARD------------------------------------\n");
	fprintf(f, "\n\\R1 - Número de aulas\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int r = 0; r < inst->numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
		fprintf(f, "= %d\n", inst->vetDisciplinas__[c].numPer_);
	}
	fprintf(f, "\n\\R2 - Aulas na mesma sala no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int r = 0; r < inst->numSal__; r++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R3 - Aulas de uma disciplina no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int c = 0; c < inst->numDis__; c++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R4 - Aulas de um professor no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int t = 0; t < inst->numPro__; t++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->vetDisciplinas__[c].professor_ == t)
						fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R5 - Aulas de uma turma no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int c = 0; c < inst->numDis__; c++)
				{
					for (int k = 0; k < inst->vetTurmas__[u].numDis_; k++)
						if (inst->vetTurmas__[u].vetDis_[k] == c)
						{
							fprintf(f, "+ x_%d_%d_%d ", p, r, c);
							break;
						}
				}
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R6 - Restrições de oferta (alocação)\n");
	for (int s = 0; s < inst->numRes__; s++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			fprintf(f, "+ x_%d_%d_%d ", inst->vetRestricoes__[s].periodo_, r, inst->vetRestricoes__[s].disciplina_);
		fprintf(f, "= 0\n");
	}
#ifdef RES_SOFT
	fprintf(f, "\n\\ ------------------------------------ SOFT------------------------------------\n");
	fprintf(f, "\n\\R7 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
			for (int p = (d*inst->numPerDia__); p < (d*inst->numPerDia__) + inst->numPerDia__; p++)
			{
				for (int r = 0; r < inst->numSal__; r++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
				fprintf(f, "- v_%d_%d <= 0\n", d, c);
			}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R8 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int p = (d*inst->numPerDia__); p < (d*inst->numPerDia__) + inst->numPerDia__; p++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- v_%d_%d >= 0\n", d, c);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R9 - Dias mínimos\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
			fprintf(f, "+ v_%d_%d ", d, c);
		fprintf(f, "+ q_%d >= %d\n", c, inst->vetDisciplinas__[c].diaMin_);
	}

#ifndef REL_JAN_HOR
	fprintf(f, "\n\\R10 a R13+#PER_DIA - Janelas no horário das turmas\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__), r, c, (d*inst->numPerDia__) + 1, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 0);
		}
	}
	fprintf(f, "\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__) + inst->numPerDia__ - 1, r, c, (d*inst->numPerDia__) + inst->numPerDia__ - 2, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 1);
		}
	}
	fprintf(f, "\n");
	for (int s = 2; s < inst->numPerDia__; s++)
	{
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++)
							fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__) + s - 1, r, c, (d*inst->numPerDia__) + s - 2, r, c, (d*inst->numPerDia__) + s, r, c);
					}
				fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, s);
			}
		}
		fprintf(f, "\n");
	}
#endif // !REL_JAN_HOR

#ifndef REL_SAL_DIF
	fprintf(f, "\n\\R14 - Salas utilizadas por disciplina\n");
	for (int p = 0; p < inst->numPerTot__; p++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "x_%d_%d_%d - y_%d_%d <= 0\n", p, r, c, r, c);
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R15 - Salas utilizadas por disciplina\n");
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int c = 0; c < inst->numDis__; c++)
		{
			for (int p = 0; p < inst->numPerTot__; p++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- y_%d_%d >= 0\n", r, c);
		}
		fprintf(f, "\n");
	}
#endif // !REL_SAL_DIF

#endif
	fprintf(f, "\nBOUNDS\n");
	fprintf(f, "\n\\Variáveis x\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "0 <= x_%d_%d_%d <= 1\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < inst->numDia__; d++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "0 <= v_%d_%d <= 1\n", d, c);
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "0 <= q_%d <= %d\n", c, inst->numDia__);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < inst->numTur__; u++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int s = 0; s < inst->numPerDia__; s++)
				fprintf(f, "0 <= z_%d_%d_%d <= 1\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "0 <= y_%d_%d <= 1\n", r, c);
#endif
#ifdef RES_SOFT
	fprintf(f, "\nGENERALS\n"); // Para variáveis inteiras
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "q_%d\n", c);
#endif
	fprintf(f, "\nBINARIES\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "x_%d_%d_%d\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < inst->numDia__; d++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "v_%d_%d\n", d, c);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < inst->numTur__; u++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int s = 0; s < inst->numPerDia__; s++)
				fprintf(f, "z_%d_%d_%d\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "y_%d_%d\n", r, c);
#endif
	fprintf(f, "\nEND");
	fclose(f);
}
//------------------------------------------------------------------------------

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

//------------------------------------------------------------------------------
void initMultiplicadoresAle(double* vetMult, int tam, double lb, double ub) {

	for (int i = 0; i < tam; i++) {
		vetMult[i] = fRand(lb, ub);
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initMultiplicadores(double* vetMult, int tam, double val) {

	for (int i = 0; i < tam; i++) {
		vetMult[i] = val;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloRelaxado(char *arq, Instancia* inst) {

	FILE *f = fopen(arq, "w");

	double coef;
	int numRestJanHor = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	// ------------------ FO
	fprintf(f, "MIN\n");

	fprintf(f, "\n\n\\Capacidade de Salas\n");
	int posX;
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int p = 0; p < inst->numPerTot__; p++)
		{
			for (int c = 0; c < inst->numDis__; c++) {
				
				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				coef = inst->vetCoefX[posX];
				
				if (coef < 0)
					fprintf(f, "%f x_%d_%d_%d ", coef, p, r, c);
				else
					fprintf(f, "+ %f x_%d_%d_%d ", coef, p, r, c);
			}
			fprintf(f, "\n");
		}
	}

	fprintf(f, "\n\n\\Janelas de horários\n");
	int posZ;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int s = 0; s < inst->numPerDia__; s++) {

				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				coef = inst->vetCoefZ[posZ];

				if (coef < 0)
					fprintf(f, "%f z_%d_%d_%d ", coef, u, d, s);
				else
					fprintf(f, "+ %f z_%d_%d_%d ", coef, u, d, s);
			}
			fprintf(f, "\n");
		}
	}

	fprintf(f, "\n\\Dias mínimos\n");
	for (int c = 0; c < inst->numDis__; c++) {

		coef = inst->vetCoefQ[c];
		fprintf(f, "+ %f q_%d ", coef, c);
	}
		

	fprintf(f, "\n\n\\Salas diferentes\n");
	int posY;
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int r = 0; r < inst->numSal__; r++) {

			posY = offset2D(c, r, inst->numSal__);
			coef = inst->vetCoefY[posY];

			if (coef < 0)
				fprintf(f, "%f y_%d_%d ", coef, r, c);
			else
				fprintf(f, "+ %f y_%d_%d ", coef, r, c);
		}
		fprintf(f, "\n");
	}

	int val = PESOS[3] * inst->numDis__;
	fprintf(f, "- val\n");
	fprintf(f, "\nST\n");
	fprintf(f, "\n\\Valor constante\n");
	fprintf(f, "val = %f\n", val);

	fprintf(f, "\n\\ ------------------------------------ HARD------------------------------------\n");
	fprintf(f, "\n\\R1 - Número de aulas\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int r = 0; r < inst->numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
		fprintf(f, "= %d\n", inst->vetDisciplinas__[c].numPer_);
	}
	fprintf(f, "\n\\R2 - Aulas na mesma sala no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int r = 0; r < inst->numSal__; r++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R3 - Aulas de uma disciplina no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int c = 0; c < inst->numDis__; c++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R4 - Aulas de um professor no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int t = 0; t < inst->numPro__; t++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->vetDisciplinas__[c].professor_ == t)
						fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R5 - Aulas de uma turma no mesmo período\n");
	for (int p = 0; p < inst->numPerTot__; p++)
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int c = 0; c < inst->numDis__; c++)
				{
					for (int k = 0; k < inst->vetTurmas__[u].numDis_; k++)
						if (inst->vetTurmas__[u].vetDis_[k] == c)
						{
							fprintf(f, "+ x_%d_%d_%d ", p, r, c);
							break;
						}
				}
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R6 - Restrições de oferta (alocação)\n");
	for (int s = 0; s < inst->numRes__; s++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			fprintf(f, "+ x_%d_%d_%d ", inst->vetRestricoes__[s].periodo_, r, inst->vetRestricoes__[s].disciplina_);
		fprintf(f, "= 0\n");
	}
#ifdef RES_SOFT
	fprintf(f, "\n\\ ------------------------------------ SOFT------------------------------------\n");
	fprintf(f, "\n\\R7 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
			for (int p = (d*inst->numPerDia__); p < (d*inst->numPerDia__) + inst->numPerDia__; p++)
			{
				for (int r = 0; r < inst->numSal__; r++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
				fprintf(f, "- v_%d_%d <= 0\n", d, c);
			}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R8 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int r = 0; r < inst->numSal__; r++)
				for (int p = (d*inst->numPerDia__); p < (d*inst->numPerDia__) + inst->numPerDia__; p++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- v_%d_%d >= 0\n", d, c);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R9 - Dias mínimos\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int d = 0; d < inst->numDia__; d++)
			fprintf(f, "+ v_%d_%d ", d, c);
		fprintf(f, "+ q_%d >= %d\n", c, inst->vetDisciplinas__[c].diaMin_);
	}

#ifndef REL_JAN_HOR
	fprintf(f, "\n\\R10 a R13+#PER_DIA - Janelas no horário das turmas\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__), r, c, (d*inst->numPerDia__) + 1, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 0);
		}
	}
	fprintf(f, "\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__) + inst->numPerDia__ - 1, r, c, (d*inst->numPerDia__) + inst->numPerDia__ - 2, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 1);
		}
	}
	fprintf(f, "\n");
	for (int s = 2; s < inst->numPerDia__; s++)
	{
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				for (int c = 0; c < inst->numDis__; c++)
					if (matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++)
							fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__) + s - 1, r, c, (d*inst->numPerDia__) + s - 2, r, c, (d*inst->numPerDia__) + s, r, c);
					}
				fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, s);
			}
		}
		fprintf(f, "\n");
	}
#endif // !REL_JAN_HOR

#ifndef REL_SAL_DIF
	fprintf(f, "\n\\R14 - Salas utilizadas por disciplina\n");
	for (int p = 0; p < inst->numPerTot__; p++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "x_%d_%d_%d - y_%d_%d <= 0\n", p, r, c, r, c);
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R15 - Salas utilizadas por disciplina\n");
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int c = 0; c < inst->numDis__; c++)
		{
			for (int p = 0; p < inst->numPerTot__; p++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- y_%d_%d >= 0\n", r, c);
		}
		fprintf(f, "\n");
	}
#endif // !REL_SAL_DIF

#endif // RES_SOFT
	fprintf(f, "\nBOUNDS\n");
	fprintf(f, "\n\\Variáveis x\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "0 <= x_%d_%d_%d <= 1\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < inst->numDia__; d++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "0 <= v_%d_%d <= 1\n", d, c);
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "0 <= q_%d <= %d\n", c, inst->numDia__);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < inst->numTur__; u++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int s = 0; s < inst->numPerDia__; s++)
				fprintf(f, "0 <= z_%d_%d_%d <= 1\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "0 <= y_%d_%d <= 1\n", r, c);
#endif
#ifdef RES_SOFT
	fprintf(f, "\nGENERALS\n"); // Para variáveis inteiras
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "q_%d\n", c);
#endif
	fprintf(f, "\nBINARIES\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
				fprintf(f, "x_%d_%d_%d\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < inst->numDia__; d++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "v_%d_%d\n", d, c);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < inst->numTur__; u++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int s = 0; s < inst->numPerDia__; s++)
				fprintf(f, "z_%d_%d_%d\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < inst->numSal__; r++)
		for (int c = 0; c < inst->numDis__; c++)
			fprintf(f, "y_%d_%d\n", r, c);
#endif
	fprintf(f, "\nEND");
	fclose(f);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//int getVetViabJanHor(Solucao* sol, Instancia* inst) {
//
//	int viavel = 1;
//
//	int pos = 0;
//	for (int u = 0; u < inst->numTur__; u++)
//	{
//		for (int d = 0; d < inst->numDia__; d++)
//		{
//			double soma = 0;
//			for (int c = 0; c < inst->numDis__; c++)
//				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
//				{
//					for (int r = 0; r < inst->numSal__; r++) {
//						int prim = d*inst->numPerDia__;
//						int seg = (d*inst->numPerDia__) + 1;
//						soma += sol->vetSol_[offset3D(r, prim , c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
//					}
//				}
//			double z = sol->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)];
//			sol->vetViabJanHor_[pos] = soma <= z ? 0 : soma - z;
//			if (sol->vetViabJanHor_[pos] != 0) {
//				viavel = 0;
//				//printf("RES_%d (z_%d_%d_%d): %f <= %f\n", pos, u, d, 0, soma, sol->vetSolZ_[pos]);
//			}
//				
//			pos++;
//		}
//	}
//
//	for (int u = 0; u < inst->numTur__; u++)
//	{
//		for (int d = 0; d < inst->numDia__; d++)
//		{
//			double soma = 0;
//			for (int c = 0; c < inst->numDis__; c++)
//				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
//				{
//					for (int r = 0; r < inst->numSal__; r++) {
//						int prim = (d*inst->numPerDia__) + inst->numPerDia__ - 1;
//						int seg = (d*inst->numPerDia__) + inst->numPerDia__ - 2;
//						soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
//					}
//				}
//			double z = sol->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)];
//			sol->vetViabJanHor_[pos] = soma <= z ? 0 : soma - z;
//			if (sol->vetViabJanHor_[pos] != 0) {
//				viavel = 0;
//				//printf("RES_%d (z_%d_%d_%d): %f <= %f\n", pos, u, d, 1, soma, sol->vetSolZ_[pos]);
//			}
//				
//			pos++;
//		}
//	}
//
//	for (int s = 2; s < inst->numPerDia__; s++)
//	{
//		for (int u = 0; u < inst->numTur__; u++)
//		{
//			for (int d = 0; d < inst->numDia__; d++)
//			{
//				double soma = 0;
//				for (int c = 0; c < inst->numDis__; c++)
//					if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
//					{
//						for (int r = 0; r < inst->numSal__; r++) {
//							int prim = (d*inst->numPerDia__) + s - 1;
//							int seg = (d*inst->numPerDia__) + s - 2;
//							int ter = (d*inst->numPerDia__) + s;
//							soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)] -
//								sol->vetSol_[offset3D(r, ter, c, inst->numPerTot__, inst->numDis__)];
//						}
//					}
//				double z = sol->vetSolZ_[offset3D(u, d, s, inst->numDia__, inst->numPerDia__)];
//				sol->vetViabJanHor_[pos] = soma <= z ? 0 : soma - z;
//				if (sol->vetViabJanHor_[pos] != 0) {
//					viavel = 0;
//					//printf("RES_%d (z_%d_%d_%d): %f <= %f\n", pos, u, d, s, soma, sol->vetSolZ_[pos]);
//				}
//					
//				pos++;
//			}
//		}
//	}
//
//	return viavel;
//}
////------------------------------------------------------------------------------
//
////------------------------------------------------------------------------------
//int getViabSalDif14(Solucao* sol, Instancia* inst) {
//
//	int viavel = 1;
//
//	int pos = 0;
//	for (int p = 0; p < inst->numPerTot__; p++) {
//		for (int r = 0; r < inst->numSal__; r++) {
//			for (int c = 0; c < inst->numDis__; c++) {
//				double x = sol->vetSol_[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)];
//				double y = sol->vetSolY_[offset2D(c, r, inst->numSal__)];
//				if (x <= y) {
//					sol->vetViab14_[pos] = 0;
//				}
//				else {
//					//printf("RES_%d (x_%d_%d_%d <= y_%d_%d): %f <= %f \n", pos, p, r, c, r, c, x, y);
//					sol->vetViab14_[pos] = x - y;
//					viavel = 0;
//				}
//				pos++;
//			}
//		}
//	}
//
//	return viavel;
//}
////------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//int getViabSalDif15(Solucao* sol, Instancia* inst) {
//
//	int viavel = 1;
//
//	int pos = 0;
//	for (int r = 0; r < inst->numSal__; r++) {
//		for (int c = 0; c < inst->numDis__; c++) {
//
//			double soma = 0;
//			for (int p = 0; p < inst->numPerTot__; p++) {
//				soma += sol->vetSol_[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)];
//			}
//
//			double y = sol->vetSolY_[offset2D(c, r, inst->numSal__)];
//			if (soma >= y) {
//				sol->vetViab15_[pos] = 0;
//			}
//			else {
//				//printf("RES_%d (y_%d_%d): %f >= %f \n", pos, r, c, soma, y);
//				sol->vetViab15_[pos] = soma - y;
//				viavel = 0;
//			}
//			pos++;
//		}
//	}
//
//	return viavel;
//}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void exportarCsv(Solucao* sol, char *arq, Instancia* inst, RestricoesRelaxadas* rest) {

	FILE *f = fopen(arq, "w");

	int numRest10 = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int posX, posZ, posY;

	for (int r = 0; r < inst->numSal__; r++) {
		for (int p = 0; p < inst->numPerTot__; p++) {
			for (int c = 0; c < inst->numDis__; c++) {

				fprintf(f,"x_%d_%d_%d,", p, r, c);
			}
		}
	}

	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				fprintf(f, "z_%d_%d_%d,", u, d, s);
			}
		}
	}

	for (int c = 0; c < inst->numDis__; c++) {
		fprintf(f, "q_%d,", c);
	}

	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			fprintf(f, "y_%d_%d,", r, c);
		}
	}

	for (int c = 0; c < inst->numDis__; c++) {
		for (int d = 0; d < inst->numDia__; d++) {
			fprintf(f, "v_%d_%d,", d, c);
		}
	}

	fprintf(f, "\n");

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	for (int i = 0; i < numX; i++) {
		fprintf(f, "%.3f,", sol->vetSol_[i]);
	}

	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	for (int i = 0; i < numZ; i++) {
		fprintf(f, "%.3f,", sol->vetSolZ_[i]);
	}

	int numQ = inst->numDis__;
	for (int i = 0; i < numQ; i++) {
		fprintf(f, "%.3f,", sol->vetSolQ_[i]);
	}

	int numY = inst->numSal__ * inst->numDis__;
	for (int i = 0; i < numY; i++) {
		fprintf(f, "%.3f,", sol->vetSolY_[i]);
	}

	int numV = inst->numDia__ * inst->numDis__;
	for (int i = 0; i < numV; i++) {
		fprintf(f, "%.3f,", sol->vetSolV_[i]);
	}

	fprintf(f, "\n");

	fprintf(f, "\nRestrições F10 a F13\n");
	for (int i = 0; i < numRest10; i++) {
		fprintf(f, "Res %d,", i);
		// Coeficientes de X
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					if (rest->vetRestJanHor__[i].coefMatX[posX] != 0)
						fprintf(f, "%dx_%d_%d_%d,", rest->vetRestJanHor__[i].coefMatX[posX], p, r, c);
				}
			}
		}

		// Coeficientes de Z
		for (int s = 0; s < inst->numPerDia__; s++) {
			for (int u = 0; u < inst->numTur__; u++) {
				for (int d = 0; d < inst->numDia__; d++) {
					posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
					if (rest->vetRestJanHor__[i].coefMatZ[posZ] != 0) {
						fprintf(f, "%dz_%d_%d_%d,", rest->vetRestJanHor__[i].coefMatZ[posZ], u, d, s);
					}
				}
			}
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\nRestrições F14\n");
	for (int i = 0; i < numRest14; i++) {
		fprintf(f, "Res %d,", i);
		// Coeficientes de X
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					if (rest->vetRest14__[i].coefMatX[posX] != 0)
						fprintf(f, "%dx_%d_%d_%d,", rest->vetRest14__[i].coefMatX[posX], p, r, c);
				}
			}
		}

		// Coeficientes de Y
		for (int c = 0; c < inst->numDis__; c++) {
			for (int r = 0; r < inst->numSal__; r++) {
				posY = offset2D(c, r, inst->numSal__);
				if (rest->vetRest14__[i].coefMatY[posY] != 0)
					fprintf(f, "%dy_%d_%d,", rest->vetRest14__[i].coefMatY[posY], r, c);
			}
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\nRestrições F15\n");
	for (int i = 0; i < numRest15; i++) {
		fprintf(f, "Res %d,", i);
		// Coeficientes de X
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					if (rest->vetRest15__[i].coefMatX[posX] != 0)
						fprintf(f, "%dx_%d_%d_%d,", rest->vetRest15__[i].coefMatX[posX], p, r, c);
				}
			}
		}

		// Coeficientes de Y
		for (int c = 0; c < inst->numDis__; c++) {
			for (int r = 0; r < inst->numSal__; r++) {
				posY = offset2D(c, r, inst->numSal__);
				if (rest->vetRest15__[i].coefMatY[posY] != 0)
					fprintf(f, "%dy_%d_%d,", rest->vetRest15__[i].coefMatY[posY], r, c);
			}
		}
		fprintf(f, "\n");
	}
	

	fclose(f);
}
//------------------------------------------------------------------------------

double** montaMatD(Instancia* inst, RestricoesRelaxadas* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;
	int numVar = numX + numY + numZ;
	printf("X = %d, Z = %d, Y = %d, numVar = %d\n", numX, numZ, numY, numVar);
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;
	printf("numRest10 = %d, numRest14 = %d, numRest15 = %d, numRes = %d\n", numRest10, numRest14, numRest15, numRes);
	int posX, posZ, posY;

	double** matTrans = (double**)malloc(numVar * sizeof(double*));
	for (int i = 0; i < numVar; i++) {
		matTrans[i] = (double*)malloc(numRes * sizeof(double));
	}

	// Transpondo a matriz
	int lin = 0;
	// Variáveis X
	for (int r = 0; r < inst->numSal__; r++) {
		for (int p = 0; p < inst->numPerTot__; p++) {
			for (int c = 0; c < inst->numDis__; c++) {

				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				int col = 0;
				for (int i = 0; i < numRest10; i++) {
					matTrans[lin][col] = rest->vetRestJanHor__[i].coefMatX[posX];
					col++;
				}

				for (int i = 0; i < numRest14; i++) {
					matTrans[lin][col] = rest->vetRest14__[i].coefMatX[posX];
					col++;
				}

				for (int i = 0; i < numRest15; i++) {
					matTrans[lin][col] = rest->vetRest15__[i].coefMatX[posX];
					col++;
				}

				lin++;
			}
		}
	}

	// Variáveis Z
	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {

				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				int col = 0;
				for (int i = 0; i < numRest10; i++) {
					matTrans[lin][col] = rest->vetRestJanHor__[i].coefMatZ[posZ];
					col++;
				}

				for (int i = 0; i < numRest14; i++) {
					matTrans[lin][col] = 0;
					col++;
				}

				for (int i = 0; i < numRest15; i++) {
					matTrans[lin][col] = 0;
					col++;
				}

				lin++;
			}
		}
	}


	// Variáveis Y
	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {

			posY = offset2D(c, r, inst->numSal__);

			int col = 0;
			for (int i = 0; i < numRest10; i++) {
				matTrans[lin][col] = 0;
				col++;
			}

			for (int i = 0; i < numRest14; i++) {
				matTrans[lin][col] = rest->vetRest14__[i].coefMatY[posY];
				col++;
			}

			for (int i = 0; i < numRest15; i++) {
				matTrans[lin][col] = rest->vetRest15__[i].coefMatY[posY];
				col++;
			}

			lin++;
		}
	}

	return matTrans;
}

void printMatD(Instancia* inst, double** matD) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	for (int i = 0; i < numRes; i++) {
		for (int j = 0; j < numX; j++) {
			printf("%.3f;", matD[i][j]);
		}
		printf("\n");
	}
}

// Lado direito das rstrições
double* montaVetD(Instancia* inst, Solucao* sol, RestricoesRelaxadas* rest) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	double* vetD = (double*)malloc(numRes * sizeof(double));

	int posRes10 = 0;
	int pos = 0;
	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			int posZ = offset3D(u, d, 0, inst->numDia__, inst->numPerDia__);
			vetD[pos] = rest->vetRestJanHor__[posRes10].coefMatZ[posZ] * sol->vetSolZ_[posZ];
			posRes10++;
			pos++;
		}
	}

	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			int posZ = offset3D(u, d, 1, inst->numDia__, inst->numPerDia__);
			vetD[pos] = rest->vetRestJanHor__[posRes10].coefMatZ[posZ] * sol->vetSolZ_[posZ];
			posRes10++;
			pos++;
		}
	}

	for (int s = 2; s < inst->numPerTot__; s++) {
		for (int u = 0; u < inst->numTur__; u++) {
			for (int d = 0; d < inst->numDia__; d++) {
				int posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				vetD[pos] = rest->vetRestJanHor__[posRes10].coefMatZ[posZ] * sol->vetSolZ_[posZ];
				posRes10++;
				pos++;
			}
		}
	}

	int posRes14 = 0;
	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {
				int posY = offset2D(c, r, inst->numSal__);
				vetD[pos] = rest->vetRest14__[posRes14].coefMatY[posY] * sol->vetSolY_[posY];
				posRes14++;
				pos++;
			}
		}
	}

	int posRes15 = 0;
	for (int r = 0; r < inst->numSal__; r++) {
		for (int c = 0; c < inst->numDis__; c++) {
			int posY = offset2D(c, r, inst->numSal__);
			vetD[pos] = rest->vetRest15__[posRes15].coefMatY[posY] * sol->vetSolY_[posY];
			posRes15++;
			pos++;
		}
	}

	return vetD;
}

void printVetD(Instancia* inst, double* vetD) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	for (int i = 0; i < numRes; i++) {
		printf("%f; ", vetD[i]);
	}
}

void printCoefsFO(Instancia* inst) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ * inst->numDis__;

	for (int i = 0; i < numX; i++) {
		printf("%f; ", inst->vetCoefX[i]);
	}

	for (int i = 0; i < numZ; i++) {
		printf("%f; ", inst->vetCoefZ[i]);
	}

	for (int i = 0; i < numY; i++) {
		printf("%f; ", inst->vetCoefY[i]);
	}		
}

void escreveCSVDebugCoefs(char* arq, Instancia* inst, Solucao* sol, double** matD, double* vetD, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;
	int numVar = numX + numY + numZ;
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	FILE* f = fopen(arq, "w");

	// ----------------- NOMES DAS VARIAVEIS -------------------------------------
	for (int r = 0; r < inst->numSal__; r++) {
		for (int p = 0; p < inst->numPerTot__; p++) {
			for (int c = 0; c < inst->numDis__; c++) {
				fprintf(f, "x_%d_%d_%d,", p,r,c);
			}
		}
	}

	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				fprintf(f, "z_%d_%d_%d,", u,d,s);
			}
		}
	}

	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			fprintf(f, "y_%d_%d,", r,c);
		}
	}
	fprintf(f, "\n\n");
	// ---------------------------------------------------------------------------

	// ----------------------COEFS FO---------------------------------------------
	int posX;
	fprintf(f, "COEFS FO\n");
	for (int r = 0; r < inst->numSal__; r++) {
		for (int p = 0; p < inst->numPerTot__; p++) {
			for (int c = 0; c < inst->numDis__; c++) {
				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				fprintf(f, "%.6f,", inst->vetCoefX[posX]);
			}
		}
	}

	int posZ;
	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				fprintf(f, "%.6f,", inst->vetCoefZ[posZ]);
			}
		}
	}


	int posY;
	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			posY = offset2D(c, r, inst->numSal__);
			fprintf(f, "%.6f,", inst->vetCoefY[posY]);
		}
	}

	fprintf(f, "%.6f", -1.0);
	// --------------------------------------------------------------

	fprintf(f, "\n\n");

	// ------------SOLUÇÃO-------------------------------------------
	fprintf(f, "SOLUÇÃO\n");
	for (int r = 0; r < inst->numSal__; r++) {
		for (int p = 0; p < inst->numPerTot__; p++) {
			for (int c = 0; c < inst->numDis__; c++) {
				posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				fprintf(f, "%.6f,", sol->vetSol_[posX]);
			}
		}
	}

	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				fprintf(f, "%.6f,", sol->vetSolZ_[posZ]);
			}
		}
	}


	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			posY = offset2D(c, r, inst->numSal__);
			fprintf(f, "%.6f,", sol->vetSolY_[posY]);
		}
	}
	// --------------------------------------------------------------

	fprintf(f, "\n\n");

	fprintf(f, "MAT D\n");
	for (int i = 0; i < numVar; i++) {
		for (int j = 0; j < numRes; j++) {
			fprintf(f, "%.6f,", matD[i][j]);
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\n\n");

	fprintf(f, "VET MULT\n");
	for (int i = 0; i < numRest10; i++) {
		fprintf(f, "%.6f\n", vetMultRes10[i]);
	}
	for (int i = 0; i < numRest14; i++) {
		fprintf(f, "%.6f\n", vetMultRes14[i]);
	}
	for (int i = 0; i < numRest15; i++) {
		fprintf(f, "%.6f\n", vetMultRes15[i]);
	}

	fprintf(f, "\n\n");

	fprintf(f, "VET d\n");
	for (int i = 0; i < numRes; i++) {
		fprintf(f, "%.6f\n", vetD[i]);
	}

	fclose(f);
}

void desalocaMatD(double** matD, int nLin) {

	for (int i = 0; i < nLin; i++) {
		free(matD[i]);
	}
	free(matD);
}

void escreveMatRestCplex(char* arq, MatRestCplex* matRest, Instancia* inst) {

	FILE* f = fopen(arq, "w");

	fprintf(f, "numCoefsTotal=");
	fprintf(f, "%d\n", matRest->numCoefsTotal);
	fprintf(f, "numCol=");
	fprintf(f, "%d\n", matRest->numCol); 
	fprintf(f, "numLin=");
	fprintf(f, "%d\n", matRest->numLin);

	// matbeg
	fprintf(f, "matbeg\n");
	for (int i = 0; i < matRest->numCol; i++) {
		fprintf(f, "%d;", matRest->matbeg[i]);
	}
	fprintf(f, "\n");

	// matcnt
	fprintf(f, "matcnt\n");
	for (int i = 0; i < matRest->numCol; i++) {
		fprintf(f, "%d;", matRest->matcnt[i]);
	}
	fprintf(f, "\n");

	// matval
	fprintf(f, "matval\n");
	for (int i = 0; i < matRest->numCoefsTotal; i++) {
		fprintf(f, "%d;", matRest->matval[i]);
	}
	fprintf(f, "\n");

	// matind
	fprintf(f, "matind\n");
	for (int i = 0; i < matRest->numCoefsTotal; i++) {
		fprintf(f, "%d;", matRest->matind[i]);
	}

	fclose(f);
}


void ordenaMatBegEMatVal(MatRestCplex* matRest, int ini, int fim) {

	int pos;

	if (ini < fim) {
		pos = particiona(matRest, ini, fim);
		ordenaMatBegEMatVal(matRest, ini, pos);
		ordenaMatBegEMatVal(matRest, pos + 1, fim);
	}
}


int particiona(MatRestCplex* matRest, int ini, int fim) {

	int pivo = matRest->matind[ini];
	int i = ini - 1;
	int j = fim + 1;

	while (1) {

		do {
			i++;
		} while (matRest->matind[i] < pivo);

		do {
			j--;
		} while (matRest->matind[j] > pivo);

		if (i >= j) {
			return j;
		}

		swap(&matRest->matind[i], &matRest->matind[j]);
		swap(&matRest->matval[i], &matRest->matval[j]);
	}
}


MatRestCplex* montaMatRestCplex(Instancia* inst, int numVar, int numRest) {

	int numCoefsTotal = contaCoefRestJanHor(inst) + contaCoefRestSalDif(inst);
	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ * inst->numDis__;

	MatRestCplex* matRest = (MatRestCplex*)malloc(sizeof(MatRestCplex));
	matRest->numCoefsTotal = numCoefsTotal;
	matRest->numCol = numVar;
	matRest->numLin = numRest;

	printf("NUM COEFS: %d\n", numCoefsTotal);

	matRest->matval = (int*)malloc(matRest->numCoefsTotal * sizeof(int));
	matRest->matbeg = (int*)malloc(numRest * sizeof(int));
	matRest->matind = (int*)malloc(matRest->numCoefsTotal * sizeof(int));
	matRest->matcnt = (int*)malloc(numRest * sizeof(int));

	int posX1, posX2, posX3, posZ, posY;
	int lin = 0;
	int pos = 0;
	int numCoefs = 0;

	// Primeiro período do dia
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			matRest->matbeg[lin] = pos;
			for (int c = 0; c < inst->numDis__; c++) {
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						posX1 = offset3D(r, d*inst->numPerDia__, c, inst->numPerTot__, inst->numDis__);
						posX2 = offset3D(r, d*inst->numPerDia__ + 1, c, inst->numPerTot__, inst->numDis__);

						matRest->matval[pos] = 1;
						matRest->matind[pos] = posX1;
						pos++;
						numCoefs++;

						matRest->matval[pos] = -1;
						matRest->matind[pos] = posX2;
						pos++;
						numCoefs++;
					}
				}
			}

			posZ = offset3D(u, d, 0, inst->numDia__, inst->numPerDia__);
			matRest->matval[pos] = -1;
			matRest->matind[pos] = posZ + numX;
			pos++;
			numCoefs++;

			matRest->matcnt[lin] = numCoefs;

			ordenaMatBegEMatVal(matRest, pos - numCoefs, pos - 1);

			numCoefs = 0;
			lin++;
		}
	}

	// Último período do dia
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			matRest->matbeg[lin] = pos;
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						posX1 = offset3D(r, (d*inst->numPerDia__) + inst->numPerDia__ - 1, c, inst->numPerTot__, inst->numDis__);
						posX2 = offset3D(r, (d*inst->numPerDia__) + inst->numPerDia__ - 2, c, inst->numPerTot__, inst->numDis__);

						matRest->matval[pos] = 1;
						matRest->matind[pos] = posX1;
						pos++;
						numCoefs++;

						matRest->matval[pos] = -1;
						matRest->matind[pos] = posX2;
						pos++;
						numCoefs++;
					}
				}

			posZ = offset3D(u, d, 1, inst->numDia__, inst->numPerDia__);
			matRest->matval[pos] = -1;
			matRest->matind[pos] = posZ + numX;
			pos++;
			numCoefs++;

			matRest->matcnt[lin] = numCoefs;

			ordenaMatBegEMatVal(matRest, pos - numCoefs, pos - 1);

			numCoefs = 0;
			lin++;
		}
	}

	// Períodos intermediários do dia
	for (int s = 2; s < inst->numPerDia__; s++)
	{
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				matRest->matbeg[lin] = pos;
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++) {
							posX1 = offset3D(r, (d*inst->numPerDia__) + s - 1, c, inst->numPerTot__, inst->numDis__);
							posX2 = offset3D(r, (d*inst->numPerDia__) + s - 2, c, inst->numPerTot__, inst->numDis__);
							posX3 = offset3D(r, (d*inst->numPerDia__) + s, c, inst->numPerTot__, inst->numDis__);

							matRest->matval[pos] = 1;
							matRest->matind[pos] = posX1;
							pos++;
							numCoefs++;

							matRest->matval[pos] = -1;
							matRest->matind[pos] = posX1;
							pos++;
							numCoefs++;

							matRest->matval[pos] = -1;
							matRest->matind[pos] = posX1;
							pos++;
							numCoefs++;
						}
					}

				posZ = offset3D(u, d, s, inst->numDia__, inst->numPerDia__);
				matRest->matval[pos] = -1;
				matRest->matind[pos] = posZ + numX;
				pos++;
				numCoefs++;

				matRest->matcnt[lin] = numCoefs;

				ordenaMatBegEMatVal(matRest, pos - numCoefs, pos - 1);

				numCoefs = 0;
				lin++;
			}
		}
	}

	// Restrição 14
	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {

				numCoefs = 0;
				matRest->matbeg[lin] = pos;

				posX1 = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				posY = offset2D(c, r, inst->numSal__);

				matRest->matval[pos] = 1;
				matRest->matind[pos] = posX1;
				numCoefs++;
				pos++;

				matRest->matval[pos] = -1;
				matRest->matind[pos] = posY + numX + numZ;
				numCoefs++;
				pos++;

				matRest->matcnt[lin] = numCoefs;
				
				lin++;
			}
		}
	}


	// Restrição 15
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int c = 0; c < inst->numDis__; c++)
		{
			numCoefs = 0;
			matRest->matbeg[lin] = pos;
			for (int p = 0; p < inst->numPerTot__; p++) {
				posX1 = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				matRest->matval[pos] = 1;
				matRest->matind[pos] = posX1;
				pos++;
				numCoefs++;
			}
			
			posY = offset2D(c, r, inst->numSal__);
			matRest->matval[pos] = -1;
			matRest->matind[pos] = posY + numX + numZ;
			pos++;
			numCoefs++;

			matRest->matcnt[lin] = numCoefs;

			lin++;
		}
	}
	
	return matRest;
}
