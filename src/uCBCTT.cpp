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

//==============================================================================

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char nomeInst[10];
	strcpy_s(nomeInst, INST);
	strcat_s(nomeInst, "01");

	/*double a = -1.0;
	double b = 2;
	printf("MAX(%f, %f) = %f\n", a, b, MAX(a, b));
	printf("MIN(%f, %f) = %f\n", a, b, MIN(a, b));
	printf("MAX(%f, %f) = %f\n", -a, -b, MAX(-a, -b));
	printf("MIN(%f, %f) = %f\n", -a, -b, MIN(-a, -b));*/

	execUma(nomeInst);
	//execTodas();

	/*lerInstancia("instances\\toy.ctt");
	initVetJanHor();
	montaMatCoefXFO();
	montaCoefRestJanHor();
	initMultiplicadores();

	char* arq = "teste.txt";
	montarModeloRelaxado(arq);*/

	printf("\n\n>>> Pressione ENTER para encerrar: ");
	_getch();
	return 0;
}
//------------------------------------------------------------------------------

void execUma(char* nomeInst) {
	char aux[150];
	Solucao* sol;

	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
	strcat_s(aux, ".ctt");
	printf("%s\n", aux);
	Instancia* inst = lerInstancia(aux);
	initCoefsFO(inst);
	//lerSolucoesIniciais();

	//testarEntrada();
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

	initCoefsFO(inst);

	printf("Inicializando os vetores de restricoes 10\n");
	inst->vetRestJanHor__ =  getVetJanHor(inst, numRest10);
	printf("Inicializando os vetores de restricoes 14\n");
	inst->vetRest14__ = getVetSalDif(inst, numRest14);
	printf("Inicializando os vetores de restricoes 15\n");
	inst->vetRest15__ = getVetSalDif(inst, numRest15);

	printf("Montando as matrizes de coeficientes das restricoes de Janela Horario\n");
	montaCoefRestJanHor(inst);
	printf("Montando as matrizes de coeficientes das restricoes de Salas Diferentes\n");
	montaCoefRestSalDif(inst);

	double* vetMultRes10 = (double*) malloc(numRest10 *sizeof(double));
	double* vetMultRes14 = (double*) malloc(numRest14 * sizeof(double));
	double* vetMultRes15 = (double*) malloc(numRest15 * sizeof(double));

	printf("Inicializando vetor de multiplicadores das restricoes 10\n");
	initMultiplicadores(vetMultRes10, numRest10,VAL_INIT_ALPHA);
	printf("Inicializando vetor de multiplicadores das restricoes 14\n");
	initMultiplicadores(vetMultRes14, numRest14, VAL_INIT_RES_14);
	printf("Inicializando vetor de multiplicadores das restricoes 15\n");
	initMultiplicadores(vetMultRes15, numRest15, VAL_INIT_RES_15);

#else
	montarModeloPLI(aux, inst);
#endif
	printf("Executando Relaxacao Lagrangiana\n");
	sol = execRelLagran(aux, inst, vetMultRes10, vetMultRes14, vetMultRes15);
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
	printf("Escrevendo Solucao\n");
	escreverSol(sol, aux, inst);
	free(inst);
}

//void execTodas() {
//	for (int i = 1; i <= NUM_INST; i++) {
//
//		char aux[150];
//		char nomeInst[20];
//		char temp[10];
//		Solucao* sol;
//
//		strcpy_s(aux, PATH_INST);
//		strcpy_s(nomeInst, INST);
//		if (i < 10) {
//			strcat_s(nomeInst, "0");
//			sprintf_s(temp, "%d", i);
//			strcat_s(nomeInst, temp);
//		}
//		else {
//			if (i == 11)
//				continue;
//			sprintf(temp, "%d", i);
//			strcat_s(nomeInst, temp);
//		}
//		strcat_s(aux, nomeInst);
//		strcat_s(aux, ".ctt");
//		Instancia* inst = lerInstancia(aux);
//		//lerSolucoesIniciais();
//
//		//testarEntrada();
//		strcpy_s(aux, PATH_INST);
//		strcat_s(aux, nomeInst);
//#ifndef RES_SOFT
//		strcat_s(aux, "-H");
//#endif 
//		strcat_s(aux, ".lp");
//		montarModeloPLI(aux,inst);
//		sol = execCpx(aux, inst);
//		strcpy_s(aux, PATH_INST);
//		strcat_s(aux, nomeInst);
//#ifndef RES_SOFT
//		strcat_s(aux, "-H");
//#endif 
//		strcat_s(aux, ".sol");
//		escreverSol(sol, aux, inst);
//		free(inst);
//	}
//}
//------------------------------------------------------------------------------

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
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Solucao* execCpx(char *arq, Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15)
{
	int sts;
	Solucao* s = (Solucao*) malloc(sizeof(Solucao));
	clock_t h;
	CPXENVptr env;
	CPXLPptr lp;
	env = CPXopenCPLEX(&sts);
	sts = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
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
#ifdef RES_CAP_SAL
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
#endif

#ifdef RES_JAN_HOR
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
#endif

#ifdef RES_DIA_MIN
	fprintf(f, "\n\\Dias mínimos\n");
	for (int c = 0; c < inst->numDis__; c++)
		fprintf(f, "+ %d q_%d ", PESOS[2], c);
#endif

#ifdef RES_SAL_DIF
	fprintf(f, "\n\n\\Salas diferentes\n");
	for (int c = 0; c < inst->numDis__; c++)
	{
		for (int r = 0; r < inst->numSal__; r++)
			fprintf(f, "+ %d y_%d_%d ", PESOS[3], r, c);
		fprintf(f, "\n");
	}
#endif

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
	fprintf(f, "\n\\R10 a R13+#PER_DIA - Janelas no horário das turmas\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[c][u] == 1)
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
				if (inst->matDisTur__[c][u] == 1)
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
					if (inst->matDisTur__[c][u] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++)
							fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d - x_%d_%d_%d ", (d*inst->numPerDia__) + s - 1, r, c, (d*inst->numPerDia__) + s - 2, r, c, (d*inst->numPerDia__) + s, r, c);
					}
				fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, s);
			}
		}
		fprintf(f, "\n");
	}
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

//------------------------------------------------------------------------------
void initMultiplicadores(double* vetMult, int tam, double val) {

	for (int i = 0; i < tam; i++) {
		vetMult[i] = val;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloRelaxado(char *arq, Instancia* inst, double* vetAlpha, double* vetMultRes14, double* vetMultRes15) {

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
	fprintf(f, "val = %d\n", val);

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
				if (matDisTur__[c][u] == 1)
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
				if (matDisTur__[c][u] == 1)
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
					if (matDisTur__[c][u] == 1)
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
//				if (inst->matDisTur__[c][u] == 1)
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
//				if (inst->matDisTur__[c][u] == 1)
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
//					if (inst->matDisTur__[c][u] == 1)
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
void exportarCsv(Solucao* sol, char *arq, Instancia* inst) {

	FILE *f = fopen(arq, "w");

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

	fclose(f);
}
//------------------------------------------------------------------------------
