#ifndef DisciplinaH
#define DisciplinaH

//------------------------------------------------------------------------------
typedef struct tDisciplina
{
	char nome_[50]; // nome da disciplina
	int professor_; // id do professor
	int numPer_;    // n�mero de per�odos de oferta (n�mero de aulas)
	int diaMin_;    // dias m�nimos para distribui��o
	int numAlu_;    // n�mero de alunos
}Disciplina;
//------------------------------------------------------------------------------

#endif