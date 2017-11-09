#ifndef DisciplinaH
#define DisciplinaH

//------------------------------------------------------------------------------
typedef struct tDisciplina
{
	char nome_[50]; // nome da disciplina
	int professor_; // id do professor
	int numPer_;    // número de períodos de oferta (número de aulas)
	int diaMin_;    // dias mínimos para distribuição
	int numAlu_;    // número de alunos
}Disciplina;
//------------------------------------------------------------------------------

#endif