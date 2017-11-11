#ifndef COLECOESH
#define COLECOESH

int offset3D(int x, int y, int z, int tamY, int tamZ);
int offset2D(int x, int y, int tamY);
double** getMat2DDouble(int numLin, int numCol, double initVal);
void initVetDouble(double* vet, int tam, double val);

#endif
