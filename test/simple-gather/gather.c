#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mpi.h"

int main(int argc, char *argv[]) {
  // Initial Setup
  int rank, size;
  double startWTime, endWTime;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc != 2) {
    printf("usage: %s size_of_D_in_kB\n", argv[0]);
    return 1;
  }

  startWTime = MPI_Wtime();

  // Obtain value for D from arguments
  const int D = atoi(argv[1]) * 1024;
  const int arrSize = D / sizeof(double);
  // Data Array
  // Size = D bytes
  double message[arrSize];

  // seed rng and populate array
  srand(time(NULL));
  for (int i = 0; i < arrSize; i++) {
    message[i] = (double)rand() / (double)RAND_MAX;
  }

  double localMessage[arrSize * size];
  // Send message
  MPI_Gather(message, arrSize, MPI_DOUBLE, localMessage, arrSize, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  endWTime = MPI_Wtime();

  double timeDelta[1], maxTime[1];
  timeDelta[0] = endWTime - startWTime;
  MPI_Reduce(timeDelta, maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // Wrapup
  MPI_Finalize();

  if (rank == 0) {
    // Print runtime of master node
    printf("Runtime = %f\n", maxTime[0]);
  }
  return 0;
}