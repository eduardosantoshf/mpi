/**
 *  \file main.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Eduardo Santos and Pedro Bastos - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h>

#include "probConst.h"

#include <mpi.h>
#include "MessageStruct.h"
#include "dispatcher.h"
#include "worker.h"

/** \brief time limits */
struct timespec start, finish;

int n_workers;






void dispatcher(char *file_names[], unsigned int num_files) {

  int worker_id;
  int last_worker;

  MessageStruct messageStruct;

  bool still_work = true;

  // TODO: calcular tempos

  allocateMemory(file_names, num_files);

  clock_gettime (CLOCK_MONOTONIC_RAW, &start); 

  while(getVal((MessageStruct *) &messageStruct)) {

    for(worker_id = 1; worker_id <= n_workers; worker_id++){

      last_worker = worker_id;

      MPI_Send(&still_work, 1, MPI_C_BOOL, worker_id, 0, MPI_COMM_WORLD);

      MPI_Send(&messageStruct, sizeof(MessageStruct), MPI_BYTE, worker_id, 0, MPI_COMM_WORLD);

      if(worker_id < n_workers && !getVal((MessageStruct *) &messageStruct))
        break;
    }

    for(worker_id = 1; worker_id <= last_worker; worker_id++){
      //printf("%d \n", worker_id);

      MPI_Recv(&messageStruct, sizeof(MessageStruct), MPI_BYTE, worker_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      save_file_results((MessageStruct *) &messageStruct);

    }

  }

  clock_gettime (CLOCK_MONOTONIC_RAW, &finish);

  still_work = false;

  for(int i = 1; i <= n_workers; i++){
    MPI_Send(&still_work, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD);
  }

  print_final_results();

  printf ("\nElapsed time = %.6f s\n",  (finish.tv_sec - start.tv_sec) / 1.0 + (finish.tv_nsec - start.tv_nsec) / 1000000000.0);

}

void worker(int rank){

  bool still_work;

  MessageStruct messageStruct;

  while (true) {

    MPI_Recv(&still_work, 1, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if(!still_work)
      return;

    MPI_Recv(&messageStruct, sizeof(MessageStruct), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    processVal((MessageStruct *) &messageStruct);

    MPI_Send(&messageStruct, sizeof(MessageStruct), MPI_BYTE, 0, 0, MPI_COMM_WORLD);

  }
  


}

/** \brief Prints command usage */
static void printUsage(char *cmdName)
{
  fprintf(stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
                  "  OPTIONS:\n"
                  "  -h      --- print this help\n"
                  "  -f      --- filename\n"
                  "  -n      --- positive number\n",
          cmdName);
}

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (workers),
 *  waiting for their termination and printing the final results for each file.
 */
int main(int argc, char *argv[]) {
  
  int rank;

  int size;

  char **file_names;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(size <= 1){
    if(rank == 0){
      printf("Wrong number of processes! It must be greater than 1. \n");
      MPI_Finalize();
      return EXIT_FAILURE;
    }
  }

  n_workers = size - 1;

  if(rank == 0){

    file_names = malloc((argc - 1) * sizeof(char *));

    int opt;                                                                                     /* selected option */
    char *fName = "no name";                                     /* file name (initialized to "no name" by default) */
    int value_opt = -1;                                             /* numeric value (initialized to -1 by default) */

      /* Handle command line options */
    do {
      switch ((opt = getopt(argc, argv, "f:n:h"))) {
        case 'f':                                                                                      /* file name */
          if (optarg[0] == '-') {
            fprintf(stderr, "%s: file name is missing\n", basename(argv[0]));
            printUsage(basename(argv[0]));
            return EXIT_FAILURE;
          }
          fName = optarg;
          break;

        case 'n':                                                                               /* numeric argument */
          if (atoi(optarg) <= 0) {
            fprintf(stderr, "%s: non positive number\n", basename(argv[0]));
            printUsage(basename(argv[0]));
            return EXIT_FAILURE;
          }
          value_opt = (int)atoi(optarg);
          break;

        case 'h':                                                                                      /* help mode */
          printUsage(basename(argv[0]));
          return EXIT_SUCCESS;

        case '?':                                                                                 /* invalid option */
          fprintf(stderr, "%s: invalid option\n", basename(argv[0]));
          printUsage(basename(argv[0]));
          return EXIT_FAILURE;

        case -1:
          break;

      }

    } while (opt != -1);

    if (argc == 1) {
      fprintf(stderr, "%s: invalid format\n", basename(argv[0]));
      printUsage(basename(argv[0]));
      return EXIT_FAILURE;
    }


    /* Save filenames */
    for (int i = 2; i < argc; i++){
      file_names[i - 2] = argv[i];
    }

    dispatcher(file_names, argc - 2);

  }else{

    worker(rank);
  }

  MPI_Finalize();
  return 0;




}

