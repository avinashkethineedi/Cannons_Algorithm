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
void write_data(char *filename, double **matrix, int r, int c)
{
	FILE *output_file;
	output_file = fopen(filename, "wb");
	if(output_file != NULL)
	{
		fwrite(&r, sizeof(int), 1, output_file);
		fwrite(&c, sizeof(int), 1, output_file);
		fwrite(matrix[0], sizeof(double), (size_t)r*c, output_file);
		fclose(output_file);
	}
	else
	{
		printf("Unbale to open file %s\n", filename);
		exit(1);
	}
}
int main(int argc, char **argv)
{
	char *filename1 = argc >= 2? strdup(argv[1]):"default-matrix-file1.dat",
		*filename2 = argc >= 3? strdup(argv[2]):"default-matrix-file2.dat",
		*filename3 = argc == 4? strdup(argv[3]):"output-matrix-file.dat";
	printf("input file1: %s\ninput file2: %s\noutput file: %s\n", filename1, filename2, filename3);
	int Ar, Ac, Br, Bc, i, j, k;
	double **matrix_A, **matrix_B, **matrix_C;
	matrix_A = load_data(filename1, &Ar, &Ac);
	matrix_B = load_data(filename2, &Br, &Bc);
	size_t elems = (size_t)Ar*Bc;
	size_t size = elems*sizeof(double);
	matrix_C = (double**)malloc(Ar*sizeof(double*));
	matrix_C[0] = (double*)malloc(size*sizeof(double));
	for(int i=1;i<Ar;i++) matrix_C[i] = matrix_C[0]+(size_t)i*Ar;
	for(i=0;i<Ar;i++)
		for(j=0;j<Bc;j++)
		{
			matrix_C[i][j] = 0;
			for(k=0;k<Ac;k++)
				matrix_C[i][j] += (matrix_A[i][k] * matrix_B[k][j]);
		}
	write_data(filename3, matrix_C, Ar, Bc);
	//free data
	free(matrix_A[0]);
	free(matrix_B[0]);
	free(matrix_C[0]);
	free(matrix_A);
	free(matrix_B);
	free(matrix_C);
}
