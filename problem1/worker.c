/**
 *  \file worker.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *
 *  Definition of the operations carried out by the workers:
 *      \li is_vowel
 *      \li is_consonant
 *      \li is_split
 *      \li processVal.
 *
 *  \author Eduardo Santos and Pedro Bastos - May 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <libgen.h>
#include <string.h>

#include "probConst.h"
#include "MessageStruct.h"

/** \brief variables used in the computation of each file */
int flag = 0;
int value_before = 0;
int end_of_word = 0;
int end = 0;


/** 
 *  \brief Check if a given char is a vowel.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a vowel, 0 otherwise.
 */

int is_vowel(int char_value) {
  /* list of all the vowels values */
  int vowels[] = {97, 101, 105, 111, 117, 65, 69, 73, 79, 85, 224, 225, 226, 227, 232, 233, 234, 236, 237, 238,
                  242, 243, 244, 245, 249, 250, 192, 193, 194, 195, 200, 201, 202, 204, 205, 206, 210,
                  211, 212, 213, 217, 218, 219, 251};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(vowels) / sizeof(vowels[0]); i++)
    if (vowels[i] == char_value) {
      return 1;
    }

  return 0;
}

/** 
 *  \brief Check if a given char is a consonant.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a consonant, 0 otherwise.
 */

int is_consonant(int char_value) {
  /* list of all the consonants values */
  int consonants[] = {98, 99, 100, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 118, 119, 120, 121, 122,
                      66, 67, 68, 70, 71, 72, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90,
                      231, 199};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(consonants) / sizeof(consonants[0]); i++)
    if (consonants[i] == char_value)
      return 1;

  return 0;
}

/** 
 *  \brief Check if a given char is a split.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a split, 0 otherwise.
 */

int is_split(int char_value) {
  /* list of all the split char values */
  int splits[] = {32, 9, 10, 45, 34, 8220, 8221, 91, 93, 123, 125, 40, 41, 46, 44,
                  58, 59, 63, 33, 8211, 8212, 8230, 171, 187, 96};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(splits) / sizeof(splits[0]); i++)
    if (splits[i] == char_value)
      return 1;

  return 0;
}

/** 
 *  \brief Process each file chunk.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param messageStruct structure that contains the chunk information.
 */

void processVal(MessageStruct *messageStruct) {

    /* initialize struct variables */
    messageStruct->num_cons = 0;
    messageStruct->num_vowels = 0;
    messageStruct->num_words = 0;

    int ch_value = 0;

    for (int counter = 0; counter < messageStruct->n_bytes_read; counter++) {                                         

        /* get next char value */
        ch_value = messageStruct->ch_values[counter];

        /* if EOF */
        if (ch_value == -1) {
            break;
        }

        /* check if first char of file is vowel */
        if (flag == 0) {
            if (is_vowel(ch_value) == 1) {
                messageStruct->num_vowels += 1;
            }
            flag = 1;
        }

        /* check if is a lonely apostrophe to avoid counting as word */
        if (ch_value == 39 || ch_value == 8216 || ch_value == 8217) {
            if (is_split(value_before))
                continue;
        }

        /* if is split char */
        if (is_split(ch_value)) {

            /* check if previous char was a consonant */
            if (is_consonant(value_before))
                messageStruct->num_cons += 1;

            end_of_word = 1;

            /* avoid consequent split chars */
            if(!is_split(value_before) && value_before != 0)
                end = 1;

        }

        /* not a split chat */
        else{
            end = 0;

            /* check if is end of word to sum total words */
            if (end_of_word == 1) {

                end_of_word = 0;

                /* if first char of new word is vowel */
                if (is_vowel(ch_value) == 1){
                    messageStruct->num_vowels += 1;
                }
            }
        }

        /* if end of word */
        if(end == 1){
            messageStruct->num_words += 1;
            end = 0;
        }

        /* save previous char to check in next iteration */
        value_before = ch_value;
    }

    /* reset temporary variables */
    flag = 0;
    value_before = 0;
    end_of_word = 0;

}