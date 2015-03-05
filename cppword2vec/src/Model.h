/*************************************************************************
	> File Name: Model.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Tue 06 Jan 2015 01:10:19 PM CST
 ************************************************************************/
#ifndef __MODEL__H__
#define __MODEL__H__

#include "glMember.h"

extern real *syn0, *syn1; 

void initNet();
void *trainModelThread(void *id);
void trainModel();

#endif
