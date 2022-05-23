#include "probConst.h"
#include "MessageStruct.h"

#ifndef WORKER
#define WORKER

extern void processVal(MessageStruct * messageStruct);

extern int is_vowel(int char_value);
extern int is_consonant(int char_value);
extern int is_split(int char_value);

#endif