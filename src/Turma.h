#ifndef TurmaH
#define TurmaH

#include "ValoresLimites.h"

//------------------------------------------------------------------------------
typedef struct tTurma
{
	char nome_[50];       // nome da turma
	int numDis_;          // número de disciplinas
	int vetDis_[MAX_DIS]; // vetor com as disciplinas (ids)
}Turma;
//------------------------------------------------------------------------------

#endif