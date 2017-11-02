#include <stdlib.h>
#include<stdio.h>

int offset3D(int x, int y, int z, int tamY, int tamZ) {
	//printf("x = %d, y = %d, z = %d, tamY = %d, tamZ = %d\n", x, y, z, tamY, tamZ);
	//printf("%d + (%d * %d) + (%d * %d * %d) = %d\n", z,y,tamZ,);
	return z + (y * tamZ) + (x * tamZ * tamY);
}

int offset2D(int x, int y, int tamY) {
	return y + (x * tamY);
}