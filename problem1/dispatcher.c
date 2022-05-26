#include "MessageStruct.h"
#include "worker.h"
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "probConst.h"

char **file_names;

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

FILE *fp;

void allocateMemory(char *filenames[], unsigned int numfiles){

    file_names = filenames;
    num_files = numfiles;

    /* allocate the needed space in the arrays to save the results */
    array_num_words = (int *)malloc(num_files * sizeof(int));
    array_num_vowels = (int *)malloc(num_files * sizeof(int));
    array_num_cons = (int *)malloc(num_files * sizeof(int));

    //for(int i = 0; i < num_files; i++){
    //    array_num_words[i] = 1;
    //}

}

/** 
 *  \brief Wait for the workers to be ready and then open the next file.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param id worker identification
 *  \return 1 for Success and 0 for Failure.
 */

int check_for_file() {

  int flag_file = 1;
  index_file++;


  if (index_file < num_files) {                                                          /* Check if there are more files to be opened */
      open_file = 1;
      close_file = 0;
      fp = fopen(file_names[index_file], "r");

      if(fp == NULL){
            printf("ERROR: Unable to open the file: %s\n", file_names[index_file]);
            flag_file = 0;
            open_file = 0;
      }

      

  }

  else{
    flag_file = 0;
  }

  return flag_file;
}

/** 
 *  \brief Wait for the workers to be ready and then close the file.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param id worker identification
 */

void check_close_file() {

  if (!close_file) {                                            /* Check if the file is not already being closed and, if not, close it */
    fclose(fp);
    open_file = 0;
    close_file = 1;
  }
}

/** 
 *  \brief Calculate the next char and convert it to integer.
 *  
 *  Operation carried out by the workers.
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

int getVal(MessageStruct *MessageStruct){

    int available = 1;
    int bytes = 0;
    unsigned int ch_value;

    if(!open_file){
        available = check_for_file();
    }

    MessageStruct->file_index = index_file;
    //printf("index: %d \n", index_file);


    if(available){
        //printf("entrei no file \n");
        while (bytes != NUM_BYTES) {
            ch_value = get_int(fp);
            //printf("%d \n", ch_value);
            if(ch_value == -1){
                break;
            }
            //if(ch_value == 1){
            //    printf("entrou aqui???");
            //}
            MessageStruct->ch_values[bytes] = ch_value;
            bytes += 1;
        }

        while (!is_split(ch_value))
        {   
            ch_value = get_int(fp);
            //printf("%d \n", ch_value);
            if(ch_value == -1){
                break;
            }
            MessageStruct->ch_values[bytes] = ch_value;
            bytes += 1;
        }
        
        
    }

    MessageStruct->n_bytes_read = bytes;

    if(bytes == 0){
        check_close_file();
        return 0;
    }

    return available;

}

/**
 *  \brief Save resuts for a single file.
 *
 *  Operation carried out by the workers.
 *
 *  \param consId worker identification.
 */

void save_file_results(MessageStruct *messageStruct) {
    //printf("%d", messageStruct->file_index);
    //printf("words: %d \n", messageStruct->num_words);
    //printf("start w/ vowel: %d \n", messageStruct->num_words);
    //printf("end w/ cons: %d \n", messageStruct->num_words);
    //printf("\n");
    array_num_words[messageStruct->file_index] += messageStruct->num_words;
    array_num_vowels[messageStruct->file_index] += messageStruct->num_vowels;
    array_num_cons[messageStruct->file_index] += messageStruct->num_cons;
}

/**
 *  \brief print final results.
 *
 *  Operation carried out by the main thread.
 */

void print_final_results() {
  for (int i = 0; i < num_files; i++) {
    printf("File name: %s \n", file_names[i]);
    printf("Total number of words = %d \n", array_num_words[i]);
    printf("N. of words beginning with a vowel = %d \n", array_num_vowels[i]);
    printf("N. of words ending with a consonant = %d \n\n", array_num_cons[i]);
  }
}