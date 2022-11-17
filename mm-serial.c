#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char **argv)
{
    char *filename1 = argc >= 2? strdup(argv[1]):"default-matrix-file1.dat",
        *filename2 = argc == 3? strdup(argv[2]):"default-matrix-file2.dat";
	printf("input file1: %s, input file2: %s\n", filename1, filename2);
	int Ar, Ac, Br, Bc, i, j;
}
