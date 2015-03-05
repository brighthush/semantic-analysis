/*************************************************************************
	> File Name: glMember.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Sun 04 Jan 2015 03:57:26 PM CST
 ************************************************************************/

#ifndef __GL_MEMBER__H__
#define __GL_MEMBER__H__

#include <string>
#include <vector>
#include <cstdio>

//#define __DEBUG__INFO__

#define MAX_EXP              6
#define EXP_TABLE_SIZE       10000             //for computing sigmoid function

#define MAX_STRING           100               //max length for a word
#define MAX_SENTENCE_LENGTH  100               //max length for a sentence
#define MAX_CODE_LENGTH      40                //max height for a huffman tree

#define MAX_TEXT_LENGTH      100000             //max text length

typedef double real;
typedef long long LL;
typedef unsigned long long ULL;

// global variables
extern real* expTable;
extern int debug_mode;
extern long long file_size, min_count, word_count_actual;
extern char train_file[], output_file[];
extern char trainDataPath[], testDataPath[];
extern int cbow_flag, sg_flag, hs_flag, ns_flag;
extern int num_threads;

// model parameters
extern int vectorSize;
extern int docvecSize;
extern real learningRate;
extern LL window;
extern LL numIteration;

extern int glPredictStage;

// global functions
void initExpTable();
inline real getSigmoid(real &x)
{
    int index = (int)((x + MAX_EXP)*(EXP_TABLE_SIZE / 2.0 / MAX_EXP));
    return expTable[index];
}

void readWord(char *word, FILE *fin);

LL readFile(char *filePath, char *&content, LL &contentSize);
std::vector< std::string > readDir(char *dirPath);
int argPos(char *str, int argc, char **argv);

#endif
