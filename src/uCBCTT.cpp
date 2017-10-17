#include "uCBCTT.h"
#include "RelLagran.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include "..\lib\cplex\include\cplex.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))

#define RES_SOFT
#define RES_CAP_SAL // Restrição soft de capacidade das salas
#define	RES_JAN_HOR // Restrição soft de janelas de horários
#define RES_DIA_MIN // Restrição soft de dias mínimos
#define RES_SAL_DIF // Restrição soft de salas diferentes

//============================= VARIÁVEIS GLOBAIS ==============================

// ------------ Dados de entrada
char nomInst__[150]; // nome da instância
int numDis__;        // número de disciplinas 
int numTur__;        // número de turmas
int numPro__;        // número de professores
int numSal__;        // número de salas
int numDia__;        // número de dias
int numPerDia__;     // número de períodos por dia
int numPerTot__;     // número de períodos total
int numRes__;        // número de restrições
int numSol__;        // número de colunas
int numVar__;        // número de variáveis
Disciplina vetDisciplinas__[MAX_DIS];
Turma vetTurmas__[MAX_TUR];
Professor vetProfessores__[MAX_PRO];
Sala vetSalas__[MAX_SAL];
Restricao vetRestricoes__[MAX_RES];

// ------------ CPLEX


// ------------ Auxiliares
int matDisTur__[MAX_DIS][MAX_TUR]; // Dis x Cur; 1 se a disciplina d faz parte do currículo c; 0 caso contrário

RestJanHor *vetRestJanHor__; // Vetor com as restrições de janela horário
int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]; // Matriz de coeficientes das variáveis x da FO.
double *vetAlpha;	// Vetor com os multiplicadores de Lagrange

//char INST[50] = "comp";
char INST[50] = "toy";
int PESOS[4] = { 1,2,5,1 };

//==============================================================================


//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char nomeInst[10];
	strcpy_s(nomeInst, INST);
	strcat_s(nomeInst, "3");

	execUma(nomeInst);
	//execTodas();

	initVetJanHor();
	montaMatCoefXFO();
	montaCoefRestJanHor();

	FILE* f = fopen("teste.txt", "w");

	fclose(f);

	printf("\n\n>>> Pressione ENTER para encerrar: ");
	_getch();
	return 0;
}
//------------------------------------------------------------------------------

void execUma(char* nomeInst) {
	char aux[150];
	Solucao sol;

	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
	strcat_s(aux, ".ctt");
	printf("%s", aux);
	lerInstancia(aux);
	//lerSolucoesIniciais();

	//testarEntrada();
	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
	strcat_s(aux, ".lp");
	montarModeloPLI(aux);
	execCpx(sol, aux);
	strcpy_s(aux, PATH_INST);
	strcat_s(aux, nomeInst);
#ifndef RES_SOFT
	strcat_s(aux, "-H");
#endif 
	strcat_s(aux, ".sol");
	escreverSol(sol, aux);
}

void execTodas() {
	for (int i = 1; i <= NUM_INST; i++) {

		char aux[150];
		char nomeInst[20];
		char temp[10];
		Solucao sol;

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
		lerInstancia(aux);
		//lerSolucoesIniciais();

		//testarEntrada();
		strcpy_s(aux, PATH_INST);
		strcat_s(aux, nomeInst);
#ifndef RES_SOFT
		strcat_s(aux, "-H");
#endif 
		strcat_s(aux, ".lp");
		montarModeloPLI(aux);
		execCpx(sol, aux);
		strcpy_s(aux, PATH_INST);
		strcat_s(aux, nomeInst);
#ifndef RES_SOFT
		strcat_s(aux, "-H");
#endif 
		strcat_s(aux, ".sol");
		escreverSol(sol, aux);

	}
}

//------------------------------------------------------------------------------
void execCpx(Solucao &s, char *arq)
{
	int sts;
	clock_t h;
	CPXENVptr env;
	CPXLPptr lp;
	env = CPXopenCPLEX(&sts);
	sts = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
	lp = CPXcreateprob(env, &sts, "");
	sts = CPXreadcopyprob(env, lp, arq, NULL);
	s.numVar_ = CPXgetnumcols(env, lp);
	s.numRes_ = CPXgetnumrows(env, lp);
	h = clock();
	sts = CPXmipopt(env, lp);
	h = clock() - h;
	s.tempo_ = (double)h / CLOCKS_PER_SEC;
	sts = CPXgetmipobjval(env, lp, &s.valSol_);
	sts = CPXgetbestobjval(env, lp, &s.bstNod_);
	sts = CPXgetmipx(env, lp, s.vetSol_, 0, (numPerTot__*numSal__*numDis__ - 1));
	sts = CPXfreeprob(env, &lp);
	sts = CPXcloseCPLEX(&env);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void escreverSol(Solucao &s, char *arq)
{
	int pos, dia, per, aux;
	FILE *f = fopen(arq, "w");
	fprintf(f, ">>> RESULTADOS DO CPLEX\n\n");
	fprintf(f, "Num. var....: %d\n", s.numVar_);
	fprintf(f, "Num. res....: %d\n", s.numRes_);
	fprintf(f, "Val. sol....: %.2f\n", s.valSol_);
	fprintf(f, "Melhor no...: %.2f\n", s.bstNod_);
	if (s.valSol_ != 0)
		fprintf(f, "GAP.........: %.2f\n", ((s.valSol_ - s.bstNod_) / s.valSol_) * 100);
	else
		fprintf(f, "GAP.........: -\n");
	fprintf(f, "Tempo.......: %.2f\n", s.tempo_);
	//------------------------------------------------------------- 
	// preencher a solucao
	fprintf(f, "\n\n>>> SOLUCAO (<periodo><sala><disciplina>)\n\n", s.tempo_);
	for (int p = 0; p < numPerDia__; p++)
		for (int d = 0; d < numDia__; d++)
			for (int r = 0; r < numSal__; r++)
				s.matSolSal_[p][d][r] = -1;
	for (int p = 0; p < numPerDia__; p++)
		for (int d = 0; d < numDia__; d++)
			for (int u = 0; u < numTur__; u++)
				s.matSolTur_[p][d][u] = -1;
	pos = 0;
	for (int r = 0; r < numSal__; r++)
		for (int p = 0; p < numPerTot__; p++)
			for (int c = 0; c < numDis__; c++)
			{
				if (s.vetSol_[pos] > 0)
				{
					dia = p / numPerDia__;
					per = p % numPerDia__;
					fprintf(f, "x_%d_%d_%d = %.2f\n", p, r, c, s.vetSol_[pos]);
					if (s.matSolSal_[per][dia][r] != -1)
					{
						printf("\n\nERRO - SALA %d- p%d d%d c%d!\n\n", r, per, dia, c);
						_getch();
					}
					else
						s.matSolSal_[per][dia][r] = c;
					for (int u = 0; u < numTur__; u++)
					{
						if (matDisTur__[c][u] == 1)
						{
							if (s.matSolTur_[per][dia][u] != -1)
							{
								printf("\n\nERRO - TURMA %d - p%d d%d c%d!\n\n", u, per, dia, c);
								_getch();
							}
							else
								s.matSolTur_[per][dia][u] = c;
						}
					}
				}
				pos++;
			}
	fprintf(f, "\n");
	for (int r = 0; r < numSal__; r++)
	{
		fprintf(f, "SALA %d\n", r);
		for (int p = 0; p < numPerDia__; p++)
		{
			for (int d = 0; d < numDia__; d++)
				fprintf(f, "%d  ", s.matSolSal_[p][d][r]);
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");
	for (int u = 0; u < numTur__; u++)
	{
		fprintf(f, "TURMA %d\n", u);
		for (int p = 0; p < numPerDia__; p++)
		{
			for (int d = 0; d < numDia__; d++)
				fprintf(f, "%d  ", s.matSolTur_[p][d][u]);
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	//------------------------------------------------------------- 
	// verificar a solucao
	// HARD

	s.vioNumAul_ = 0;
	for (int c = 0; c < numDis__; c++)
	{
		aux = 0;
		for (int r = 0; r < numSal__; r++)
		{
			for (int p = 0; p < numPerDia__; p++)
				for (int d = 0; d < numDia__; d++)
					if (s.matSolSal_[p][d][r] == c)
						aux++;
		}
		s.vioNumAul_ += abs(vetDisciplinas__[c].numPer_ - aux);
	}

	s.vioAulSim_ = 0;
	s.vioDisSim_ = 0;
	s.vioProSim_ = 0;
	s.vioTurSim_ = 0;
	for (int r = 0; r < numSal__; r++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int p = 0; p < numPerDia__; p++)
			{
				if (s.matSolSal_[p][d][r] != -1)
					for (int r2 = r + 1; r2 < numSal__; r2++)
					{
						if (s.matSolSal_[p][d][r2] != -1)
						{
							if (s.matSolSal_[p][d][r2] == s.matSolSal_[p][d][r])
								s.vioDisSim_++;
							if (vetDisciplinas__[s.matSolSal_[p][d][r2]].professor_ == vetDisciplinas__[s.matSolSal_[p][d][r]].professor_)
								s.vioProSim_++;
							for (int u = 0; u < numTur__; u++)
								if ((matDisTur__[s.matSolSal_[p][d][r2]][u] == 1) && (matDisTur__[s.matSolSal_[p][d][r]][u] == 1))
									s.vioTurSim_++;
						}
					}
			}
		}
	}
	// SOFT
	s.capSal_ = 0;
	for (int r = 0; r < numSal__; r++)
		for (int p = 0; p < numPerDia__; p++)
			for (int d = 0; d < numDia__; d++)
			{
				if (s.matSolSal_[p][d][r] != -1)
					if (vetDisciplinas__[s.matSolSal_[p][d][r]].numAlu_ > vetSalas__[r].capacidade_)
						s.capSal_++;
			}
	s.janHor_ = 0;
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			if ((s.matSolTur_[0][d][u] != -1) && (s.matSolTur_[1][d][u] == -1))
				s.janHor_++;
			if ((s.matSolTur_[numPerDia__ - 1][d][u] != -1) && (s.matSolTur_[numPerDia__ - 2][d][u] == -1))
				s.janHor_++;
			for (int p = 2; p < numPerDia__; p++)
				if ((s.matSolTur_[p - 1][d][u] != -1) && (s.matSolTur_[p - 2][d][u] == -1) && (s.matSolTur_[p][d][u] == -1))
					s.janHor_++;
		}
	}
	s.diaMin_ = 0;
	for (int c = 0; c < numDis__; c++)
	{
		aux = 0;
		for (int d = 0; d < numDia__; d++)
		{
			pos = 0;
			for (int p = 0; p < numPerDia__; p++)
			{
				for (int r = 0; r < numSal__; r++)
				{
					if (s.matSolSal_[p][d][r] == c)
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
		if (aux < vetDisciplinas__[c].diaMin_)
			s.diaMin_++;
	}
	s.salDif_ = 0;
	for (int c = 0; c < numDis__; c++)
	{
		aux = 0;
		for (int r = 0; r < numSal__; r++)
		{
			pos = 0;
			for (int d = 0; d < numDia__; d++)
			{
				for (int p = 0; p < numPerDia__; p++)
				{
					if (s.matSolSal_[p][d][r] == c)
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
		s.salDif_ += aux - 1;
	}
	s.funObj_ = PESOS[0] * s.capSal_ + PESOS[1] * s.janHor_ + PESOS[2] * s.diaMin_ + PESOS[3] * s.salDif_;
	fprintf(f, "\n\n>>> RESULTADOS CALCULADOS\n\n", s.valSol_);
	fprintf(f, "Func. obj.....: %d\n", s.funObj_);
	fprintf(f, "\n\nHARD----------------------------------\n\n");
	fprintf(f, "Num. aulas....: %d\n", s.vioNumAul_);
	fprintf(f, "Aulas simul...: %d\n", s.vioAulSim_);
	fprintf(f, "Disc. simul...: %d\n", s.vioDisSim_);
	fprintf(f, "Prof. simul...: %d\n", s.vioProSim_);
	fprintf(f, "Turm. simul...: %d\n", s.vioTurSim_);
	fprintf(f, "\n\nSOFT------------------------------------\n\n");
	fprintf(f, "Cap. salas....: %d\n", s.capSal_);
	fprintf(f, "Jan. turmas...: %d\n", s.janHor_);
	fprintf(f, "Dias minimo...: %d\n", s.diaMin_);
	fprintf(f, "Salas difer...: %d\n", s.salDif_);
	fclose(f);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloPLI(char *arq)
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
	for (int r = 0; r < numSal__; r++)
	{
		for (int p = 0; p < numPerTot__; p++)
		{
			for (int c = 0; c < numDis__; c++)
				if (vetDisciplinas__[c].numAlu_ > vetSalas__[r].capacidade_)
					fprintf(f, "+ %d x_%d_%d_%d ", PESOS[0] * (vetDisciplinas__[c].numAlu_ - vetSalas__[r].capacidade_), p, r, c);
				else
					fprintf(f, "+ 0 x_%d_%d_%d ", p, r, c);
			fprintf(f, "\n");
		}
	}
#endif

#ifdef RES_JAN_HOR
	fprintf(f, "\n\n\\Janelas de horários\n");
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int s = 0; s < numPerDia__; s++)
				fprintf(f, "+ %d z_%d_%d_%d ", PESOS[1], u, d, s);
			fprintf(f, "\n");
		}
	}
#endif

#ifdef RES_DIA_MIN
	fprintf(f, "\n\\Dias mínimos\n");
	for (int c = 0; c < numDis__; c++)
		fprintf(f, "+ %d q_%d ", PESOS[2], c);
#endif

#ifdef RES_SAL_DIF
	fprintf(f, "\n\n\\Salas diferentes\n");
	for (int c = 0; c < numDis__; c++)
	{
		for (int r = 0; r < numSal__; r++)
			fprintf(f, "+ %d y_%d_%d ", PESOS[3], r, c);
		fprintf(f, "\n");
	}
#endif

	val = PESOS[3] * numDis__;
	fprintf(f, "- val\n");
	fprintf(f, "\nST\n");
	fprintf(f, "\n\\Valor constante\n");
	fprintf(f, "val = %d\n", val);
#endif
	fprintf(f, "\n\\ ------------------------------------ HARD------------------------------------\n");
	fprintf(f, "\n\\R1 - Número de aulas\n");
	for (int c = 0; c < numDis__; c++)
	{
		for (int p = 0; p < numPerTot__; p++)
			for (int r = 0; r < numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
		fprintf(f, "= %d\n", vetDisciplinas__[c].numPer_);
	}
	fprintf(f, "\n\\R2 - Aulas na mesma sala no mesmo período\n");
	for (int p = 0; p < numPerTot__; p++)
		for (int r = 0; r < numSal__; r++)
		{
			for (int c = 0; c < numDis__; c++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R3 - Aulas de uma disciplina no mesmo período\n");
	for (int p = 0; p < numPerTot__; p++)
		for (int c = 0; c < numDis__; c++)
		{
			for (int r = 0; r < numSal__; r++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R4 - Aulas de um professor no mesmo período\n");
	for (int p = 0; p < numPerTot__; p++)
		for (int t = 0; t < numPro__; t++)
		{
			for (int r = 0; r < numSal__; r++)
				for (int c = 0; c < numDis__; c++)
					if (vetDisciplinas__[c].professor_ == t)
						fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R5 - Aulas de uma turma no mesmo período\n");
	for (int p = 0; p < numPerTot__; p++)
		for (int u = 0; u < numTur__; u++)
		{
			for (int r = 0; r < numSal__; r++)
				for (int c = 0; c < numDis__; c++)
				{
					for (int k = 0; k < vetTurmas__[u].numDis_; k++)
						if (vetTurmas__[u].vetDis_[k] == c)
						{
							fprintf(f, "+ x_%d_%d_%d ", p, r, c);
							break;
						}
				}
			fprintf(f, "<= 1\n");
		}
	fprintf(f, "\n\\R6 - Restrições de oferta (alocação)\n");
	for (int s = 0; s < numRes__; s++)
	{
		for (int r = 0; r < numSal__; r++)
			fprintf(f, "+ x_%d_%d_%d ", vetRestricoes__[s].periodo_, r, vetRestricoes__[s].disciplina_);
		fprintf(f, "= 0\n");
	}
#ifdef RES_SOFT
	fprintf(f, "\n\\ ------------------------------------ SOFT------------------------------------\n");
	fprintf(f, "\n\\R7 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < numDis__; c++)
	{
		for (int d = 0; d < numDia__; d++)
			for (int p = (d*numPerDia__); p < (d*numPerDia__) + numPerDia__; p++)
			{
				for (int r = 0; r < numSal__; r++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
				fprintf(f, "- v_%d_%d <= 0\n", d, c);
			}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R8 - Número de salas usadas por disciplina\n");
	for (int c = 0; c < numDis__; c++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int r = 0; r < numSal__; r++)
				for (int p = (d*numPerDia__); p < (d*numPerDia__) + numPerDia__; p++)
					fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- v_%d_%d >= 0\n", d, c);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R9 - Dias mínimos\n");
	for (int c = 0; c < numDis__; c++)
	{
		for (int d = 0; d < numDia__; d++)
			fprintf(f, "+ v_%d_%d ", d, c);
		fprintf(f, "+ q_%d >= %d\n", c, vetDisciplinas__[c].diaMin_);
	}
	fprintf(f, "\n\\R10 a R13+#PER_DIA - Janelas no horário das turmas\n");
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int c = 0; c < numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*numPerDia__), r, c, (d*numPerDia__) + 1, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 0);
		}
	}
	fprintf(f, "\n");
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int c = 0; c < numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < numSal__; r++)
						fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d ", (d*numPerDia__) + numPerDia__ - 1, r, c, (d*numPerDia__) + numPerDia__ - 2, r, c);
				}
			fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, 1);
		}
	}
	fprintf(f, "\n");
	for (int s = 2; s < numPerDia__; s++)
	{
		for (int u = 0; u < numTur__; u++)
		{
			for (int d = 0; d < numDia__; d++)
			{
				for (int c = 0; c < numDis__; c++)
					if (matDisTur__[c][u] == 1)
					{
						for (int r = 0; r < numSal__; r++)
							fprintf(f, "+ x_%d_%d_%d - x_%d_%d_%d - x_%d_%d_%d ", (d*numPerDia__) + s - 1, r, c, (d*numPerDia__) + s - 2, r, c, (d*numPerDia__) + s, r, c);
					}
				fprintf(f, "- z_%d_%d_%d <= 0\n", u, d, s);
			}
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R14 - Salas utilizadas por disciplina\n");
	for (int p = 0; p < numPerTot__; p++)
	{
		for (int r = 0; r < numSal__; r++)
			for (int c = 0; c < numDis__; c++)
				fprintf(f, "x_%d_%d_%d - y_%d_%d <= 0\n", p, r, c, r, c);
		fprintf(f, "\n");
	}
	fprintf(f, "\n\\R15 - Salas utilizadas por disciplina\n");
	for (int r = 0; r < numSal__; r++)
	{
		for (int c = 0; c < numDis__; c++)
		{
			for (int p = 0; p < numPerTot__; p++)
				fprintf(f, "+ x_%d_%d_%d ", p, r, c);
			fprintf(f, "- y_%d_%d >= 0\n", r, c);
		}
		fprintf(f, "\n");
	}
#endif
	fprintf(f, "\nBOUNDS\n");
	fprintf(f, "\n\\Variáveis x\n");
	for (int r = 0; r < numSal__; r++)
		for (int p = 0; p < numPerTot__; p++)
			for (int c = 0; c < numDis__; c++)
				fprintf(f, "0 <= x_%d_%d_%d <= 1\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < numDia__; d++)
		for (int c = 0; c < numDis__; c++)
			fprintf(f, "0 <= v_%d_%d <= 1\n", d, c);
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < numDis__; c++)
		fprintf(f, "0 <= q_%d <= %d\n", c, numDia__);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < numTur__; u++)
		for (int d = 0; d < numDia__; d++)
			for (int s = 0; s < numPerDia__; s++)
				fprintf(f, "0 <= z_%d_%d_%d <= 1\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < numSal__; r++)
		for (int c = 0; c < numDis__; c++)
			fprintf(f, "0 <= y_%d_%d <= 1\n", r, c);
#endif
#ifdef RES_SOFT
	fprintf(f, "\nGENERALS\n"); // Para variáveis inteiras
	fprintf(f, "\n\\Variáveis q\n");
	for (int c = 0; c < numDis__; c++)
		fprintf(f, "q_%d\n", c);
#endif
	fprintf(f, "\nBINARIES\n");
	for (int r = 0; r < numSal__; r++)
		for (int p = 0; p < numPerTot__; p++)
			for (int c = 0; c < numDis__; c++)
				fprintf(f, "x_%d_%d_%d\n", p, r, c);
#ifdef RES_SOFT
	fprintf(f, "\n\\Variáveis v\n");
	for (int d = 0; d < numDia__; d++)
		for (int c = 0; c < numDis__; c++)
			fprintf(f, "v_%d_%d\n", d, c);
	fprintf(f, "\n\\Variáveis z\n");
	for (int u = 0; u < numTur__; u++)
		for (int d = 0; d < numDia__; d++)
			for (int s = 0; s < numPerDia__; s++)
				fprintf(f, "z_%d_%d_%d\n", u, d, s);
	fprintf(f, "\n\\Variáveis y\n");
	for (int r = 0; r < numSal__; r++)
		for (int c = 0; c < numDis__; c++)
			fprintf(f, "y_%d_%d\n", r, c);
#endif
	fprintf(f, "\nEND");
	fclose(f);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void lerInstancia(char *arq)
{
	int pos;
	char aux[50];
	FILE *f = fopen(arq, "r");
	fscanf(f, "Name: %s\n", &nomInst__);
	fscanf(f, "Courses: %d\n", &numDis__);
	fscanf(f, "Rooms: %d\n", &numSal__);
	fscanf(f, "Days: %d\n", &numDia__);
	fscanf(f, "Periods_per_day: %d\n", &numPerDia__);
	fscanf(f, "Curricula: %d\n", &numTur__);
	fscanf(f, "Constraints: %d\n", &numRes__);
	fscanf(f, "\nCOURSES:\n");
	numPerTot__ = numDia__ * numPerDia__;
	numPro__ = 0;
	for (int i = 0; i < numDis__; i++)
	{
		fscanf(f, "%s %s %d %d %d\n", &vetDisciplinas__[i].nome_, &aux,
			&vetDisciplinas__[i].numPer_, &vetDisciplinas__[i].diaMin_, &vetDisciplinas__[i].numAlu_);
		pos = -1;
		for (int p = 0; p < numPro__; p++)
			if (strcmp(aux, vetProfessores__[p].nome_) == 0)
			{
				vetDisciplinas__[i].professor_ = p;
				pos = p;
				break;
			}
		if (pos == -1)
		{
			vetDisciplinas__[i].professor_ = numPro__;
			strcpy_s(vetProfessores__[numPro__].nome_, aux);
			numPro__++;
		}
	}
	for (int i = 0; i < numDis__; i++)
		for (int j = 0; j < numTur__; j++)
			matDisTur__[i][j] = 0;
	fscanf(f, "\nROOMS:\n");
	for (int i = 0; i < numSal__; i++)
		fscanf(f, "%s %d\n", &vetSalas__[i].nome_, &vetSalas__[i].capacidade_);
	fscanf(f, "\nCURRICULA:\n");
	for (int i = 0; i < numTur__; i++)
	{
		fscanf(f, "%s %d", &vetTurmas__[i].nome_, &vetTurmas__[i].numDis_);
		for (int j = 0; j < vetTurmas__[i].numDis_; j++)
		{
			fscanf(f, "%s ", &aux);
			vetTurmas__[i].vetDis_[j] = -1;
			for (int k = 0; k < numDis__; k++)
				if (strcmp(aux, vetDisciplinas__[k].nome_) == 0)
				{
					vetTurmas__[i].vetDis_[j] = k;
					matDisTur__[k][i] = 1;
					break;
				}
		}
	}
	fscanf(f, "\nUNAVAILABILITY_CONSTRAINTS:\n");
	for (int i = 0; i < numRes__; i++)
	{
		fscanf(f, "%s %d %d\n", &aux, &pos, &vetRestricoes__[i].periodo_);
		vetRestricoes__[i].periodo_ = (pos * numPerDia__) + vetRestricoes__[i].periodo_;
		vetRestricoes__[i].disciplina_ = -1;
		for (int j = 0; j < numDis__; j++)
			if (strcmp(aux, vetDisciplinas__[j].nome_) == 0)
			{
				vetRestricoes__[i].disciplina_ = j;
				break;
			}
	}
	fclose(f);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void testarEntrada()
{
	printf("Name: %s\n", nomInst__);
	printf("Courses: %d\n", numDis__);
	printf("Rooms: %d\n", numSal__);
	printf("Days: %d\n", numDia__);
	printf("Periods_per_day: %d\n", numPerDia__);
	printf("Curricula: %d\n", numTur__);
	printf("Constraints: %d\n", numRes__);

	printf("\nDISCIPLINAS:\n");
	for (int i = 0; i < numDis__; i++)
		printf("%d  %s  %d  %d  %d  %d\n", i, vetDisciplinas__[i].nome_, vetDisciplinas__[i].professor_, vetDisciplinas__[i].numPer_, vetDisciplinas__[i].diaMin_, vetDisciplinas__[i].numAlu_);

	printf("\nTURMAS:\n");
	for (int i = 0; i < numTur__; i++)
	{
		printf("%d  %s  %d --> ", i, vetTurmas__[i].nome_, vetTurmas__[i].numDis_);
		for (int j = 0; j < vetTurmas__[i].numDis_; j++)
			printf("%d  ", vetTurmas__[i].vetDis_[j]);
		printf("\n");
	}

	printf("\nPROFESSORES:\n");
	for (int i = 0; i < numPro__; i++)
		printf("%d  %s\n", i, vetProfessores__[i].nome_);

	printf("\nSALAS:\n");
	for (int i = 0; i < numSal__; i++)
		printf("%d  %s  %d\n", i, vetSalas__[i].nome_, vetSalas__[i].capacidade_);

	printf("\nRESTRICOES:\n");
	for (int i = 0; i < numRes__; i++)
		printf("%d  %d  %d\n", i, vetRestricoes__[i].disciplina_, vetRestricoes__[i].periodo_);

	printf("\nDISC x TURMA:\n");
	for (int i = 0; i < numDis__; i++)
	{
		for (int j = 0; j < numTur__; j++)
			printf("%d  ", matDisTur__[i][j]);
		printf("\n");
	}
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
void initVetJanHor() {

	int numRest = numTur__*numDia__*numPerDia__;
	vetRestJanHor__ = (RestJanHor*)malloc(numRest * sizeof(RestJanHor));

	for (int i = 0; i < numRest; i++) {
		initRestJanHor(&vetRestJanHor__[i]);
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaMatCoefXFO() {

	for (int p = 0; p < MAX_PER * MAX_DIA; p++) {
		for (int r = 0; r < MAX_SAL; r++) {
			for (int c = 0; c < MAX_DIS; c++) {
				if (vetDisciplinas__[c].numAlu_ > vetSalas__[r].capacidade_)
					coefMatXFO[p][r][c] = (vetDisciplinas__[c].numAlu_ - vetSalas__[r].capacidade_);
				else
					coefMatXFO[p][r][c] = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaCoefRestJanHor() {

	int numRest = numTur__*numDia__*numPerDia__;

	RestJanHor *rest = &vetRestJanHor__[0];

	// Primeiro período do dia
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int c = 0; c < numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < numSal__; r++) {
						rest->coefMatX[d*numPerDia__][r][c] = 1;
						rest->coefMatX[(d*numPerDia__) + 1][r][c] = -1;
					}
				}
			rest->coefMatZ[u][d][0] = -1;
		}
	}

	rest = &vetRestJanHor__[1];
	// Último período do dia
	for (int u = 0; u < numTur__; u++)
	{
		for (int d = 0; d < numDia__; d++)
		{
			for (int c = 0; c < numDis__; c++)
				if (matDisTur__[c][u] == 1)
				{
					for (int r = 0; r < numSal__; r++) {
						rest->coefMatX[(d*numPerDia__) + numPerDia__ - 1][r][c] = 1;
						rest->coefMatX[(d*numPerDia__) + numPerDia__ - 2][r][c] = -1;
					}
				}
			rest->coefMatZ[u][d][1] = -1;
		}
	}

	// Períodos intermediários do dia
	for (int s = 2; s < numPerDia__; s++)
	{
		rest = &vetRestJanHor__[s];
		for (int u = 0; u < numTur__; u++)
		{
			for (int d = 0; d < numDia__; d++)
			{
				for (int c = 0; c < numDis__; c++)
					if (matDisTur__[c][u] == 1)
					{
						for (int r = 0; r < numSal__; r++) {
							rest->coefMatX[(d*numPerDia__) + s - 1][r][c] = 1;
							rest->coefMatX[(d*numPerDia__) + s - 2][r][c] = -1;
							rest->coefMatX[(d*numPerDia__) + s][r][c] = -1;
						}
					}
				rest->coefMatZ[u][d][s] = -1;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montarModeloRelaxado(char *arq) {

	FILE *f = fopen(arq, "w");

	int numRest = numTur__*numDia__*numPerDia__;
	// ------------------ FO
	fprintf(f, "MIN\n");

	for (int r = 0; r < numSal__; r++)
	{
		for (int p = 0; p < numPerTot__; p++)
		{
			for (int c = 0; c < numDis__; c++) {
				
				double aux = 0;
				for (int i = 0; i < numRest; i++) {
					aux -= vetRestJanHor__[0].coefMatX[p][r][c] * vetAlpha[i];
				}

				fprintf(f, "+ %d x_%d_%d_%d ", (PESOS[0] * coefMatXFO[p][r][c]) - aux, p, r, c);
			}
			fprintf(f, "\n");
		}
	}

}
//------------------------------------------------------------------------------
