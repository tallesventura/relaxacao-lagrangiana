
#include<stdlib.h>
#include <conio.h>
#include<stdio.h>

#include "Solucao.h"

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
						if (inst->matDisTur__[c][u] == 1)
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
								if ((inst->matDisTur__[s->matSolSal_[p][d][r2]][u] == 1) && (inst->matDisTur__[s->matSolSal_[p][d][r]][u] == 1))
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
						s->capSal_ += inst->vetDisciplinas__[s->matSolSal_[p][d][r]].numAlu_ - inst->vetSalas__[r].capacidade_;
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
