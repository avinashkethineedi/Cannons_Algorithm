/*
This file creates square matrix and each value is a 64-bit double precision floating point.
All the values are within the range of l(lower bound) and u(upper bound)
*/
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
void print_help()
{
	printf("matrix should be square matrix (r==c)\nmake sure r and c are multiples of 2520\n");
    printf("usage: ./executable -r \"rows\" -c \"columns\" -l \"lower bound\" -u \"upper bound\" -o \"file name\"\n");
}
int main(int argc, char *argv[]) 
{
	int opt, r = 2520, c = 2520, l = 50, u = 150;
	char *filename = "default-matrix-file1.dat";
	while((opt = getopt(argc, argv, ":r:c:l:u:o:")) != -1)
	{
		switch(opt)
		{
			case 'r':
				r = atoi(optarg);
				if(r < 1)
				{
					printf("invalid value for -r: %d (it requires positive integer)\n", r);
					print_help();
					exit(1);
				}
				break;
			case 'c':
				c = atoi(optarg);
				if(c < 1)
				{
					printf("invalid value for -c: %d (it requires positive integer)\n", c);
					print_help();
					exit(1);
				}
				break;
			case 'l':
				l = atoi(optarg);
				if(l < 1)
				{
					printf("invalid value for -l: %d (it requires positive integer)\n", l);
					print_help();
					exit(1);
				}
				break;
			case 'u':
				u = atoi(optarg);
				if(u < 1)
				{
					printf("invalid value for -u: %d (it requires positive integer)\n", u);
					print_help();
					exit(1);
				}
				break;
			case 'o':
				filename=strdup(optarg);
				break;
			default:
				if(optopt == 'r')
					r = 2520;
				if(optopt == 'c')
					c = 2520;
				if(optopt == 'l')
					l = 100;
				if(optopt == 'u')
					u = 100;
				else if(optopt == 'o')
					filename = "default-matrix-file1.dat";
				else
				{
					print_help();
					exit(1);
				}
		}
	}
	if(r!=c)
	{
		printf("It's not a square matrix (rows(%d) != columns(%d))\n", r, c);
		print_help();
	}
	size_t i, size = (size_t)r*c*sizeof(double), elems = (size_t)r*c;
	double *matrix = (double*)malloc(size);
	for(i=0;i<elems;i++) matrix[i] = ((double)l + ((double)rand() / (u - l))) / 1000.0;
	printf("r: %d, c: %d, l: %d, u: %d, o: %s\n", r, c, l, u, filename);
	FILE *output_file;
	output_file = fopen(filename, "wb");
	if(output_file != NULL)
	{
		fwrite(&r, sizeof(int), 1, output_file);
		fwrite(&c, sizeof(int), 1, output_file);
		fwrite(matrix, sizeof(double), (size_t)r*c, output_file);
	}
	else
	{
		printf("Unbale to open file %s\n", filename);
		exit(1);
	}
	free(matrix);
}