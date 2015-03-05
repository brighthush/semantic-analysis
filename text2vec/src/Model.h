/*************************************************************************
	> File Name: Model.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Tue 06 Jan 2015 01:10:19 PM CST
 ************************************************************************/
#ifndef __MODEL__H__
#define __MODEL__H__

#include "glMember.h"
#include "Vocab.h"

extern Vocab vocabulary;
extern real *syn0, *syn1, *docvecs; 

void initNet();
void *trainModelThread(void *id);
void trainModel();

void writeWordvec(char *filePath);
void readWordvec(char *filePath);
void writeParameter(char *filePath);
void readParameter(char *filePath);
void writeDocvec(char *filePath);

#endif
