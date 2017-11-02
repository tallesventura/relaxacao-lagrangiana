#include <stdlib.h>
#include<stdio.h>

int offset3D(int x, int y, int z, int tamY, int tamZ) {
	return z + (y * tamZ) + (x * tamZ * tamY);
}

int offset2D(int x, int y, int tamY) {
	return y + (x * tamY);
}