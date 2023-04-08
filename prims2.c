#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int *MatrixChunk;
int *displs;
int *sendcounts;
struct Solution{
  int min;
  int rank;
  int v1;
  int v2;
};

int *MST;
FILE *f_matrix;
FILE *f_result;

int main(int argc, char *argv[]) {

  int minWeight, rank, numberOfProcessors;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);

  char processsor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processsor_name, &name_len);

  printf("hello from processor %s, rank %d out of %d processors\n", processsor_name, rank, numberOfProcessors);

  int amountOfVertices;
  if (rank == 0) {
    f_matrix = fopen("./Matrix.txt", "r");
    if (f_matrix) {
      fscanf(f_matrix, "%d\n", &amountOfVertices);
    }
    printf("numberOfProcessors is %d\n", amountOfVertices);
  }

  MPI_Bcast(&amountOfVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int i, j, k;

  displs = (int *)malloc(sizeof(int) * numberOfProcessors);
  sendcounts = (int *)malloc(sizeof(int) * numberOfProcessors);

  displs[0] = 0;
  sendcounts[0] = amountOfVertices / numberOfProcessors;
  int remains = numberOfProcessors -
                (amountOfVertices %
                 numberOfProcessors);
  for (i = 1; i < remains; ++i) {
    sendcounts[i] = sendcounts[0];
    displs[i] = displs[i - 1] + sendcounts[i - 1];
  }
  for (i = remains; i < numberOfProcessors; ++i) {
    sendcounts[i] = sendcounts[0] + 1;
    displs[i] = displs[i - 1] + sendcounts[i - 1];
  }

  int *matrix;
  if (rank == 0)
  {
    matrix = (int *)malloc(amountOfVertices * amountOfVertices * sizeof(int));
    for (i = 0; i < amountOfVertices; ++i) {
      matrix[amountOfVertices * i + j] = 0;
      for (j = i + 1; j < amountOfVertices; ++j) {
        int buf;
        fscanf(f_matrix, "%d\n", &buf);
        matrix[amountOfVertices * i + j] = buf;
        printf("matrix(%d,%d) is %d\n", i, j, matrix[amountOfVertices * i + j]);
      }
    }
    fclose(f_matrix);
  }

  MatrixChunk = (int *)calloc(sendcounts[rank] * amountOfVertices, sizeof(int));

  MPI_Datatype matrixString;
  MPI_Type_contiguous(amountOfVertices, MPI_INT, &matrixString);
  MPI_Type_commit(&matrixString);

  MPI_Datatype solution;
  MPI_Type_contiguous(4, MPI_INT, &solution);
  MPI_Type_commit(&solution);
  MPI_Scatterv(matrix, sendcounts, displs, matrixString, MatrixChunk,
               sendcounts[rank], matrixString, 0, MPI_COMM_WORLD);

  if (rank == 0)
  {
    for (int i = 0; i < numberOfProcessors; i++) {
      printf("displs[%d] = %d\n", i, displs[i]);
      printf("sendcounts[%d] = %d\n", i, sendcounts[i]);
    }
    printf("\n");
  }

  MST = (int *)calloc(
      amountOfVertices,
      sizeof(int));
  for (i = 0; i < amountOfVertices; ++i) {
    MST[i] = -1;
  }

  MST[0] = 0;
  minWeight = 0;

  int min;
  int v1 = 0;
  int v2 = 0;

  struct Solution sol;
  struct Solution result;
  for (k = 0; k < amountOfVertices - 1; ++k) {
    min = INT_MAX;
    for (i = 0; i < sendcounts[rank]; ++i) {
      if (MST[i + displs[rank]] != -1) {
        for (j = 0; j < amountOfVertices; ++j) {
          if (MST[j] == -1) {
            if (MatrixChunk[amountOfVertices * i + j] < min &&
                MatrixChunk[amountOfVertices * i + j] != 0) {
              min = MatrixChunk[amountOfVertices * i + j];
              v2 = j;
              v1 = i;
            }
          }
        }
      }
    }
    sol.min = abs(min);
    sol.rank = rank;
    sol.v1 = v1 + displs[rank];
    sol.v2 = v2;
    int done=numberOfProcessors-1;
    if(rank==0)
    {
      while(done>0)
      {
        MPI_Recv(&result,1,solution,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);
        if(sol.min>result.min)
          sol=result;
        done--;
      }
    }
    else
    {
      MPI_Send(&sol,1,solution,0,0,MPI_COMM_WORLD);
    }
    MPI_Bcast(&sol, 1, solution, 0, MPI_COMM_WORLD);

    MST[sol.v2] = sol.v1;
    minWeight += sol.min;
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0)
  {
    f_result = fopen("./Result.txt", "w");

    fprintf(f_result, "The min minWeight is %d\n ", minWeight);
    for (i = 0; i < amountOfVertices; ++i) {
      fprintf(f_result, "Edge %d %d\n", i, MST[i]);
    }
    fclose(f_result);
  }
  MPI_Type_free(&matrixString);
  MPI_Type_free(&solution);
  MPI_Finalize();
  return 0;
}
