CFLAGS = -O3 -lm
CC = gcc
MCC = mpicc

all: build

MM_PARALLEL_OBJS=mm-parallel.o MyMPI.o

build: mm-parallel make-matrix mm-serial print-matrix
mm-parallel: $(MM_PARALLEL_OBJS)
	$(MCC) $(CFLAGS) $(MM_PARALLEL_OBJS) -o $@

mm-parallel.o: mm-parallel.c
	$(MCC) $(CFLAGS) $< -c -o $@

MyMPI.o: MyMPI.c MyMPI.h
	$(MCC) $(CFLAGS) $< -c -o $@

make-matrix: make-matrix.o
	$(CC) $< -o $@

make-matrix.o: make-matrix.c
	$(CC) $(CFLAGS) $< -c -o $@

mm-serial: mm-serial.o
	$(CC) $< -o $@

mm-serial.o: mm-serial.c
	$(CC) $(CFLAGS) $< -c -o $@

print-matrix: print-matrix.o
	$(CC) $< -o $@

print-matrix.o: print-matrix.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf mm-parallel make-matrix mm-serial print-matrix core* *.o
