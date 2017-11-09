#ifndef InstanciaH
#define InstanciaH

#include "Disciplina.h"
#include "Turma.h"
#include "Professor.h"
#include "Sala.h"
#include "Restricao.h"

//------------------------------------------------------------------------------
typedef struct tInstancia {
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

	// ------------ Auxiliares
	int matDisTur__[MAX_DIS][MAX_TUR]; // Dis x Cur; 1 se a disciplina d faz parte do currículo c; 0 caso contrário
}Instancia;
//------------------------------------------------------------------------------


// MÉTODOS =====================================================================
Instancia* lerInstancia(char *arq);

#endif // !InstanciaH