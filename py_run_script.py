#python automation
import os
matrix_size = [2520*i for i in range(1,5)]
ranks = [i**2 for i in range(2,11)]
print(matrix_size)
print(ranks)
for i in matrix_size:
	cmd = "./make-matrix -r " + str(i) + " -c " + str(i)
	os.system(cmd)
	os.system("cp default-matrix-file1.dat default-matrix-file2.dat")
	for j in ranks:
		cmd = "mpirun -n " + str(j) + " ./mm-parallel > log_" + str(j) + "_" + str(i) 
		os.system(cmd)
os.system("rm default-matrix-file* output-matrix-file.dat")
os.system("mkdir log_files")
os.system("mv log_* log_files")