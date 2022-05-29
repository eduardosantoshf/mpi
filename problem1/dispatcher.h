/**
 *  \file dispatcher.h (interface file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Definition of the operations carried out by the dispatcher:
 *     \li allocateMemory
 *     \li check_for_file
 *     \li check_close_file
 *     \li get_int
 *     \li getVal
 *     \li save_file_results
 *     \li print_final_results
 *
 *  \author Eduardo Santos and Pedro Bastos - May 2022
 */

#include "MessageStruct.h"

#ifndef DISPATCHER
#define DISPATCHER

/** \brief Allocate memory to save final results */
extern void allocateMemory(char *filenames[], unsigned int numfiles);

/** \brief open file if available */
extern int check_for_file();

/** \brief close file */
extern void check_close_file();

/** \brief convert char to integer */
int get_int(FILE *fp);

/** \brief read next chunk to send to workers */
extern int getVal(MessageStruct *MessageStruct);

/** \brief save partial results */
extern void save_file_results(MessageStruct *messageStruct);

/** \brief print final results */
extern void print_final_results();

#endif