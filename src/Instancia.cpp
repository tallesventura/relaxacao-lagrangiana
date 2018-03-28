#include "Instancia.h"
#include "Colecoes.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//------------------------------------------------------------------------------
Instancia* lerInstancia(char *arq)
{
	Instancia* inst = (Instancia*)malloc(sizeof(Instancia));
	int pos;
	char aux[50];
	FILE *f = fopen(arq, "r");
	fscanf(f, "Name: %s\n", &inst->nomInst__);
	fscanf(f, "Courses: %d\n", &inst->numDis__);
	inst->vetDisciplinas__ = (Disciplina*)malloc(inst->numDis__ * sizeof(Disciplina));
	fscanf(f, "Rooms: %d\n", &inst->numSal__);
	inst->vetSalas__ = (Sala*)malloc(inst->numSal__ * sizeof(Sala));
	fscanf(f, "Days: %d\n", &inst->numDia__);
	fscanf(f, "Periods_per_day: %d\n", &inst->numPerDia__);
	fscanf(f, "Curricula: %d\n", &inst->numTur__);
	inst->vetTurmas__ = (Turma*)malloc(inst->numTur__ * sizeof(Turma));
	fscanf(f, "Constraints: %d\n", &inst->numRes__);
	inst->vetRestricoes__ = (Restricao*)malloc(inst->numRes__ * sizeof(Restricao));

	inst->vetProfessores__ = (Professor*)malloc(MAX_PRO * sizeof(Professor));
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

	inst->matDisTur__ = (int*)malloc(inst->numDis__ * inst->numTur__ * sizeof(int));

	pos = 0;
	for (int i = 0; i < inst->numDis__; i++) {
		for (int j = 0; j < inst->numTur__; j++) {
			inst->matDisTur__[pos] = 0;
			pos++;
		}
	}
			
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
					inst->matDisTur__[offset2D(k,i, inst->numTur__)] = 1;
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
void initCoefsFO(Instancia* inst) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;

	inst->vetCoefX = (double*)malloc(numX * sizeof(double));
	inst->vetCoefZ = (double*)malloc(numZ * sizeof(double));
	inst->vetCoefQ = (double*)malloc(numQ * sizeof(double));
	inst->vetCoefY = (double*)malloc(numY * sizeof(double));

	initVetCoefXFO(inst);
	initVetDouble(inst->vetCoefZ, numZ, PESOS[1]);
	initVetDouble(inst->vetCoefQ, numQ, PESOS[2]);
	initVetDouble(inst->vetCoefY, numY, PESOS[3]);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Instancia* clonarInstancia(Instancia* inst) {

	Instancia* clone = (Instancia*) malloc(sizeof(Instancia));

	strcpy_s(clone->nomInst__, inst->nomInst__);
	clone->numDis__ = inst->numDis__;
	clone->numTur__ = inst->numTur__;
	clone->numPro__ = inst->numPro__;
	clone->numSal__ = inst->numSal__;
	clone->numDia__ = inst->numDia__;
	clone->numPerDia__ = inst->numPerDia__;
	clone->numPerTot__ = inst->numPerTot__;
	clone->numRes__ = inst->numRes__;
	clone->numSol__ = inst->numSol__;
	clone->numVar__ = inst->numVar__;

	clone->vetDisciplinas__ = (Disciplina*)malloc(inst->numDis__ * sizeof(Disciplina));
	clone->vetTurmas__ = (Turma*)malloc(inst->numTur__ * sizeof(Turma));
	clone->vetProfessores__ = (Professor*)malloc(MAX_PRO * sizeof(Professor));
	clone->vetSalas__ = (Sala*)malloc(inst->numSal__ * sizeof(Sala));
	clone->vetRestricoes__ = (Restricao*)malloc(inst->numRes__ * sizeof(Restricao));

	memcpy(clone->vetDisciplinas__, inst->vetDisciplinas__, inst->numDis__*sizeof(Disciplina));
	memcpy(clone->vetTurmas__, inst->vetTurmas__, inst->numTur__*sizeof(Turma));
	memcpy(clone->vetProfessores__, inst->vetProfessores__, MAX_PRO*sizeof(Professor));
	memcpy(clone->vetSalas__, inst->vetSalas__, inst->numSal__*sizeof(Sala));
	memcpy(clone->vetRestricoes__, inst->vetRestricoes__, inst->numRes__*sizeof(Restricao));

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numQ = inst->numDis__;
	int numY = inst->numSal__ * inst->numDis__;

	clone->vetCoefX = (double*)malloc(numX * sizeof(double));
	clone->vetCoefZ = (double*)malloc(numZ * sizeof(double));
	clone->vetCoefQ = (double*)malloc(numQ * sizeof(double));
	clone->vetCoefY = (double*)malloc(numY * sizeof(double));

	memcpy(clone->vetCoefX, inst->vetCoefX, numX * sizeof(double));
	memcpy(clone->vetCoefZ, inst->vetCoefZ, numZ * sizeof(double));
	memcpy(clone->vetCoefQ, inst->vetCoefQ, numQ * sizeof(double));
	memcpy(clone->vetCoefY, inst->vetCoefY, numY * sizeof(double));

	/*int numRest10 = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__*inst->numDis__;

	clone->vetRestJanHor__ = (RestJanHor*)malloc(numRest10 * sizeof(RestJanHor));
	clone->vetRest14__ = (RestSalDif*)malloc(numRest14 * sizeof(RestSalDif));
	clone->vetRest15__ = (RestSalDif*)malloc(numRest15 * sizeof(RestSalDif));

	memcpy(clone->vetRestJanHor__, inst->vetRestJanHor__, numRest10 * sizeof(RestJanHor));
	memcpy(clone->vetRest14__, inst->vetRest14__, numRest14 * sizeof(RestSalDif));
	memcpy(clone->vetRest15__, inst->vetRest15__, numRest15 * sizeof(RestSalDif));*/

	clone->matDisTur__ = (int*)malloc(inst->numDis__ * inst->numTur__ * sizeof(int));
	memcpy(clone->matDisTur__, inst->matDisTur__, inst->numDis__ * inst->numTur__ * sizeof(int));

	return clone;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initRestJanHor(RestJanHor *rest, Instancia* inst) {

	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;

	rest->coefMatZ = (int*)malloc(numZ * sizeof(int));
	rest->coefMatX = (int*)malloc(numX * sizeof(int));

	for (int i = 0; i < numZ; i++) {
		rest->coefMatZ[i] = 0;
	}
	
	for (int i = 0; i < numX; i++) {
		rest->coefMatX[i] = 0;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
RestJanHor* getVetJanHor(Instancia* inst, int numRest) {

	RestJanHor* vet = (RestJanHor*)malloc(numRest * sizeof(RestJanHor));

	for (int i = 0; i < numRest; i++) {
		initRestJanHor(&vet[i], inst);
	}

	return vet;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initRestSalDif(RestSalDif *rest, Instancia* inst) {

	int numY = inst->numSal__ * inst->numDis__;
	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;

	rest->coefMatY = (int*)malloc(numY * sizeof(int));
	rest->coefMatX = (int*)malloc(numX * sizeof(int));

	for (int i = 0; i < numY; i++) {
		rest->coefMatY[i] = 0;
	}

	for (int i = 0; i < numX; i++) {
		rest->coefMatX[i] = 0;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
RestSalDif* getVetSalDif(Instancia *inst, int numRest) {

	RestSalDif *vet = (RestSalDif*)malloc(numRest * sizeof(RestSalDif));

	for (int i = 0; i < numRest; i++) {
		initRestSalDif(&vet[i], inst);
	}

	return vet;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void initVetCoefXFO(Instancia* inst) {

	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {
				int pos = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
				if (inst->vetDisciplinas__[c].numAlu_ > inst->vetSalas__[r].capacidade_)
					inst->vetCoefX[pos] = PESOS[0] * (inst->vetDisciplinas__[c].numAlu_ - inst->vetSalas__[r].capacidade_);
				else
					inst->vetCoefX[pos] = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------

int contaCoefsNaoNulos(Instancia* inst, RestricoesRelaxadas* rest) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__*inst->numDis__;

	int numCoefs = 0;

	for (int i = 0; i < numRest10; i++) {
		numCoefs += rest->vetRestJanHor__[i].numCoefsNaoNulos;
	}
	for (int i = 0; i < numRest14; i++) {
		numCoefs += rest->vetRest14__[i].numCoefsNaoNulos;
	}
	for (int i = 0; i < numRest15; i++) {
		numCoefs += rest->vetRest15__[i].numCoefsNaoNulos;
	}

	return numCoefs;
}

MatRestCplex* montaMatRestricoesCplex(RestricoesRelaxadas* rest, Instancia* inst) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ * inst->numDis__;
	int numVar = numX + numZ + numY;

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__*inst->numDis__;
	int numRest = numRest10 + numRest14 + numRest15;

	MatRestCplex* matRest = (MatRestCplex*)malloc(sizeof(MatRestCplex));
	matRest->numCoefsTotal = contaCoefsNaoNulos(inst, rest);
	matRest->numCol = numVar;
	matRest->numLin = numRest;
	matRest->matval = (int*)malloc(matRest->numCoefsTotal * sizeof(int));
	matRest->matbeg = (int*)malloc(numVar * sizeof(int));
	matRest->matind = (int*)malloc(matRest->numCoefsTotal * sizeof(int));
	matRest->matcnt = (int*)malloc(numVar * sizeof(int));

	int posX1, posX2, posX3, posZ, posY, coef, col, lin, posVal, countVal;
	col = lin = posVal = countVal = 0;

	// Coeficientes de X
	for (int j = 0; j < numX; j++) {
		matRest->matbeg[col] = posVal;
		countVal = 0;
		lin = 0;

		// Rest 10
		for (int i = 0; i < numRest10; i++) {
			coef = rest->vetRestJanHor__[i].coefMatX[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		// Rest 14
		for (int i = 0; i < numRest14; i++) {
			coef = rest->vetRest14__[i].coefMatX[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		// Rest 15
		for (int i = 0; i < numRest15; i++) {
			coef = rest->vetRest15__[i].coefMatX[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		matRest->matcnt[col] = countVal;
		col++;
	}

	// Coeficientes de Z
	for (int j = 0; j < numZ; j++) {
		matRest->matbeg[col] = posVal;
		countVal = 0;
		lin = 0;

		// Rest 10
		for (int i = 0; i < numRest10; i++) {
			coef = rest->vetRestJanHor__[i].coefMatZ[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		matRest->matcnt[col] = countVal;
		col++;
	}

	// Coeficientes de Y
	for (int j = 0; j < numY; j++) {
		matRest->matbeg[col] = posVal;
		countVal = 0;
		lin = numRest10;

		// Rest 14
		for (int i = 0; i < numRest14; i++) {
			coef = rest->vetRest14__[i].coefMatY[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		// Rest 15
		for (int i = 0; i < numRest15; i++) {
			coef = rest->vetRest15__[i].coefMatY[j];

			if (coef != 0) {
				matRest->matval[posVal] = coef;
				matRest->matind[posVal] = lin;
				posVal++;
				countVal++;
			}
			lin++;
		}

		matRest->matcnt[col] = countVal;
		col++;
	}

	return matRest;
}

void desalocaMatRestCplex(MatRestCplex* mat) {

	free(mat->matbeg);
	free(mat->matcnt);
	free(mat->matind);
	free(mat->matval);
	free(mat);
}

void imprimeMatRestCplex(MatRestCplex* mat, Instancia* inst) {

	printf("MATBEG:\n");
	for (int i = 0; i < mat->numCol; i++) {
		printf("%d; ", mat->matbeg[i]);
	}
	printf("\n");

	printf("MATIND:\n");
	for (int i = 0; i < mat->numCoefsTotal; i++) {
		printf("%d; ", mat->matind[i]);
	}
	printf("\n");

	printf("MATCNT:\n");
	for (int i = 0; i < mat->numCol; i++) {
		printf("%d; ", mat->matcnt[i]);
	}
	printf("\n");

	printf("MATVAL:\n");
	for (int i = 0; i < mat->numCoefsTotal; i++) {
		printf("%d; ", mat->matval[i]);
	}
	printf("\n");

}

//------------------------------------------------------------------------------
void montaCoefRestJanHor(Instancia* inst, RestricoesRelaxadas* rest) {

	int numRest = inst->numTur__*inst->numDia__*inst->numPerDia__;
	int pos = 0;
	int numCoefsNaoNulos = 0;

	// Primeiro período do dia
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						rest->vetRestJanHor__[pos].coefMatX[offset3D(r, d*inst->numPerDia__, c, inst->numPerTot__, inst->numDis__)] = 1;
						rest->vetRestJanHor__[pos].coefMatX[offset3D(r, d*inst->numPerDia__ + 1, c, inst->numPerTot__, inst->numDis__)] = -1;
						numCoefsNaoNulos += 2;
					}
				}
			rest->vetRestJanHor__[pos].coefMatZ[offset3D(u, d, 0, inst->numDia__, inst->numPerDia__)] = -1;
			numCoefsNaoNulos++;
			rest->vetRestJanHor__[pos].numCoefsNaoNulos = numCoefsNaoNulos;
			pos++;
			numCoefsNaoNulos = 0;
		}
	}

	// Último período do dia
	for (int u = 0; u < inst->numTur__; u++)
	{
		for (int d = 0; d < inst->numDia__; d++)
		{
			for (int c = 0; c < inst->numDis__; c++)
				if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
				{
					for (int r = 0; r < inst->numSal__; r++) {
						rest->vetRestJanHor__[pos].coefMatX[offset3D(r, (d*inst->numPerDia__) + inst->numPerDia__ - 1, c, inst->numPerTot__, inst->numDis__)] = 1;
						rest->vetRestJanHor__[pos].coefMatX[offset3D(r, (d*inst->numPerDia__) + inst->numPerDia__ - 2, c, inst->numPerTot__, inst->numDis__)] = -1;
						numCoefsNaoNulos += 2;
					}
				}
			rest->vetRestJanHor__[pos].coefMatZ[offset3D(u, d, 1, inst->numDia__, inst->numPerDia__)] = -1;
			numCoefsNaoNulos++;
			rest->vetRestJanHor__[pos].numCoefsNaoNulos = numCoefsNaoNulos;
			pos++;
			numCoefsNaoNulos = 0;
		}
	}

	// Períodos intermediários do dia
	for (int s = 2; s < inst->numPerDia__; s++)
	{
		for (int u = 0; u < inst->numTur__; u++)
		{
			for (int d = 0; d < inst->numDia__; d++)
			{
				for (int c = 0; c < inst->numDis__; c++)
					if (inst->matDisTur__[offset2D(c, u, inst->numTur__)] == 1)
					{
						for (int r = 0; r < inst->numSal__; r++) {
							rest->vetRestJanHor__[pos].coefMatX[offset3D(r, (d*inst->numPerDia__) + s - 1, c, inst->numPerTot__, inst->numDis__)] = 1;
							rest->vetRestJanHor__[pos].coefMatX[offset3D(r, (d*inst->numPerDia__) + s - 2, c, inst->numPerTot__, inst->numDis__)] = -1;
							rest->vetRestJanHor__[pos].coefMatX[offset3D(r, (d*inst->numPerDia__) + s, c, inst->numPerTot__, inst->numDis__)] = -1;
							numCoefsNaoNulos += 3;
						}
					}
				rest->vetRestJanHor__[pos].coefMatZ[offset3D(u, d, s, inst->numDia__, inst->numPerDia__)] = -1;
				numCoefsNaoNulos++;
				rest->vetRestJanHor__[pos].numCoefsNaoNulos = numCoefsNaoNulos;
				pos++;
				numCoefsNaoNulos = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaCoefRestSalDif(Instancia* inst, RestricoesRelaxadas* rest) {

	// Restrição 14
	int numCoefsNaoNulos = 0;
	int pos = 0;
	for (int p = 0; p < inst->numPerTot__; p++) {
		for (int r = 0; r < inst->numSal__; r++) {
			for (int c = 0; c < inst->numDis__; c++) {
				rest->vetRest14__[pos].coefMatX[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)] = 1;
				rest->vetRest14__[pos].coefMatY[offset2D(c, r, inst->numSal__)] = -1;
				numCoefsNaoNulos += 2;
				rest->vetRest14__[pos].numCoefsNaoNulos = numCoefsNaoNulos;
				pos++;
				numCoefsNaoNulos = 0;
			}
		}
	}
	

	// Restrição 15
	numCoefsNaoNulos = 0;
	pos = 0;
	for (int r = 0; r < inst->numSal__; r++)
	{
		for (int c = 0; c < inst->numDis__; c++)
		{
			for (int p = 0; p < inst->numPerTot__; p++) {
				rest->vetRest15__[pos].coefMatX[offset3D(r, p, c, inst->numPerTot__, inst->numDis__)] = 1;
				numCoefsNaoNulos++;
			}
			rest->vetRest15__[pos].coefMatY[offset2D(c, r, inst->numSal__)] = -1;
			numCoefsNaoNulos++;
			rest->vetRest15__[pos].numCoefsNaoNulos = numCoefsNaoNulos;
			pos++;
			numCoefsNaoNulos = 0;
		}
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaVetCoefXFO(Instancia* inst, double* vetMult, MatRestCplex* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int ini, fim;

	double soma = 0;
	int k = 0;

	for (int col = 0; col < numX; col++) {
		ini = rest->matbeg[col];
		fim = ini + rest->matcnt[col];
		for (int i = ini; i < fim; i++) {
			soma += rest->matval[i] * vetMult[rest->matind[i]];
		}

		inst->vetCoefX[k] -= soma;
		k++;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaVetCoefZFO(Instancia* inst, double* vetMult, MatRestCplex* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int firstCol = offsetZ(0, numX);
	int lastCol = firstCol + numZ;
	int ini, fim;

	double soma = 0;
	int k = 0;

	for (int col = firstCol; col < lastCol; col++) {
		ini = rest->matbeg[col];
		fim = ini + rest->matcnt[col];
		for (int i = ini; i < fim; i++) {
			soma += rest->matval[i] * vetMult[rest->matind[i]];
		}

		inst->vetCoefZ[k] -= soma;
		k++;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaVetCoefQFO(Instancia* inst) {

	int numQ = inst->numDis__;

	for (int c = 0; c < inst->numDis__; c++) {
		inst->vetCoefQ[c] = PESOS[2] * inst->vetCoefQ[c];
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaVetCoefYFO(Instancia* inst, double* vetMult, MatRestCplex* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numZ = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numY = inst->numSal__ + inst->numDis__;
	int firstCol = offsetY(0, numX, numZ);
	int lastCol = firstCol + numY;
	int ini, fim;

	double soma = 0;
	int k = 0;

	for (int col = firstCol; col < lastCol; col++) {
		ini = rest->matbeg[col];
		fim = ini + rest->matcnt[col];
		for (int i = ini; i < fim; i++) {
			soma += rest->matval[i] * vetMult[rest->matind[i]];
		}

		inst->vetCoefY[k] -= soma;
		k++;
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void montaVetCoefsFO(Instancia* inst, double* vetMult, MatRestCplex* rest) {

	montaVetCoefXFO(inst, vetMult, rest);
	montaVetCoefZFO(inst, vetMult, rest);
	montaVetCoefYFO(inst, vetMult, rest);

}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
void desalocaIntancia(Instancia* inst) {

	free(inst->vetDisciplinas__);
	free(inst->vetTurmas__);
	free(inst->vetProfessores__);
	free(inst->vetSalas__);
	free(inst->vetRestricoes__);
	free(inst->vetCoefX);
	free(inst->vetCoefZ);
	free(inst->vetCoefQ);
	free(inst->vetCoefY);
	free(inst);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double** getMatD(Instancia* inst, RestricoesRelaxadas* rest) {

	int numX = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	double** matD = (double**)malloc(numRes * sizeof(double*));
	for (int i = 0; i < numRes; i++) {
		matD[i] = (double*)malloc(numX * sizeof(double));
	}

	for (int i = 0; i < numRes; i++) {
		for (int j = 0; j < numX; j++) {
			matD[i][j] = 0;
		}
	}

	int posX;
	int col = 0;
	int lin = 0;

	// Restrição 10
	for (int i = 0; i < numRest10; i++) {

		col = 0;
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					matD[lin][col] = rest->vetRestJanHor__[i].coefMatX[posX];
					col++;
				}
			}
		}
		lin++;
	}

	// Restrição 14
	for (int i = 0; i < numRest14; i++) {

		col = 0;
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					matD[lin][col] = rest->vetRest14__[i].coefMatX[posX];
					col++;
				}
			}
		}
		lin++;
	}

	// Restrição 15
	for (int i = 0; i < numRest15; i++) {

		col = 0;
		for (int r = 0; r < inst->numSal__; r++) {
			for (int p = 0; p < inst->numPerTot__; p++) {
				for (int c = 0; c < inst->numDis__; c++) {
					posX = offset3D(r, p, c, inst->numPerTot__, inst->numDis__);
					matD[lin][col] = rest->vetRest15__[i].coefMatX[posX];
					col++;
				}
			}
		}
		lin++;
	}


	return matD;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
double* getVetD(Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;
	int numRes = numRest10 + numRest14 + numRest15;

	double* vetD = (double*)malloc(numRes * sizeof(double));

	for (int i = 0; i < numRes; i++) {
		vetD[i] = 1;
	}

	return vetD;
}
//------------------------------------------------------------------------------

void desalocaRestricoes(RestricoesRelaxadas* rest, Instancia* inst) {

	int numRest10 = inst->numTur__ * inst->numDia__ * inst->numPerDia__;
	int numRest14 = inst->numPerTot__ * inst->numSal__ * inst->numDis__;
	int numRest15 = inst->numSal__ * inst->numDis__;

	for (int i = 0; i < numRest10; i++) {
		free(rest->vetRestJanHor__[i].coefMatX);
		free(rest->vetRestJanHor__[i].coefMatZ);
	}

	for (int i = 0; i < numRest14; i++) {
		free(rest->vetRest14__[i].coefMatX);
		free(rest->vetRest14__[i].coefMatY);
	}

	for (int i = 0; i < numRest15; i++) {
		free(rest->vetRest15__[i].coefMatX);
		free(rest->vetRest15__[i].coefMatY);
	}

	free(rest->vetRestJanHor__);
	free(rest->vetRest14__);
	free(rest->vetRest15__);
	free(rest);
}

int findCol(int pos, MatRestCplex* rest) {

	int col, index;
	index = col = 0;
	
	while (col < rest->numCol) {
		if (index >= pos) { break; }
		index += rest->matcnt[col];
		col++;
	}
	
	return col;
}