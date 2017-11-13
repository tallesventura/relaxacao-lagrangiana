#ifndef SolucaoH
#define SolucaoH

#include "ValoresLimites.h"
#include "Instancia.h"

typedef struct tSolucao
{
	// restri��es SOFT
	int capSal_;    // n�mero de viola��es � restri��o de capacidade da sala
	int janHor_;    // n�mero de viola��es � restri��o de janela de hor�rio das turmas
	int diaMin_;    // n�mero de viola��es � restri��o de dias m�nimos
	int salDif_;    // n�mero de viola��es � restri��o desalas diferentes

					// FO
	double funObj_;    // valor da fun��o objetivo

					// restri��es HARD 
	int vioNumAul_; // viola��es na restri��o 1 - n�mero de aulasA
	int vioAulSim_; // viola��es na restri��o 2 - aulas simultaneas
	int vioDisSim_; // viola��es na restri��o 3 - disciplinas simultaneas (Duas aulas da mesma disciplina em um mesmo per�odo)
	int vioProSim_; // viola��es na restri��o 4 - professores simultaneos
	int vioTurSim_; // viola��es na restri��o 5 - turmas simultaneas

					// CPLEX
	int numVar_;
	int numRes_;
	//double* vetViab_;  // vetor que armazena se as restri��es foram satisfeitas
	double valSol_; // valor da solu��o obtida pelo CPLEX
	double bstNod_; // valor do melhor n� (lower bound)
	double tempo_;  // tempo de execu��o

	//double* vetViabJanHor_; // vetor que armazena se as restri��es de janela-horario foram satisfeitas
	//double* vetViab14_;     // vetor que armazena se as restri��es 14 foram satisfeitas
	//double* vetViab15_;		// vetor que armazena se as restri��es 15 foram satisfeitas

	// vetor de solu��o (vari�veis x)
	double* vetSol_;
	// Matriz de solu��o (vari�veis z)
	double* vetSolZ_;
	// vetor de solu��o (vari�veis q)
	double* vetSolQ_;
	// vetor de solu��o (vari�veis z)
	double* vetSolY_;
	// vetor de solu��o (vari�veis v)
	double* vetSolV_;

	// matriz de solu��o (per�odo x dia x sala)
	int matSolSal_[MAX_PER][MAX_DIA][MAX_SAL];

	// matriz de solu��o (per�odo x dia x turma)
	int matSolTur_[MAX_PER][MAX_DIA][MAX_TUR];
}Solucao;

//// M�TODOS ===================================================================================

void escreverSol(Solucao* s, char *arq, Instancia* inst);
void contaViolacoes(Solucao *s, Instancia* inst);
void montaSolucao(Solucao *s, Instancia* inst);
void viabilizaSol(Solucao *s, Instancia* inst);
void imprimeZ(Solucao *sol, Instancia* inst);
void imprimeY(Solucao *sol, Instancia* inst);
void calculaFO(Solucao *sol, Instancia* inst);
Solucao* clonarSolucao(Solucao *sol, Instancia* inst);
void desalocaSolucao(Solucao *s);
#endif // Solucao