//Cannon's algorithm
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"MyMPI.h"
void write_matrix_data(char* filename, void **a, MPI_Datatype dtype, int m, int n, MPI_Comm grid_comm)
{
	void *buffer, *laddr;
	int coords[2], datum_size ,els, grid_coords[2],
	grid_id ,grid_period[2] ,grid_size[2] ,i, j, k,
	local_cols, p, src;
	MPI_Status status;
	MPI_Comm_rank (grid_comm, &grid_id);
	MPI_Comm_size (grid_comm, &p);
	datum_size = sizeof(MPI_DOUBLE);
	MPI_Cart_get (grid_comm, 2, grid_size, grid_period, grid_coords);
	FILE *output_file;
	output_file = fopen(filename, "wb");
	if(output_file == NULL)
	{
		printf("Unbale to open file %s\n", filename);
		exit(1);
	}
	fwrite(&m, sizeof(int), 1, output_file);
	fwrite(&n, sizeof(int), 1, output_file);
	local_cols = BLOCK_SIZE(grid_coords[1],grid_size[1],n);
	if (!grid_id) buffer = malloc ((size_t)n*datum_size);
	/* For each row of the process grid */
	for (i = 0; i < grid_size[0]; i++)
	{
		coords[0] = i;
		/* For each matrix row controlled by the process row */
		for (j = 0; j < BLOCK_SIZE(i,grid_size[0],m); j++)
		{
			/* Collect the matrix row on grid process 0 and print it */
			if (!grid_id)
			{
				for (k = 0; k < grid_size[1]; k++)
				{
					coords[1] = k;
					MPI_Cart_rank (grid_comm, coords, &src);
					els = BLOCK_SIZE(k,grid_size[1],n);
					laddr = buffer + BLOCK_LOW(k,grid_size[1],n) * datum_size;
					if (src == 0) memcpy (laddr, a[j], els * datum_size);
					else MPI_Recv(laddr, els, dtype, src, 0, grid_comm, &status);
				}
				fwrite(buffer, sizeof(double), (size_t)n, output_file);
			}
			else if (grid_coords[0] == i) MPI_Send (a[j], local_cols, dtype, 0, 0, grid_comm);
		}
   }
   fclose(output_file);
   if (!grid_id)free (buffer);
}
double **alloc_2d_double(int r, int c)
{
	int i;
	double **matrix;
	size_t size = (size_t)r*c*sizeof(double);
	matrix = (double **)malloc(r*sizeof(double*));
	if(matrix == NULL)
	{
		printf("2d memory allocation failed\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	matrix[0] = (double *) malloc((size_t)r*c*sizeof(double));
	if(matrix[0] == NULL)
	{
		printf("memory allocation falied\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	for (i=1;i<r;i++)
		matrix[i] = matrix[0]+i*c;
	return matrix;
}
void submatmul(double **C, double **A, double **B, int Ar, int Ac, int Bc)
{
	int i,j,k;
	for(i=0;i<Ar;i++)
		for(j=0;j<Bc;j++)
			for(k=0;k<Ac;k++)
				C[i][j] += A[i][k] * B[k][j];
}
int main(int argc, char**argv)
{
	MPI_Init(&argc, &argv);
	int rank, size, grid_sz;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	char *filename1 = argc >= 2? strdup(argv[1]):"default-matrix-file1.dat",
		*filename2 = argc >= 3? strdup(argv[2]):"default-matrix-file2.dat",
		*filename3 = argc == 4? strdup(argv[3]):"output-matrix-file.dat";
	grid_sz = (int)sqrt(size);
	int Ar, Ac, Br, Bc, subAr, subAc, subBr, subBc, i;
	//create sub communicators
	int dimsizes[2], wraparound[2], coordinates[2], free_coords[2], reorder = 1, my_grid_rank;
	MPI_Comm grid_comm, row_comm, col_comm;
	dimsizes[0] = dimsizes[1] = grid_sz;
	wraparound[0] = wraparound[1] = 1;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dimsizes, wraparound, reorder, &grid_comm);
	MPI_Comm_rank(grid_comm, &my_grid_rank);
	MPI_Cart_coords(grid_comm, my_grid_rank, 2, coordinates);
	//-------------------------
	double **matrixA, *storageA, **matrixB, *storageB, **matrixC, **bufferA, **bufferB, **temp, time;
	read_checkerboard_matrix(filename1, (void***)&matrixA, (void**)&storageA, MPI_DOUBLE, &Ar, &Ac, grid_comm);
	read_checkerboard_matrix(filename2, (void***)&matrixB, (void**)&storageB, MPI_DOUBLE, &Br, &Bc, grid_comm);
	subAr = Ar/grid_sz;
	subAc = Ac/grid_sz;
	subBr = Br/grid_sz;
	subBc = Bc/grid_sz;
	//row communicator
	free_coords[0] = 0;
	free_coords[1] = 1;
	MPI_Cart_sub(grid_comm,free_coords, &row_comm);
	int row_rank, row_size;
	MPI_Comm_rank(row_comm, &row_rank);
	MPI_Comm_size(row_comm, &row_size);
	//-------------------------
	//column communicator
	free_coords[0] = 1;
	free_coords[1] = 0;
	MPI_Cart_sub(grid_comm, free_coords, &col_comm);
	int col_rank, col_size;
	MPI_Comm_rank(col_comm, &col_rank);
	MPI_Comm_size(col_comm, &col_size);
	//-------------------------
	int send_A_to, send_B_to, recv_A_from, recv_B_from;
	bufferA = alloc_2d_double(subAr, subAc);
	bufferB = alloc_2d_double(subBr, subBc);
	matrixC = alloc_2d_double(subAr, subBc);
	MPI_Barrier(MPI_COMM_WORLD);
	time = MPI_Wtime();
	//initial aligment
	if(coordinates[0])
	{
		recv_A_from = (row_rank+coordinates[0])%row_size;
		send_A_to = (row_rank-coordinates[0])<0?row_size+row_rank-coordinates[0]:row_rank-coordinates[0];
		MPI_Sendrecv(matrixA[0], subAr*subAc, MPI_DOUBLE, send_A_to, 0, bufferA[0], subAr*subAc, MPI_DOUBLE, recv_A_from, 0, row_comm, MPI_STATUS_IGNORE);
		temp = bufferA; bufferA = matrixA; matrixA = temp;
	}
	if(coordinates[1])
	{
		recv_B_from = (col_rank+coordinates[1])%col_size;
		send_B_to = (col_rank-coordinates[1])<0?col_size+col_rank-coordinates[1]:col_rank-coordinates[1];
		MPI_Sendrecv(matrixB[0], subBr*subBc, MPI_DOUBLE, send_B_to, 0, bufferB[0], subBr*subBc, MPI_DOUBLE, recv_B_from, 0, col_comm, MPI_STATUS_IGNORE);
		temp = bufferB; bufferB = matrixB; matrixB = temp;
	}
	//initialise C
	for(i=0;i<subAr*subBc;i++) matrixC[0][i]=0;
	//Cannon's Algorithm
	recv_A_from = (row_rank+1)%row_size;
	recv_B_from = (col_rank+1)%col_size;
	send_A_to = !row_rank?row_size-1:row_rank-1;
	send_B_to = !col_rank?col_size-1:col_rank-1;
	//MPI_Barrier(grid_comm);
	for(i=0;i<grid_sz;i++)
	{
		submatmul(matrixC, matrixA, matrixB, subAr, subAc, subBc);
		//Send A
		MPI_Sendrecv(matrixA[0], subAr*subAc, MPI_DOUBLE, send_A_to, 0, bufferA[0], subAr*subAc, MPI_DOUBLE, recv_A_from, 0, row_comm, MPI_STATUS_IGNORE);
		//Send B
		MPI_Sendrecv(matrixB[0], subBr*subBc, MPI_DOUBLE, send_B_to, 0, bufferB[0], subBr*subBc, MPI_DOUBLE, recv_B_from, 0, col_comm, MPI_STATUS_IGNORE);
		temp = bufferA; bufferA = matrixA; matrixA = temp;
		temp = bufferB; bufferB = matrixB; matrixB = temp;
	}
	//MPI_Barrier(MPI_COMM_WORLD);
	time = MPI_Wtime() - time;
	//stats-------------
	double min_time = 0, max_time = 0, avg_time = 0;
	MPI_Reduce(&time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&time, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	avg_time /= size;
	printf("rank: %d time: %lf\n", rank, time);
	if(!rank)
	{
		FILE * fp_stats;
		fp_stats = fopen("stats.txt","a+");
		if (fp_stats != NULL)
		{
			fprintf(fp_stats, "npes: %d, A:(%d x %d), B:(%d x %d), C:(%d x %d)\n", size, Ar, Ac, Br, Bc, Ar, Bc);
			fprintf(fp_stats, "time (min: %lf s, max: %lf s, avg: %lf s)\n\n\n", min_time, max_time, avg_time);
		}
	}
	write_matrix_data(filename3, (void**)matrixC, MPI_DOUBLE, Ar, Bc, grid_comm);
	//free data
	free(matrixA[0]);
	free(matrixB[0]);
	free(matrixC[0]);
	free(bufferA[0]);
	free(bufferB[0]);
	free(matrixA);
	free(matrixB);
	free(matrixC);
	free(bufferA);
	free(bufferB);
	MPI_Finalize();
	return EXIT_SUCCESS;
}