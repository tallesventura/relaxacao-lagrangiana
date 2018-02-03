
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
				fprintf(f, "%d  ", s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)]);
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
				fprintf(f, "%d  ", s->matSolTur_[offset3D(p, d, u, inst->numDia__, inst->numTur__)]);
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
	
	s->matSolSal_ = (int*)malloc(inst->numPerDia__ * inst->numDia__ * inst->numSal__ * sizeof(int));
	s->matSolTur_ = (int*) malloc(inst->numPerDia__ * inst->numDia__ * inst->numTur__ * sizeof(int));

	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int r = 0; r < inst->numSal__; r++)
				s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] = -1;
	for (int p = 0; p < inst->numPerDia__; p++)
		for (int d = 0; d < inst->numDia__; d++)
			for (int u = 0; u < inst->numTur__; u++)
				s->matSolTur_[offset3D(p, d, u, inst->numDia__, inst->numTur__)] = -1;
	int pos = 0;
	for (int r = 0; r < inst->numSal__; r++)
		for (int p = 0; p < inst->numPerTot__; p++)
			for (int c = 0; c < inst->numDis__; c++)
			{
				if (s->vetSol_[pos] > 0)
				{
					int dia = p / inst->numPerDia__;
					int per = p % inst->numPerDia__;
					if (s->matSolSal_[offset3D(per, dia, r, inst->numDia__, inst->numSal__)] == -1)
					{
						s->matSolSal_[offset3D(per, dia, r, inst->numDia__, inst->numSal__)] = c;
					}

					for (int u = 0; u < inst->numTur__; u++) {
						if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1) {
							if (s->matSolTur_[offset3D(per, dia, u, inst->numDia__, inst->numTur__)] == -1) {
								s->matSolTur_[offset3D(per, dia, u, inst->numDia__, inst->numTur__)] = c;
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
					if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] == c)
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
				if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] != -1)
					for (int r2 = r + 1; r2 < inst->numSal__; r2++)
					{
						if (s->matSolSal_[offset3D(p, d, r2, inst->numDia__, inst->numSal__)] != -1)
						{
							if (s->matSolSal_[offset3D(p, d, r2, inst->numDia__, inst->numSal__)] == s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)])
								s->vioDisSim_++;
							if (inst->vetDisciplinas__[s->matSolSal_[offset3D(p, d, r2, inst->numDia__, inst->numSal__)]].professor_ == inst->vetDisciplinas__[s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)]].professor_)
								s->vioProSim_++;
							for (int u = 0; u < inst->numTur__; u++)
								if ((inst->matDisTur__[offset2D(s->matSolSal_[offset3D(p, d, r2, inst->numDia__, inst->numSal__)], u, inst->numTur__)] == 1) &&
									(inst->matDisTur__[offset2D(s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)], u, inst->numTur__)] == 1))
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
				if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] != -1)
					if (inst->vetDisciplinas__[s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)]].numAlu_ > inst->vetSalas__[r].capacidade_)
						s->capSal_ += inst->vetDisciplinas__[s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)]].numAlu_ - inst->vetSalas__[r].capacidade_;
			}

	s->janHor_ = 0;
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			if ((s->matSolTur_[offset3D(0, d, u, inst->numDia__, inst->numTur__)] != -1) && (s->matSolTur_[offset3D(1, d, u, inst->numDia__, inst->numTur__)] == -1)) {
				s->janHor_++;
			}
			if ((s->matSolTur_[offset3D(inst->numPerDia__ -1, d, u, inst->numDia__, inst->numTur__)] != -1) && 
				(s->matSolTur_[offset3D(inst->numPerDia__ -2, d, u, inst->numDia__, inst->numTur__)] == -1)) {
				s->janHor_++;
			}
			for (int p = 2; p < inst->numPerDia__; p++)
				if ((s->matSolTur_[offset3D(p - 1, d, u, inst->numDia__, inst->numTur__)] != -1) && 
					(s->matSolTur_[offset3D(p - 2, d, u, inst->numDia__, inst->numTur__)] == -1) && 
					(s->matSolTur_[offset3D(p, d, u, inst->numDia__, inst->numTur__)] == -1)) {
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
					if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] == c)
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
					if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] == c)
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
	//contaViolacoes(s, inst);
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
			if ((s->matSolTur_[offset3D(0, d, u, inst->numDia__, inst->numTur__)] != -1) && (s->matSolTur_[offset3D(1, d, u, inst->numDia__, inst->numTur__)] == -1)) {
				s->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)] = 1;
				//printf("violacao z\n");
			}
			else
				s->vetSolZ_[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)] = 0;
			

			if ((s->matSolTur_[offset3D(inst->numPerDia__-1, d, u, inst->numDia__, inst->numTur__)] != -1) && 
				(s->matSolTur_[offset3D(inst->numDia__ - 2, d, u, inst->numDia__, inst->numTur__)] == -1)) {
				s->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)] = 1;
				//printf("violacao z\n");
			}
			else 
				s->vetSolZ_[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)] = 0;
			

			for (int p = 2; p < inst->numPerDia__; p++)
				if ((s->matSolTur_[offset3D(p - 1, d, u, inst->numDia__, inst->numTur__)] != -1) && 
					(s->matSolTur_[offset3D(p - 2, d, u, inst->numDia__, inst->numTur__)] == -1) && 
					(s->matSolTur_[offset3D(p, d, u, inst->numDia__, inst->numTur__)] == -1)) {
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
					if (s->matSolSal_[offset3D(p, d, r, inst->numDia__, inst->numSal__)] == c)
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
void imprimeX(Solucao *sol, Instancia* inst) {

	int pos = 0;
	for (int r = 0; r < inst->numTur__; r++) {
		for (int p = 0; p < inst->numDia__; p++) {
			for (int c = 0; c < inst->numPerDia__; c++) {
				//printf("x_%d_%d_%d = %f\n", p, r, c, sol->vetSolZ_[pos]);
				printf("%.4f; ", sol->vetSol_[pos]);
				pos++;
			}
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
				//printf("z_%d_%d_%d = %f\n", u, d, s, sol->vetSolZ_[pos]);
				printf("%.4f; ", sol->vetSolZ_[pos]);
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
			//printf("y_%d_%d = %f\n", r, c, sol->vetSolY_[pos]);
			printf("%.4f; ", sol->vetSolY_[pos]);
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

	int tamSolSal = inst->numPerDia__ * inst->numDia__ * inst->numSal__;
	clone->matSolSal_ = (int*)malloc(tamSolSal * sizeof(int));
	memcpy(clone->matSolSal_, sol->matSolSal_, tamSolSal * sizeof(int));

	int tamSolTur = inst->numPerDia__ * inst->numDia__ * inst->numTur__;
	clone->matSolTur_ = (int*)malloc(tamSolTur * sizeof(int));
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
	free(s->matSolSal_);
	free(s->matSolTur_);
	free(s);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void compararSolucoes(Solucao* sol1, Solucao* sol2, Instancia* inst) {

	int pos = 0;
	for (int r = 0; r < inst->numTur__; r++) {
		for (int p = 0; p < inst->numDia__; p++) {
			for (int c = 0; c < inst->numPerDia__; c++) {
				if (sol1->vetSol_[pos] != sol2->vetSol_[pos]) {
					printf("x_%d_%d_%d diferente\n", p, r, c);
				}
				pos++;
			}
		}
	}

	pos = 0;
	for (int u = 0; u < inst->numTur__; u++) {
		for (int d = 0; d < inst->numDia__; d++) {
			for (int s = 0; s < inst->numPerDia__; s++) {
				if (sol1->vetSolZ_[pos] != sol2->vetSolZ_[pos]) {
					printf("z_%d_%d_%d diferente\n", u, d, s);
				}
				pos++;
			}
		}
	}

	pos = 0;
	for (int c = 0; c < inst->numDis__; c++) {
		for (int r = 0; r < inst->numSal__; r++) {
			if (sol1->vetSolY_[pos] != sol2->vetSolY_[pos]) {
				printf("y_%d_%d diferente\n", r, c);
			}
			pos++;
		}
	}
}
//------------------------------------------------------------------------------
