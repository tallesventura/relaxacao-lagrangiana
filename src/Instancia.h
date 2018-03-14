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
	Disciplina* vetDisciplinas__;
	Turma* vetTurmas__;
	Professor* vetProfessores__;
	Sala* vetSalas__;
	Restricao* vetRestricoes__;

	double* vetCoefX; //J� tem os pesos
	double* vetCoefZ; //J� tem os pesos
	double* vetCoefQ; //J� tem os pesos
	double* vetCoefY; //J� tem os pesos

	// ------------ Auxiliares
	int* matDisTur__; // Dis x Cur; 1 se a disciplina d faz parte do curr�culo c; 0 caso contr�rio
}Instancia;
//------------------------------------------------------------------------------


typedef struct tRestricoesRelaxadas {
	RestJanHor *vetRestJanHor__;	// Vetor com as restri��es de janela hor�rio
	RestSalDif *vetRest14__;		// Vetor com as restri��es do tipo 14 (salas diferentes)
	RestSalDif *vetRest15__;		// Vetor com as restri��es do tipo 15 (salas diferentes)
}RestricoesRelaxadas;


// M�TODOS =====================================================================
Instancia* lerInstancia(char *arq);
void initCoefsFO(Instancia* inst);
Instancia* clonarInstancia(Instancia* inst);
void initVetCoefXFO(Instancia* inst);
void montaCoefRestJanHor(Instancia* inst, RestricoesRelaxadas* rest);
void montaCoefRestSalDif(Instancia* inst, RestricoesRelaxadas* rest);
RestJanHor* getVetJanHor(Instancia* inst, int numRest);
RestSalDif* getVetSalDif(Instancia *inst, int numRest);
void initRestSalDif(RestSalDif *rest, Instancia* inst);
void initRestJanHor(RestJanHor *rest, Instancia* inst);
void montaVetCoefXFO(Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest);
void montaVetCoefZFO(Instancia* inst, double* vetMultRes10, RestricoesRelaxadas* rest);
void montaVetCoefQFO(Instancia* inst);
void montaVetCoefYFO(Instancia* inst, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest);
void montaVetCoefsFO(Instancia* inst, double* vetMultRes10, double* vetMultRes14, double* vetMultRes15, RestricoesRelaxadas* rest);
void desalocaIntancia(Instancia* inst);
double** getMatD(Instancia* inst, RestricoesRelaxadas* rest);
double* getVetD(Instancia* inst);
void desalocaRestricoes(RestricoesRelaxadas* rest, Instancia* inst);
#endif // !InstanciaH