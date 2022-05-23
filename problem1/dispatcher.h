#include "MessageStruct.h"

#ifndef DISPATCHER
#define DISPATCHER

extern void allocateMemory(char *filenames[], unsigned int numfiles);

extern int check_for_file();

extern void check_close_file();

int get_int(FILE *fp);

extern int getVal(MessageStruct *MessageStruct);

extern void save_file_results(MessageStruct *messageStruct);

extern void print_final_results();

#endif