#include "RelLagran.h"
//#include "uCBCTT.h"

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include "..\lib\cplex\include\cplex.h"

// coefs originais da FO, 
Solucao* execRelLagran(double* vetAlpha, int tamAlpha, Instancia* inst, RestJanHor *vetRestJanHor, int coefMatXFO[MAX_PER * MAX_DIA][MAX_SAL][MAX_DIS]) {

	Solucao* sol = (Solucao*)malloc(sizeof(Solucao));
	double lb = -INFINITY;
	double ub = INFINITY;

	return sol;
}
	
