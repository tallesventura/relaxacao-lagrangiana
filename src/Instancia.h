#ifndef InstanciaH
#define InstanciaH

#include "Disciplina.h"
#include "Turma.h"
#include "Professor.h"
#include "Sala.h"
#include "Restricao.h"

//------------------------------------------------------------------------------
typedef struct tInstancia {
	char nomInst__[150]; // nome da inst�ncia
	int numDis__;        // n�mero de disciplinas 
	int numTur__;        // n�mero de turmas
	int numPro__;        // n�mero de professores
	int numSal__;        // n�mero de salas
	int numDia__;        // n�mero de dias
	int numPerDia__;     // n�mero de per�odos por dia
	int numPerTot__;     // n�mero de per�odos total
	int numRes__;        // n�mero de restri��es
	int numSol__;        // n�mero de colunas
	int numVar__;        // n�mero de vari�veis
	Disciplina vetDisciplinas__[MAX_DIS];
	Turma vetTurmas__[MAX_TUR];
	Professor vetProfessores__[MAX_PRO];
	Sala vetSalas__[MAX_SAL];
	Restricao vetRestricoes__[MAX_RES];

	// ------------ Auxiliares
	int matDisTur__[MAX_DIS][MAX_TUR]; // Dis x Cur; 1 se a disciplina d faz parte do curr�culo c; 0 caso contr�rio
}Instancia;
//------------------------------------------------------------------------------


// M�TODOS =====================================================================
Instancia* lerInstancia(char *arq);

#endif // !InstanciaH