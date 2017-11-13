
#include "Solucao.h"
#include "Colecoes.h"

#include<stdlib.h>
#include <conio.h>
#include<stdio.h>
#include<string.h>

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
	pos = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
			{
				if (s->vetSol_[pos] > 0)
				{
					fprintf(f, "x_%d_%d_%d = %f\n", p, r, c, s->vetSol_[pos]);
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
	
	fprintf(f, "\n\n>>> RESULTADOS CALCULADOS\n\n", s->valSol_);
	fprintf(f, "Func. obj.....: %f\n", s->funObj_);
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
void initMats(Solucao *s, Instancia* inst) {

	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int r = 0; r < inst->numSal__; r++)
				s->matSolSal_[p][d][r] = -1;
	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int u = 0; u < inst->numTur__; u++)
				s->matSolTur_[p][d][u] = -1;
	int pos = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
			{
				if (s->vetSol_[pos] > 0)
				{
					int dia = p / inst->numPerDia__;
					int per = p % inst->numPerDia__;
					if (s->matSolSal_[per][dia][r] == -1)
					{
						s->matSolSal_[per][dia][r] = c;
					}

					for (int u = 0; u < inst->numTur__; u++) {
						if (inst->matDisTur__[c][u] == 1) {
							if (s->matSolTur_[per][dia][u] == -1) {
								s->matSolTur_[per][dia][u] = c;
							}
						}
					}
				}
				pos++;
			}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void contaViolacoes(Solucao *s, Instancia* inst) {

	// Calculando as violacoes
	// HARD
	int aux;
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
								if ((inst->matDisTur__[s->matSolSal_[p][d][r2]][u] == 1) && (inst->matDisTur__[s->matSolSal_[p][d][r]][u] == 1))
									s->vioTurSim_++;
						}
					}
			}
		}
	}

	// SOFT
	int pos;
	s->capSal_ = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerDia__; p++)
			for (int d = 0; d < inst->numDia__; d++)
			{
				if (s->matSolSal_[p][d][r] != -1)
					if (inst->vetDisciplinas__[s->matSolSal_[p][d][r]].numAlu_ > inst->vetSalas__[r].capacidade_)
						s->capSal_ += inst->vetDisciplinas__[s->matSolSal_[p][d][r]].numAlu_ - inst->vetSalas__[r].capacidade_;
			}

	s->janHor_ = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			if ((s->matSolTur_[0][d][u] != -1) && (s->matSolTur_[1][d][u] == -1)) {
				s->janHor_++;
			}
			if ((s->matSolTur_[inst->numPerDia__ - 1][d][u] != -1) && (s->matSolTur_[inst->numPerDia__ - 2][d][u] == -1)) {
				s->janHor_++;
			}
			for (int p = 2; p < inst->numPerDia__; p++)
				if ((s->matSolTur_[p - 1][d][u] != -1) && (s->matSolTur_[p - 2][d][u] == -1) && (s->matSolTur_[p][d][u] == -1)) {
					s->janHor_++;
				}
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
			s->diaMin_ += inst->vetDisciplinas__[c].diaMin_ - aux;
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
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaSolucao(Solucao *s, Instancia* inst) {

	initMats(s, inst);
	contaViolacoes(s, inst);
	calculaFO(s, inst);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void viabilizaSol(Solucao *s, Instancia* inst) {
	
	// Variáveis Z
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			if ((s->matSolTur_[0][d][u] != -1) && (s->matSolTur_[1][d][u] == -1)) {
				s->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)] = 1;
				//printf("violacao z\n");
			}
			else
				s->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)] = 0;
			

			if ((s->matSolTur_[inst->numPerDia__ - 1][d][u] != -1) && (s->matSolTur_[inst->numPerDia__ - 2][d][u] == -1)) {
				s->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)] = 1;
				//printf("violacao z\n");
			}
			else 
				s->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)] = 0;
			

			for (int p = 2; p < inst->numPerDia__; p++)
				if ((s->matSolTur_[p - 1][d][u] != -1) && (s->matSolTur_[p - 2][d][u] == -1) && (s->matSolTur_[p][d][u] == -1)) {
					s->vetSolZ_[offset3D(u, d, p, inst->numDia__, inst->numPerDia__)] = 1;
					//printf("violacao z\n");
				}
				else
					s->vetSolZ_[offset3D(u, d, p, inst->numDia__, inst->numPerDia__)] = 0;
		}
	}

	// Variáveis Y
	int aux, pos;
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
				if (pos == 1) {
					break;
				}	
			}
			if (pos == 1) {
				s->vetSolY_[offset2D(c, r, inst->numSal__)] = 1;
			}
			else
				s->vetSolY_[offset2D(c, r, inst->numSal__)] = 0;
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void imprimeZ(Solucao *sol, Instancia* inst) {

	int pos = 0;
	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				printf("z_%d_%d_%d = %f\n", u, d, s, sol->vetSolZ_[pos]);
				pos++;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void imprimeY(Solucao *sol, Instancia* inst) {

	int pos = 0;
	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			printf("y_%d_%d = %f\n", r, c, sol->vetSolY_[pos]);
			pos++;
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void calculaFO(Solucao *sol, Instancia* inst) {

	double somaX, somaZ, somaQ, somaY, val;
	somaX = somaZ = somaQ = somaY = 0;
	val = inst->numDis__ * PESOS[3];

	int numX = inst->numPerTot__*inst->numSal__*inst->numDis__;
	int numZ = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__*inst->numDis__;
	

	for (int i = 0; i < numX; i++) {
		somaX += inst->vetCoefX[i] * sol->vetSol_[i];
	}

	for (int i = 0; i < numZ; i++) {
		somaZ += inst->vetCoefZ[i] * sol->vetSolZ_[i];
	}

	for (int i = 0; i < numQ; i++) {
		somaQ += inst->vetCoefQ[i] * sol->vetSolQ_[i];
	}

	for (int i = 0; i < numY; i++) {
		somaY += inst->vetCoefY[i] * sol->vetSolY_[i];
	}

	sol->funObj_ = somaX + somaZ + somaQ + somaY - val;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Solucao* clonarSolucao(Solucao *sol, Instancia* inst) {

	Solucao *clone = (Solucao*) malloc(sizeof(Solucao));

	clone->capSal_ = sol->capSal_;
	clone->janHor_ = sol->janHor_;
	clone->diaMin_ = sol->diaMin_;
	clone->salDif_ = sol->salDif_;

	clone->funObj_ = sol->funObj_;

	clone->vioNumAul_ = sol->vioNumAul_;
	clone->vioAulSim_ = sol->vioAulSim_;
	clone->vioDisSim_ = sol->vioDisSim_;
	clone->vioProSim_ = sol->vioProSim_;
	clone->vioTurSim_ = sol->vioTurSim_;

	clone->numVar_ = sol->numVar_;
	clone->numRes_ = sol->numRes_;

	//clone->vetViab_ = (double*) malloc(sol->numRes_ * sizeof(double));
	//memcpy(clone->vetViab_, sol->vetViab_, sol->numRes_ * sizeof(double));

	clone->valSol_ = sol->valSol_;
	clone->bstNod_ = sol->bstNod_;
	clone->tempo_ = sol->tempo_;

	/*int numRestJanHor = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	clone->vetViabJanHor_ = (double*) malloc(numRestJanHor * sizeof(double));
	clone->vetViab14_ = (double*)malloc(numRest14 * sizeof(double));
	clone->vetViab15_ = (double*)malloc(numRest15 * sizeof(double));
	memcpy(clone->vetViabJanHor_, sol->vetViabJanHor_, numRestJanHor * sizeof(double));
	memcpy(clone->vetViab14_, sol->vetViab14_, numRest14 * sizeof(double));
	memcpy(clone->vetViab15_, sol->vetViab15_, numRest15 * sizeof(double));*/

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;
	int numV = inst->numDia__ * inst->numDis__;
	clone->vetSol_ = (double*)malloc(numX * sizeof(double));
	clone->vetSolZ_ = (double*)malloc(numZ * sizeof(double));
	clone->vetSolQ_ = (double*)malloc(numQ * sizeof(double));
	clone->vetSolY_ = (double*)malloc(numY * sizeof(double));
	clone->vetSolV_ = (double*)malloc(numV * sizeof(double));
	memcpy(clone->vetSol_, sol->vetSol_, numX * sizeof(double));
	memcpy(clone->vetSolZ_, sol->vetSolZ_, numZ * sizeof(double));
	memcpy(clone->vetSolQ_, sol->vetSolQ_, numQ * sizeof(double));
	memcpy(clone->vetSolY_, sol->vetSolY_, numY * sizeof(double));
	memcpy(clone->vetSolV_, sol->vetSolV_, numV * sizeof(double));

	int tamSolSal = MAX_PER * MAX_DIA * MAX_SAL;
	memcpy(clone->matSolSal_, sol->matSolSal_, tamSolSal * sizeof(int));

	int tamSolTur = MAX_PER * MAX_DIA * MAX_TUR;
	memcpy(clone->matSolTur_, sol->matSolTur_, tamSolTur * sizeof(int));

	return clone;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void desalocaSolucao(Solucao *s) {

	free(s->vetSol_);
	free(s->vetSolZ_);
	free(s->vetSolQ_);
	free(s->vetSolY_);
	free(s->vetSolV_);
	free(s);
}
//------------------------------------------------------------------------------
