/**
 *  \file dispatcher.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *
 *  Definition of the operations carried out by the dispatcher:
 *     \li allocateMemory
 *     \li check_for_file
 *     \li check_close_file
 *     \li get_int
 *     \li getVal
 *     \li save_file_results
 *     \li print_final_results.
 *
 *  \author Eduardo Santos and Pedro Bastos - May 2022
 */

#include "MessageStruct.h"
#include "worker.h"
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "probConst.h"

/** \brief pointer to save the filenames */
char **file_names;

/** \brief number of files */
int num_files;

/** \brief array to save the total number of words for each file */
int *array_num_words;

/** \brief array to save the number of words beginning with a vowel for each file */
int *array_num_vowels;

/** \brief array to save the number of words ending with a consonant for each file */
int *array_num_cons;

/** \brief variable to save the current file index */
int index_file = -1;

/** \brief flag that indicates if the file was already opened */
int open_file = 0;

/** \brief flag that indicates if the file was already closed */
int close_file = 0;

/** \brief pointer to the file */
FILE *fp;


/** 
 *  \brief Allocate memory to save the results.
 *  
 *  Operation carried out by the dispatcher.
 * 
 *  \param filenames array with the file names
 *  \param numFiles number of files
 * 
 */

void allocateMemory(char *filenames[], unsigned int numfiles){

    file_names = filenames;
    num_files = numfiles;

    /* allocate the needed space in the arrays to save the results */
    array_num_words = (int *)malloc(num_files * sizeof(int));
    array_num_vowels = (int *)malloc(num_files * sizeof(int));
    array_num_cons = (int *)malloc(num_files * sizeof(int));

    for(int i = 0; i < num_files; i++){
        array_num_words[i] = 0;
        array_num_vowels[i] = 0;
        array_num_cons[i] = 0;
    }

}

/** 
 *  \brief Open the next file, if available.
 *  
 *  Operation carried out by the dispatcher.
 * 
 *  \return 1 for Success and 0 for Failure.
 */

int check_for_file() {

  int flag_file = 1;
  index_file++;

  /* if there is still files to open */
  if (index_file < num_files) {
      close_file = 0;
      open_file = 1;
      fp = fopen(file_names[index_file], "r");
  }
  else{
    flag_file = 0;
  }

  return flag_file;
}

/** 
 *  \brief close the file.
 *  
 *  Operation carried out by the dispatcher.
 * 
 */

void check_close_file() {
  if (!close_file) {                                            
    fclose(fp);
    close_file = 1;
    open_file = 0;
  }
}

/** 
 *  \brief Calculate the next char and convert it to integer.
 *  
 *  Operation carried out by the dispatcher.
 * 
 *  \param fp pointer to file.
 *  \return value.
 */

int get_int(FILE *fp) {

  int ch_value = fgetc(fp);
  int b = 0;

  if (ch_value == -1) /* if EOF */
    return -1;

  if ((ch_value & 128) == 0) { /* if is only 1 byte char, return it */
    return ch_value;
  }

  /* if contains 226 ('e2'), then it is a 3 byte char */
  if (ch_value == 226) { 
    b = 3;
    ch_value = ch_value & (1 << 4) - 1;
  }

  /* else, is a 2 byte char */
  else {
    b = 2;
    ch_value = ch_value & (1 << 5) - 1;
  }

  /* go through number of the char bytes */
  for (int x = 1; x < b; x++) {

    /* get next byte */
    int next_ch_value = fgetc(fp);

    /* if EOF */
    if (next_ch_value == -1)
      return -1;

    /* calculate int value of the char */
    ch_value = (ch_value << 6) | (next_ch_value & 63);
  }

  return ch_value;
}

/** 
 *  \brief open each file and read next chunk.
 *  
 *  Operation carried out by the dispatcher.
 * 
 *  \param MessageStuct message struct to save the chunk information.
 *  \return 1 if still data to read, 0 otherwise.
 */

int getVal(MessageStruct *MessageStruct){

    int available = 1;
    int bytes = 0;
    unsigned int ch_value;

    /* if file not opened */
    if(!open_file){
        available = check_for_file();
    }

    /* if file available */
    if(available){

        while (bytes != NUM_BYTES) {

            ch_value = get_int(fp);

            /* if EOF */
            if(ch_value == -1){
                break;
            }

            /* save next byte in the structure */
            MessageStruct->ch_values[bytes] = ch_value;
            bytes += 1;
        }

        /* avoid ending in the middle of a word */
        while (!is_split(ch_value)) {   

            ch_value = get_int(fp);

            /* if EOF */
            if(ch_value == -1){
                break;
            }

            /* save next byte in the structure */
            MessageStruct->ch_values[bytes] = ch_value;
            bytes += 1;
        }
    }

    /* save file index in the structure */
    MessageStruct->file_index = index_file;

    /* save number of bytes read */
    MessageStruct->n_bytes_read = bytes;

    /* if 0 bytes are read, is EOF */
    if(bytes == 0){
        check_close_file();
    }

    return available;
}

/**
 *  \brief Save partial results of each chunk.
 *
 *  Operation carried out by the dispatcher.
 *
 *  \param messageStruct message structure to save.
 */

void save_file_results(MessageStruct *messageStruct) {
    array_num_words[messageStruct->file_index] += messageStruct->num_words;
    array_num_vowels[messageStruct->file_index] += messageStruct->num_vowels;
    array_num_cons[messageStruct->file_index] += messageStruct->num_cons;
}

/**
 *  \brief print final results.
 *
 *  Operation carried out by the dispatcher.
 */

void print_final_results() {
  for (int i = 0; i < num_files; i++) {
    printf("File name: %s \n", file_names[i]);
    printf("Total number of words = %d \n", array_num_words[i]);
    printf("N. of words beginning with a vowel = %d \n", array_num_vowels[i]);
    printf("N. of words ending with a consonant = %d \n\n", array_num_cons[i]);
  }
}