#include "probConst.h"

#ifndef MESSAGESTRUCT_H_
#define MESSAGESTRUCT_H_

typedef struct{
    int file_index;
    int n_bytes_read;
    int num_words;
    int num_vowels;
    int num_cons;
    unsigned int ch_values[NUM_BYTES];

} MessageStruct;

#endif