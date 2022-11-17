#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
double** load_data(char* filename, int *_r, int *_c)
{
	double **matrix;
	int r, c, i;
	FILE *input_file;
	input_file = fopen(filename, "r");
	if(input_file != NULL)
	{
		fread(&r, sizeof(int), 1, input_file);
		fread(&c, sizeof(int), 1, input_file);
		size_t elems = (size_t)r*c;
		size_t size = elems*sizeof(double);
		//printf("r: %d, c: %d, elems: %ld, size: %ld\n", r, c, elems, size);
		matrix = (double**)malloc(r*sizeof(double*));
		matrix[0] = (double*)malloc(size*sizeof(double));
		for(int i=1;i<r;i++) matrix[i] = matrix[0]+(size_t)i*r;
		fread(matrix[0], sizeof(double), elems, input_file);
		fclose(input_file);
	}
	else
	{
		printf("Unbale to open file %s\n", filename);
		exit(1);
	}
	*_r = r;
	*_c = c;
	return matrix;
}
int main(int argc, char **argv)
{
	char *filename = argc == 2? strdup(argv[1]):"default-matrix-file1.dat";
	printf("filename: %s\n", filename);
	int r, c, i, j;
	double **matrix = load_data(filename, &r, &c);
	printf("r: %d, c: %d\n", r, c);
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
			printf("%0.2lf ", matrix[i][j]);
		printf("\n");
	}
}
