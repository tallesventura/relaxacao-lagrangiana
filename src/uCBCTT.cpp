#include "uCBCTT.h"
#include "RelLagran.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include "..\lib\cplex\include\cplex.h"
#include "Colecoes.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))

#define RES_SOFT
#define RES_CAP_SAL // Restrição soft de capacidade das salas
#define	RES_JAN_HOR // Restrição soft de janelas de horários
#define RES_DIA_MIN // Restrição soft de dias mínimos
#define RES_SAL_DIF // Restrição soft de salas diferentes

#define RELAXAR

//#define REL_CAP_SAL // Relaxar restrição soft de capacidade das salas
#define	REL_JAN_HOR // Relaxar restrição soft de janelas de horários
//#define REL_DIA_MIN // Relaxar restrição soft de dias mínimos
//#define REL_SAL_DIF // Relaxar restrição soft de salas diferentes


// ------------ Auxiliares
int matDisTur__[MAX_DIS][MAX_TUR]; // Dis x Cur; 1 se a disciplina d faz parte do currículo c; 0 caso contrário

RestJanHor *vetRestJanHor__; // Vetor com as restrições de janela horário
int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]; // Matriz de coeficientes das variáveis x da FO.

//char INST[50] = "comp";
char INST[50] = "toy";
int PESOS[4] = { 1,2,5,1 };

//==============================================================================


//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char nomeInst[10];
	strcpy_s(nomeInst, INST);
	//strcat_s(nomeInst, "3");

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
	printf("%s", aux);
	Instancia* inst = lerInstancia(aux);
	//lerSolucoesIniciais();

	//testarEntrada();
	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
	strcat_s(aux, ".lp");
#ifdef RELAXAR
	int numRest = inst->numTur__*inst->numDia__*inst->numPerDia__;
	initVetJanHor(inst);
	montaMatCoefXFO(inst);
	montaCoefRestJanHor(inst);
	double* vetAlpha = (double*) malloc(numRest*sizeof(double));
	initMultiplicadores(vetAlpha,numRest,VAL_INIT_ALPHA);
	montarModeloRelaxado(aux,inst,vetAlpha);
#else
	montarModeloPLI(aux);
#endif
	sol = execCpx(aux, inst);
	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
	strcat_s(aux, ".sol");
	escreverSol(sol, aux, inst);
	free(inst);
}

void execTodas() {
	for (int i = 1; i <= NUM_INST; i++) {

		char aux[150];
		char nomeInst[20];
		char temp[10];
		Solucao* sol;

		strcpy_s(aux, PATH_INST);
		strcpy_s(nomeInst, INST);
		if (i < 10) {
			strcat_s(nomeInst, "0");
			sprintf_s(temp, "%d", i);
			strcat_s(nomeInst, temp);
		}
		else {
			if (i == 11)
				continue;
			sprintf(temp, "%d", i);
			strcat_s(nomeInst, temp);
		}
		strcat_s(aux, nomeInst);
		strcat_s(aux, ".ctt");
		Instancia* inst = lerInstancia(aux);
		//lerSolucoesIniciais();

		//testarEntrada();
		strcpy_s(aux, PATH_INST);
		strcat_s(aux, nomeInst);
#ifndef RES_SOFT
		strcat_s(aux, "-H");
#endif 
		strcat_s(aux, ".lp");
		montarModeloPLI(aux,inst);
		sol = execCpx(aux, inst);
		strcpy_s(aux, PATH_INST);
		strcat_s(aux, nomeInst);
#ifndef RES_SOFT
		strcat_s(aux, "-H");
#endif 
		strcat_s(aux, ".sol");
		escreverSol(sol, aux, inst);
		free(inst);
	}
}
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
Solucao* execCpx(char *arq, Instancia* inst)
{
	int sts;
	Solucao* s = (Solucao*) malloc(sizeof(Solucao));
	clock_t h;
	CPXENVptr env;
	CPXLPptr lp;
	env = CPXopenCPLEX(&sts);
	sts = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
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

	int pos = 0;
	printf("\n");
	for (int c = 0; c < inst->numDis__; c++) {
		printf("q_%d = %.2f\n", c, s->vetSolQ_[c]);
	}
	printf("\n");

	double* vetViabJanHor = (double*)malloc(inst->numTur__*inst->numDia__*inst->numPerDia__ * sizeof(double));
	int viavel = getVetViabJanHor(s, vetViabJanHor, inst);
	if (viavel) {
		printf("JanHor VIAVEL\n");
	}
	else {
		printf("JanHor INVIAVEL:\n");
		for (int i = 0; i < inst->numTur__*inst->numDia__*inst->numPerDia__; i++) {
			printf("RES_%d: %.2f\n", i, vetViabJanHor[i]);
		}
	}
	
	int pfeas, dfeas;
	sts = CPXsolninfo(env, lp, NULL, NULL, &pfeas, &dfeas);
	if (sts == 0) {
		printf("\nPrimal feasible: %d\n", pfeas);
		printf("Dual feasible: %d\n", dfeas);
	}

	sts = CPXfreeprob(env, &lp);
	sts = CPXcloseCPLEX(&env);

	return s;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void escreverSol(Solucao* s, char *arq, Instancia* inst)
{
	int pos, dia, per, aux;
	FILE *f = fopen(arq, "w");
	fprintf(f, ">>> RESULTADOS DO CPLEX\n\n");
	fprintf(f, "Num. var....: %d\n", s->numVar_);
	fprintf(f, "Num. res....: %d\n", s->numRes_);
	fprintf(f, "Val. sol....: %.2f\n", s->valSol_);
	fprintf(f, "Melhor no...: %.2f\n", s->bstNod_);
	if (s->valSol_ != 0)
		fprintf(f, "GAP.........: %.2f\n", ((s->valSol_ - s->bstNod_) / s->valSol_) * 100);
	else
		fprintf(f, "GAP.........: -\n");
	fprintf(f, "Tempo.......: %.2f\n", s->tempo_);
	//------------------------------------------------------------- 
	// preencher a solucao
	fprintf(f, "\n\n>>> SOLUCAO (<periodo><sala><disciplina>)\n\n", s->tempo_);
	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int r = 0; r < inst->numSal__; r++)
				s->matSolSal_[p][d][r] = -1;
	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int u = 0; u < inst->numTur__; u++)
				s->matSolTur_[p][d][u] = -1;
	pos = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
			{
				if (s->vetSol_[pos] > 0)
				{
					dia = p / inst->numPerDia__;
					per = p % inst->numPerDia__;
					fprintf(f, "x_%d_%d_%d = %.2f\n", p, r, c, s->vetSol_[pos]);
					if (s->matSolSal_[per][dia][r] != -1)
					{
						printf("\n\nERRO - SALA %d- p%d d%d c%d!\n\n", r, per, dia, c);
						_getch();
					}
					else
						s->matSolSal_[per][dia][r] = c;
					for (int u = 0; u < inst->numTur__; u++)
					{
						if (matDisTur__[c][u] == 1)
						{
							if (s->matSolTur_[per][dia][u] != -1)
							{
								printf("\n\nERRO - TURMA %d - p%d d%d c%d!\n\n", u, per, dia, c);
								_getch();
							}
							else
								s->matSolTur_[per][dia][u] = c;
						}
					}
				}
				pos++;
			}
	fprintf(f, "\n");
	for (int r = 0; r < inst->numSal__; r++)
	{
		fprintf(f, "SALA %d\n", r);
		for (int p = 0; p < inst->numPerDia__; p++)
		{
			for (int d = 0; d < inst->numDia__; d++)
				fprintf(f, "%d  ", s->matSolSal_[p][d][r]);
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		fprintf(f, "TURMA %d\n", u);
		for (int p = 0; p < inst->numPerDia__; p++)
		{
			for (int d = 0; d < inst->numDia__; d++)
				fprintf(f, "%d  ", s->matSolTur_[p][d][u]);
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	//------------------------------------------------------------- 
	// verificar a solucao
	// HARD
	s->vioNumAul_ = 0;
	for (int c = 0; c < inst->numDis__; c++)
	{
		aux = 0;
		for (int r = 0; r < inst->numSal__; r++)
		{
			for (int p = 0; p < inst->numPerDia__; p++)
				for (int d = 0; d < inst->numDia__; d++)
					if (s->matSolSal_[p][d][r] == c)
						aux++;
		}
		s->vioNumAul_ += abs(inst->vetDisciplinas__[c].numPer_ - aux);
	}

	s->vioAulSim_ = 0;
	s->vioDisSim_ = 0;
	s->vioProSim_ = 0;
	s->vioTurSim_ = 0;
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int p = 0; p < inst->numPerDia__; p++)
			{
				if (s->matSolSal_[p][d][r] != -1)
					for (int r2 = r + 1; r2 < inst->numSal__; r2++)
					{
						if (s->matSolSal_[p][d][r2] != -1)
						{
							if (s->matSolSal_[p][d][r2] == s->matSolSal_[p][d][r])
								s->vioDisSim_++;
							if (inst->vetDisciplinas__[s->matSolSal_[p][d][r2]].professor_ == inst->vetDisciplinas__[s->matSolSal_[p][d][r]].professor_)
								s->vioProSim_++;
							for (int u = 0; u < inst->numTur__; u++)
								if ((matDisTur__[s->matSolSal_[p][d][r2]][u] == 1) && (matDisTur__[s->matSolSal_[p][d][r]][u] == 1))
									s->vioTurSim_++;
						}
					}
			}
		}
	}
	// SOFT
	s->capSal_ = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerDia__; p++)
			for (int d = 0; d < inst->numDia__; d++)
			{
				if (s->matSolSal_[p][d][r] != -1)
					if (inst->vetDisciplinas__[s->matSolSal_[p][d][r]].numAlu_ > inst->vetSalas__[r].capacidade_)
						s->capSal_+= inst->vetDisciplinas__[s->matSolSal_[p][d][r]].numAlu_ - inst->vetSalas__[r].capacidade_;
			}

	s->janHor_ = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			if ((s->matSolTur_[0][d][u] != -1) && (s->matSolTur_[1][d][u] == -1))
				s->janHor_++;
			if ((s->matSolTur_[inst->numPerDia__ - 1][d][u] != -1) && (s->matSolTur_[inst->numPerDia__ - 2][d][u] == -1))
				s->janHor_++;
			for (int p = 2; p < inst->numPerDia__; p++)
				if ((s->matSolTur_[p - 1][d][u] != -1) && (s->matSolTur_[p - 2][d][u] == -1) && (s->matSolTur_[p][d][u] == -1))
					s->janHor_++;
		}
	}

	s->diaMin_ = 0;
	for (int c = 0; c < inst->numDis__; c++)
	{
		aux = 0;
		for (int d = 0; d < inst->numDia__; d++)
		{
			pos = 0;
			for (int p = 0; p < inst->numPerDia__; p++)
			{
				for (int r = 0; r < inst->numSal__; r++)
				{
					if (s->matSolSal_[p][d][r] == c)
					{
						pos = 1;
						break;
					}
				}
				if (pos == 1)
					break;
			}
			aux += pos;
		}
		if (aux < inst->vetDisciplinas__[c].diaMin_)
			s->diaMin_+= inst->vetDisciplinas__[c].diaMin_ - aux;
	}
	s->salDif_ = 0;
	for (int c = 0; c < inst->numDis__; c++)
	{
		aux = 0;
		for (int r = 0; r < inst->numSal__; r++)
		{
			pos = 0;
			for (int d = 0; d < inst->numDia__; d++)
			{
				for (int p = 0; p < inst->numPerDia__; p++)
				{
					if (s->matSolSal_[p][d][r] == c)
					{
						pos = 1;
						break;
					}
				}
				if (pos == 1)
					break;
			}
			aux += pos;
		}
		s->salDif_ += aux - 1;
	}

	s->funObj_ = PESOS[0] * s->capSal_ + PESOS[1] * s->janHor_ + PESOS[2] * s->diaMin_ + PESOS[3] * s->salDif_;
	fprintf(f, "\n\n>>> RESULTADOS CALCULADOS\n\n", s->valSol_);
	fprintf(f, "Func. obj.....: %d\n", s->funObj_);
	fprintf(f, "\n\nHARD----------------------------------\n\n");
	fprintf(f, "Num. aulas....: %d\n", s->vioNumAul_);
	fprintf(f, "Aulas simul...: %d\n", s->vioAulSim_);
	fprintf(f, "Disc. simul...: %d\n", s->vioDisSim_);
	fprintf(f, "Prof. simul...: %d\n", s->vioProSim_);
	fprintf(f, "Turm. simul...: %d\n", s->vioTurSim_);
	fprintf(f, "\n\nSOFT------------------------------------\n\n");
	fprintf(f, "Cap. salas....: %d\n", s->capSal_);
	fprintf(f, "Jan. turmas...: %d\n", s->janHor_);
	fprintf(f, "Dias minimo...: %d\n", s->diaMin_);
	fprintf(f, "Salas difer...: %d\n", s->salDif_);
	fclose(f);
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
Instancia* lerInstancia(char *arq)
{
	Instancia* inst = (Instancia*) malloc(sizeof(Instancia));
	int pos;
	char aux[50];
	FILE *f = fopen(arq, "r");
	fscanf(f, "Name: %s\n", &inst->nomInst__);
	fscanf(f, "Courses: %d\n", &inst->numDis__);
	fscanf(f, "Rooms: %d\n", &inst->numSal__);
	fscanf(f, "Days: %d\n", &inst->numDia__);
	fscanf(f, "Periods_per_day: %d\n", &inst->numPerDia__);
	fscanf(f, "Curricula: %d\n", &inst->numTur__);
	fscanf(f, "Constraints: %d\n", &inst->numRes__);
	fscanf(f, "\nCOURSES:\n");
	inst->numPerTot__ = inst->numDia__ * inst->numPerDia__;
	inst->numPro__ = 0;
	for (int i = 0; i < inst->numDis__; i++)
	{
		fscanf(f, "%s %s %d %d %d\n", &inst->vetDisciplinas__[i].nome_, &aux,
			&inst->vetDisciplinas__[i].numPer_, &inst->vetDisciplinas__[i].diaMin_, &inst->vetDisciplinas__[i].numAlu_);
		pos = -1;
		for (int p = 0; p < inst->numPro__; p++)
			if (strcmp(aux, inst->vetProfessores__[p].nome_) == 0)
			{
				inst->vetDisciplinas__[i].professor_ = p;
				pos = p;
				break;
			}
		if (pos == -1)
		{
			inst->vetDisciplinas__[i].professor_ = inst->numPro__;
			strcpy_s(inst->vetProfessores__[inst->numPro__].nome_, aux);
			inst->numPro__++;
		}
	}
	for (int i = 0; i < inst->numDis__; i++)
		for (int j = 0; j < inst->numTur__; j++)
			matDisTur__[i][j] = 0;
	fscanf(f, "\nROOMS:\n");
	for (int i = 0; i < inst->numSal__; i++)
		fscanf(f, "%s %d\n", &inst->vetSalas__[i].nome_, &inst->vetSalas__[i].capacidade_);
	fscanf(f, "\nCURRICULA:\n");
	for (int i = 0; i < inst->numTur__; i++)
	{
		fscanf(f, "%s %d", &inst->vetTurmas__[i].nome_, &inst->vetTurmas__[i].numDis_);
		for (int j = 0; j < inst->vetTurmas__[i].numDis_; j++)
		{
			fscanf(f, "%s ", &aux);
			inst->vetTurmas__[i].vetDis_[j] = -1;
			for (int k = 0; k < inst->numDis__; k++)
				if (strcmp(aux, inst->vetDisciplinas__[k].nome_) == 0)
				{
					inst->vetTurmas__[i].vetDis_[j] = k;
					matDisTur__[k][i] = 1;
					break;
				}
		}
	}
	fscanf(f, "\nUNAVAILABILITY_CONSTRAINTS:\n");
	for (int i = 0; i < inst->numRes__; i++)
	{
		fscanf(f, "%s %d %d\n", &aux, &pos, &inst->vetRestricoes__[i].periodo_);
		inst->vetRestricoes__[i].periodo_ = (pos * inst->numPerDia__) + inst->vetRestricoes__[i].periodo_;
		inst->vetRestricoes__[i].disciplina_ = -1;
		for (int j = 0; j < inst->numDis__; j++)
			if (strcmp(aux, inst->vetDisciplinas__[j].nome_) == 0)
			{
				inst->vetRestricoes__[i].disciplina_ = j;
				break;
			}
	}
	fclose(f);
	return inst;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initRestJanHor(RestJanHor *rest) {

	for (int u = 0; u < MAX_TUR; u++) {
		for (int d = 0; d < MAX_DIA; d++) {
			for (int s = 0; s < MAX_PER; s++) {
				rest->coefMatZ[u][d][s] = 0;
			}
		}
	}

	for (int p = 0; p < MAX_PER * MAX_DIA; p++) {
		for (int r = 0; r < MAX_SAL; r++) {
			for (int c = 0; c < MAX_DIS; c++) {
				rest->coefMatX[p][r][c] = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initVetJanHor(Instancia* inst) {

	int numRest = inst->numTur__*inst->numDia__*inst->numPerDia__;
	vetRestJanHor__ = (RestJanHor*)malloc(numRest * sizeof(RestJanHor));

	for (int i = 0; i < numRest; i++) {
		initRestJanHor(&vetRestJanHor__[i]);
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaMatCoefXFO(Instancia* inst) {

	for (int p = 0; p < MAX_PER * MAX_DIA; p++) {
		for (int r = 0; r < MAX_SAL; r++) {
			for (int c = 0; c < MAX_DIS; c++) {
				if (inst->vetDisciplinas__[c].numAlu_ > inst->vetSalas__[r].capacidade_)
					coefMatXFO[p][r][c] = (inst->vetDisciplinas__[c].numAlu_ - inst->vetSalas__[r].capacidade_);
				else
					coefMatXFO[p][r][c] = 0;
			}
		}
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
void montaCoefRestJanHor(Instancia* inst) {

	int numRest = inst->numTur__*inst->numDia__*inst->numPerDia__;

	
	// Primeiro período do dia
	RestJanHor *rest = &vetRestJanHor__[0];
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						rest->coefMatX[d*inst->numPerDia__][r][c] = 1;
						rest->coefMatX[(d*inst->numPerDia__) + 1][r][c] = -1;
					}
				}
			rest->coefMatZ[u][d][0] = -1;
		}
	}

	// Último período do dia
	rest = &vetRestJanHor__[1];
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						rest->coefMatX[(d*inst->numPerDia__) + inst->numPerDia__ - 1][r][c] = 1;
						rest->coefMatX[(d*inst->numPerDia__) + inst->numPerDia__ - 2][r][c] = -1;
					}
				}
			rest->coefMatZ[u][d][1] = -1;
		}
	}

	// Períodos intermediários do dia
	for (int s = 2; s < inst->numPerDia__; s++)
	{
		rest = &vetRestJanHor__[s];
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				for (int c = 0; c < inst->numDis__; c++)
					if (matDisTur__[c][u] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++) {
							rest->coefMatX[(d*inst->numPerDia__) + s - 1][r][c] = 1;
							rest->coefMatX[(d*inst->numPerDia__) + s - 2][r][c] = -1;
							rest->coefMatX[(d*inst->numPerDia__) + s][r][c] = -1;
						}
					}
				rest->coefMatZ[u][d][s] = -1;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloRelaxado(char *arq, Instancia* inst, double* vetAlpha) {

	FILE *f = fopen(arq, "w");

	int numRest = inst->numTur__*inst->numDia__*inst->numPerDia__;
	// ------------------ FO
	fprintf(f, "MIN\n");

	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int p = 0; p < inst->numPerTot__; p++)
		{
			for (int c = 0; c < inst->numDis__; c++) {
				
				double aux = 0;
				for (int i = 0; i < numRest; i++) {
					aux -= vetRestJanHor__[i].coefMatX[p][r][c] * vetAlpha[i];
				}
				double coef = (PESOS[0] * coefMatXFO[p][r][c]) + aux;
				
				if (coef < 0)
					fprintf(f, "%f x_%d_%d_%d ", coef, p, r, c);
				else
					fprintf(f, "+ %f x_%d_%d_%d ", coef, p, r, c);
			}
			fprintf(f, "\n");
		}
	}

	fprintf(f, "\n\n\\Janelas de horários\n");
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int s = 0; s < inst->numPerDia__; s++) {
				double aux = 0;
				for (int i = 0; i < numRest; i++) {
					aux -= vetRestJanHor__[i].coefMatZ[u][d][s] * vetAlpha[i];
				}
				double coef = PESOS[1] + aux;

				if (coef < 0)
					fprintf(f, "%f z_%d_%d_%d ", coef, u, d, s);
				else
					fprintf(f, "+ %f z_%d_%d_%d ", coef, u, d, s);
			}
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
int getVetViabJanHor(Solucao* sol, double* vetViabJanHor, Instancia* inst) {

	int viavel = 1;

	int pos = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			double soma = 0;
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						int prim = d*inst->numPerDia__;
						int seg = (d*inst->numPerDia__) + 1;
						soma += sol->vetSol_[offset3D(r, prim , c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
					}
				}
			vetViabJanHor[pos] = soma <= sol->vetSolZ_[pos] ? 0 : soma - sol->vetSolZ_[pos];
			if (vetViabJanHor[pos] != 0)
				viavel = 0;
			pos++;
		}
	}

	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			double soma = 0;
			for (int c = 0; c < inst->numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						int prim = (d*inst->numPerDia__) + inst->numPerDia__ - 1;
						int seg = (d*inst->numPerDia__) + inst->numPerDia__ - 2;
						soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)];
					}
				}
			vetViabJanHor[pos] = soma <= sol->vetSolZ_[pos] ? 0 : soma - sol->vetSolZ_[pos];
			if (vetViabJanHor[pos] != 0)
				viavel = 0;
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
					if (matDisTur__[c][u] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++) {
							int prim = (d*inst->numPerDia__) + s - 1;
							int seg = (d*inst->numPerDia__) + s - 2;
							int ter = (d*inst->numPerDia__) + s;
							soma += sol->vetSol_[offset3D(r, prim, c, inst->numPerTot__, inst->numDis__)] - sol->vetSol_[offset3D(r, seg, c, inst->numPerTot__, inst->numDis__)] -
								sol->vetSol_[offset3D(r, ter, c, inst->numPerTot__, inst->numDis__)];
						}
					}
				vetViabJanHor[pos] = soma <= sol->vetSolZ_[pos] ? 0 : soma - sol->vetSolZ_[pos];
				if (vetViabJanHor[pos] != 0)
					viavel = 0;
				pos++;
			}
		}
	}

	return viavel;
}
//------------------------------------------------------------------------------