/*************************************************************************
	> File Name: glMember.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Sun 04 Jan 2015 03:59:40 PM CST
 ************************************************************************/

#include "glMember.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <dirent.h>

using namespace std;

real* expTable;

int debug_mode = 1;
long long file_size, min_count=0, word_count_actual = 0;
char train_file[MAX_STRING], output_file[MAX_STRING];
char trainDataPath[MAX_STRING], testDataPath[MAX_STRING];
// continuous bag of words, skip-gram, hierarchical softmax, negative sampling
int cbow_flag = 1, sg_flag = 0, hs_flag = 1, ns_flag = 0;
int num_threads = 20;

// model parameters
int vectorSize = 150, docvecSize = 600;
real learningRate = 0.025;
LL window = 4;
LL numIteration = 15;

int glPredictStage = 0;

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

LL readFile(char *filePath, char *&content, LL &contentSize)
{
    FILE *pf = NULL;
    pf = fopen(filePath, "rb");
    if(pf == NULL) { printf("open file %s failed.\n", filePath); exit(-1); }
    fseek(pf, 0, SEEK_END);
    LL length = ftell(pf);
    if(length+1 >= contentSize) { contentSize = length + 1000; content = (char *)realloc(content, contentSize); }
    if(content == NULL) { printf("realloc for content faild.\n"); exit(-1); }
    rewind(pf);
    length = fread(content, 1, length, pf);
    content[length] = '\0';
    fclose(pf);
    return length;
}

vector<string> readDir(char *dirPath)
{
    vector<string> paths;
    struct dirent *pdirent;
    DIR *dir;
    dir = opendir(dirPath);
    if(dir == NULL) { printf("open dir %s failed.\n", dirPath); exit(-1); }
    string path = string(dirPath);
    if(path[path.size() - 1] != '/') path += string(1, '/');
    while((pdirent = readdir(dir)) != NULL) {
        if(pdirent->d_name[0] == '.') continue;
        //printf("%s\n", pdirent->d_name);
        string temp = path + string(pdirent->d_name);
        paths.push_back(temp);
    }
    closedir(dir);
    return paths;
}

int argPos(char *str, int argc, char **argv)
{
    int pos = -1;
    for(int i=1; i<argc; ++i)
        if(strcmp(str, argv[i]) == 0) { pos = i; break; }
    if(pos == argc-1) { 
        printf("argument missing for %s\n", str);
        exit(-1);
    }
    return pos;
}

