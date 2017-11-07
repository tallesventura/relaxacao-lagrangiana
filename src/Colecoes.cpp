#include <stdlib.h>
#include<stdio.h>

int offset3D(int x, int y, int z, int tamY, int tamZ) {
	return z + (y * tamZ) + (x * tamZ * tamY);
}

int offset2D(int x, int y, int tamY) {
	return y + (x * tamY);
}

double** getMat2DDouble(int numLin, int numCol, double initVal) {

	double **mat = (double**)malloc(numLin * sizeof(double*));

	for (int i = 0; i < numLin; i++) {
		mat[i] = (double*)malloc(numCol * sizeof(double));
	}

	for (int i = 0; i < numLin; i++) {
		for (int j = 0; j < numCol; j++) {
			mat[i][j] = initVal;
		}
	}

	return mat;
}