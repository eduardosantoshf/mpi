/**
 *  \file probConst.h (interface file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Structure to save file chunks and partial results.
 *
 *  \author Eduardo Santos and Pedro Bastos - May 2022
 */

#include "probConst.h"

#ifndef MESSAGESTRUCT_H_
#define MESSAGESTRUCT_H_

/** \brief  structure */
typedef struct{
    int file_index;
    int n_bytes_read;
    int num_words;
    int num_vowels;
    int num_cons;
    unsigned int ch_values[NUM_BYTES + 10];

} MessageStruct;

#endif