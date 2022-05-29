#include <stdio.h>

#include <papi.h>

#include "mpi.h"

static int rank = -1;
static int papi_error = 0;
static long_long ins_count = -1;
static FILE *log;

enum SimpiEvent { Error, Compute, Recv, Send, Bcast, Scatter, Gather };

void handle_papi_error(int retval) {
  if (retval == PAPI_OK) {
    return;
  }
  fprintf(stderr, "[%d] PAPI error %d: %s\n", rank, retval,
          PAPI_strerror(retval));
  fprintf(log, "%d %d %d\n", rank, Error, retval);
  papi_error = 1;
}

void papi_log_compute() {
  if (!papi_error) {
    long_long prev_count = ins_count;
    float rtime, ptime, ipc;
    handle_papi_error(PAPI_ipc(&rtime, &ptime, &ins_count, &ipc));
    if (!papi_error) {
      long_long diff = ins_count - prev_count;
      fprintf(stderr, "[%d] compute %lld\n", rank, diff);
      fprintf(log, "%d %d %lld\n", rank, Compute, diff);
    }
  }
}

int MPI_Init(int *argc, char ***argv) {
  int result = PMPI_Init(argc, argv);
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char logName[30];
  sprintf(logName, "./simpi-%d.log", rank);
  log = fopen(logName, "w+");

  float rtime, ptime, ipc;
  handle_papi_error(PAPI_ipc(&rtime, &ptime, &ins_count, &ipc));

  return result;
}

int MPI_Finalize() {
  papi_log_compute();

  fclose(log);
  fprintf(stderr, "[%d] wrapping up\n", rank);
  return PMPI_Finalize();
}

int MPI_Send(const void *buffer, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm) {
  papi_log_compute();

  int size;
  int result = PMPI_Send(buffer, count, datatype, dest, tag, comm);
  PMPI_Type_size(datatype, &size); /* Compute size */
  fprintf(stderr, "[%d] send %d to %d\n", rank, count * size, dest);
  fprintf(log, "%d %d %d %d\n", rank, Send, count * size, dest);

  return result;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status) {
  papi_log_compute();

  int actual_count, size;
  int result = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
  PMPI_Type_size(datatype, &size);                 /* Compute size */
  PMPI_Get_count(status, datatype, &actual_count); /* Compute count */

  fprintf(stderr, "[%d] recv %d from %d\n", rank, actual_count * size,
          status->MPI_SOURCE);
  fprintf(log, "%d %d %d %d\n", rank, Recv, actual_count * size,
          status->MPI_SOURCE);

  return result;
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root,
              MPI_Comm comm) {
  papi_log_compute();

  int size;
  int result = PMPI_Bcast(buffer, count, datatype, root, comm);
  PMPI_Type_size(datatype, &size); /* Compute size */

  fprintf(stderr, "[%d] bcast %d %d\n", rank, count * size, root);
  fprintf(log, "%d %d %d %d\n", rank, Bcast, count * size, root);

  return result;
}

int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                MPI_Comm comm) {
  papi_log_compute();

  int size;
  int result = PMPI_Scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                            recvtype, root, comm);
  PMPI_Type_size(sendtype, &size); /* Compute size */

  fprintf(stderr, "[%d] scatter %d %d\n", rank, sendcount * size, root);
  fprintf(log, "%d %d %d %d\n", rank, Scatter, sendcount * size, root);

  return result;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
               MPI_Comm comm) {
  papi_log_compute();

  int size;
  int result = PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                           recvtype, root, comm);
  PMPI_Type_size(sendtype, &size); /* Compute size */

  fprintf(stderr, "[%d] gather %d %d\n", rank, sendcount * size, root);
  fprintf(log, "%d %d %d %d\n", rank, Gather, sendcount * size, root);

  return result;
}
