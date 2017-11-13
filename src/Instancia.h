#ifndef InstanciaH
#define InstanciaH

#include "Disciplina.h"
#include "Turma.h"
#include "Professor.h"
#include "Sala.h"
#include "Restricao.h"
#include "ValoresLimites.h"
#include "RestricoesModelo.h"

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
	Disciplina* vetDisciplinas__;
	Turma* vetTurmas__;
	Professor vetProfessores__[MAX_PRO];
	Sala* vetSalas__;
	Restricao* vetRestricoes__;

	double* vetCoefX; //Já tem os pesos
	double* vetCoefZ; //Já tem os pesos
	double* vetCoefQ; //Já tem os pesos
	double* vetCoefY; //Já tem os pesos

	RestJanHor *vetRestJanHor__; // Vetor com as restrições de janela horário
	RestSalDif *vetRest14__; // Vetor com as restrições do tipo 14 (salas diferentes)
	RestSalDif *vetRest15__; // Vetor com as restrições do tipo 15 (salas diferentes)

	// ------------ Auxiliares
	int matDisTur__[MAX_DIS][MAX_TUR]; // Dis x Cur; 1 se a disciplina d faz parte do currículo c; 0 caso contrário
}Instancia;
//------------------------------------------------------------------------------


// MÉTODOS =====================================================================
Instancia* lerInstancia(char *arq);
void initCoefsFO(Instancia* inst);
Instancia* clonarInstancia(Instancia* inst);
void initVetCoefXFO(Instancia* inst);
void montaCoefRestJanHor(Instancia* inst);
void montaCoefRestSalDif(Instancia* inst);
RestJanHor* getVetJanHor(Instancia* inst, int numRest);
RestSalDif* getVetSalDif(Instancia *inst, int numRest);
void initRestSalDif(RestSalDif *rest, Instancia* inst);
void initRestJanHor(RestJanHor *rest, Instancia* inst);
void montaVetCoefXFO(Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15);
void montaVetCoefZFO(Instancia* inst, double* vetMultRes10);
void montaVetCoefQFO(Instancia* inst);
void montaVetCoefYFO(Instancia* inst, double* vetMultRes14, double* vetMultRes15);
void montaVetCoefsFO(Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15);
void desalocaIntancia(Instancia* inst);

#endif // !InstanciaH