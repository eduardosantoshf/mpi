/**
 *  \file main.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  \author Eduardo Santos and Pedro Bastos - May 2022
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
#include <mpi.h>

#include "MessageStruct.h"
#include "dispatcher.h"
#include "worker.h"
#include "probConst.h"

/** \brief time limits */
struct timespec start, finish;

/** \brief number of workers */
int n_workers;

/**
 *  \brief dispatcher.
 *
 *  Its role is to simulate the lifecycle of the dispatcher, the root.
 *  It reads the file chunks, sends them to the workers and prints the final results.
 *
 *  \param file_names array with the file names.
 *  \param num_files number of files.
 */

void dispatcher(char *file_names[], unsigned int num_files) {

  int worker_id;
  int last_worker;

  /* structure to save file chunks */
  MessageStruct messageStruct;

  /* bool to indicate workers if is still work to be done */
  bool still_work = true;

  /* allocate memory */
  allocateMemory(file_names, num_files);

  clock_gettime (CLOCK_MONOTONIC_RAW, &start); 

  /* while there is data available to read */
  while(getVal((MessageStruct *) &messageStruct)) {

    /* signal workers that there is work to be done and send them the chunks */
    for(worker_id = 1; worker_id <= n_workers; worker_id++){
      
      /* save last worker to work */
      last_worker = worker_id;

      /* signal that there is work to be done */
      MPI_Send(&still_work, 1, MPI_C_BOOL, worker_id, 0, MPI_COMM_WORLD);

      /* send the chunks */
      MPI_Send(&messageStruct, sizeof(MessageStruct), MPI_BYTE, worker_id, 0, MPI_COMM_WORLD);


      if(worker_id < n_workers && !getVal((MessageStruct *) &messageStruct))
        break;
    }

    /* receive messages from workers and save partial results */
    for(worker_id = 1; worker_id <= last_worker; worker_id++){

      MPI_Recv(&messageStruct, sizeof(MessageStruct), MPI_BYTE, worker_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* save results of the chunk */
      save_file_results((MessageStruct *) &messageStruct);

    }

  }

  clock_gettime (CLOCK_MONOTONIC_RAW, &finish);

  still_work = false;

  /* signal workers that there is no more work to be done */
  for(int i = 1; i <= n_workers; i++){
    MPI_Send(&still_work, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD);
  }

  /* print final reults */
  print_final_results();

  /* print enlapsed time */
  printf ("\nElapsed time = %.6f s\n",  (finish.tv_sec - start.tv_sec) / 1.0 + (finish.tv_nsec - start.tv_nsec) / 1000000000.0);

}


/**
 *  \brief worker.
 *
 *  Its role is to simulate the lifecycle of the worker.
 *  It receives the file chunks, processes it and sends the results back to the dispatcher.
 *
 *  \param rank worker id.
 */

void worker(int rank){

  bool still_work;

  /* structure that contains chunk information */
  MessageStruct messageStruct;

  while (true) {

    /* receive the singal to check if there is still work */
    MPI_Recv(&still_work, 1, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /* if no work, return */
    if(!still_work){
      return;
    }

    /* else, work */
    else{
      /* receive the chunk */
      MPI_Recv(&messageStruct, sizeof(MessageStruct), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* process chunk */
      processVal((MessageStruct *) &messageStruct);

      /* send results */
      MPI_Send(&messageStruct, sizeof(MessageStruct), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

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
 *  Its role is starting the simulation by initializing the MPI, processing the command line and
 *  decide if the process is the root or a worker. 
 * 
 */

int main(int argc, char *argv[]) {
  
  int rank;
  int size;

  char **file_names;

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Fail if size <= 1 */
  if(size <= 1){
    if(rank == 0){
      printf("Wrong number of processes! It must be greater than 1. \n");
      MPI_Finalize();
      return EXIT_FAILURE;
    }
  }

  /* number of workers */
  n_workers = size - 1;

  /* if rank = 0 is root */
  if(rank == 0){
    
    /* allocate memory to save file names */
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

    /* run the dispatcher */
    dispatcher(file_names, argc - 2);

  }
  
  /* if rank > 0, is a worker */
  else{
    /* run the worker */
    worker(rank);
  }

  MPI_Finalize();
  return 0;
}

