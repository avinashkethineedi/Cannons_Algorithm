#!/bin/bash
#PBS -N cannons_algorithm
#PBS -l select=25:ncpus=4:mem=40gb:mpiprocs=4:interconnect=fdr
#PBS -l walltime=20:00:00
#PBS -j oe
module purge
module load openmpi/4.1.3-gcc/9.5.0-ucx
cd $PBS_O_WORKDIR
make clean;make
python3 py_run_script.py