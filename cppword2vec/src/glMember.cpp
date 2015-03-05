/*************************************************************************
	> File Name: glMember.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Sun 04 Jan 2015 03:59:40 PM CST
 ************************************************************************/

#include "glMember.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

using namespace std;

real* expTable;

int debug_mode = 1;
long long file_size, min_count=0, word_count_actual = 0;
char train_file[MAX_STRING], output_file[MAX_STRING];
// continuous bag of words, skip-gram, hierarchical softmax, negative sampling
int cbow_flag = 1, sg_flag = 0, hs_flag = 1, ns_flag = 0;
int num_threads = 20;

// model parameters
int vectorSize = 100;
real learningRate = 0.025;
LL window = 5;
LL numIteration = 15;

void initExpTable()
{
    expTable = (real *)malloc((EXP_TABLE_SIZE+1)*sizeof(real));
    if(expTable == NULL)
    {
        cout << "[error] malloc expTable failed." << endl;
        exit(-1);
    }
    for(int i=0; i<=EXP_TABLE_SIZE; ++i)//project i into interval[-MAX_EXP, MAX_EXP]
    {
        expTable[i] = exp((i/(real)EXP_TABLE_SIZE*2.0 - 1.0)*MAX_EXP);
        expTable[i] = expTable[i]/(1+expTable[i]);//sigmoid(x) = 1/(1+exp(-x)) = exp(-x)/(1+exp(-x))
    }
}

inline bool isDelimiter(const char &ch)
{
    if(ch==' ' || ch=='\t' || ch=='\n') return true;
    return false;
}

void readWord(char *word, FILE *fin)
{
    int a = 0, ch;
    while(!feof(fin))
    {
        ch = fgetc(fin);
        if(ch == 13) continue; //ch is a cr
        if(isDelimiter(ch))
        {
            if(a>0)
            {
                if(ch=='\n') ungetc(ch, fin);
                break;
            }
            else
            {
                if(ch=='\n')
                {
                    strcpy(word, (char*)"</s>");
                    return;
                }
                else continue ;
            }
        }
        word[a++] = ch;
        if(a>=MAX_STRING-1) --a;   //if word too long, only keep the first MAX_STRING-2 and the last charcter
    }
    word[a] = 0; //set the end to be '\0'
}


