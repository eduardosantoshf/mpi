/**
 *  \file worker.h (interface file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Definition of the operations carried out by the workers:
 *     \li is_vowel
 *     \li is_consonant
 *     \li is_split
 *     \li processVal.
 * 
 *  \author Eduardo Santos and Pedro Bastos - May 2022
 */

#include "probConst.h"
#include "MessageStruct.h"

#ifndef WORKER
#define WORKER

/** \brief Process each chunk */
extern void processVal(MessageStruct * messageStruct);

/** \brief Check if character is a vowel */
extern int is_vowel(int char_value);

/** \brief Check if character is a consonant */
extern int is_consonant(int char_value);

/** \brief Check if character is a split char */
extern int is_split(int char_value);

#endif