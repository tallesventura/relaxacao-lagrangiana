#ifndef SolucaoH
#define SolucaoH

#include "ValoresLimites.h"
#include "Instancia.h"

typedef struct tSolucao
{
	// restrições SOFT
	int capSal_;    // número de violações à restrição de capacidade da sala
	int janHor_;    // número de violações à restrição de janela de horário das turmas
	int diaMin_;    // número de violações à restrição de dias mínimos
	int salDif_;    // número de violações à restrição desalas diferentes

					// FO
	double funObj_;    // valor da função objetivo

					// restrições HARD 
	int vioNumAul_; // violações na restrição 1 - número de aulasA
	int vioAulSim_; // violações na restrição 2 - aulas simultaneas
	int vioDisSim_; // violações na restrição 3 - disciplinas simultaneas (Duas aulas da mesma disciplina em um mesmo período)
	int vioProSim_; // violações na restrição 4 - professores simultaneos
	int vioTurSim_; // violações na restrição 5 - turmas simultaneas

					// CPLEX
	int numVar_;
	int numRes_;
	//double* vetViab_;  // vetor que armazena se as restrições foram satisfeitas
	double valSol_; // valor da solução obtida pelo CPLEX
	double bstNod_; // valor do melhor nó (lower bound)
	double tempo_;  // tempo de execução

	//double* vetViabJanHor_; // vetor que armazena se as restrições de janela-horario foram satisfeitas
	//double* vetViab14_;     // vetor que armazena se as restrições 14 foram satisfeitas
	//double* vetViab15_;		// vetor que armazena se as restrições 15 foram satisfeitas

	// vetor de solução (variáveis x)
	double* vetSol_;
	// Matriz de solução (variáveis z)
	double* vetSolZ_;
	// vetor de solução (variáveis q)
	double* vetSolQ_;
	// vetor de solução (variáveis z)
	double* vetSolY_;
	// vetor de solução (variáveis v)
	double* vetSolV_;

	// matriz de solução (período x dia x sala)
	int matSolSal_[MAX_PER][MAX_DIA][MAX_SAL];

	// matriz de solução (período x dia x turma)
	int matSolTur_[MAX_PER][MAX_DIA][MAX_TUR];
}Solucao;

//// MÉTODOS ===================================================================================

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