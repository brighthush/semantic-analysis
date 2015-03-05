/*************************************************************************
	> File Name: WordTable.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Thu 08 Jan 2015 04:24:46 PM CST
 ************************************************************************/
#ifndef __WORD__TABLE__H__
#define __WORD__TABLE__H__

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "glMember.h"

class WordTable
{
private:
    std::map<std::string, int> word2Index;
    std::string *index2Word;
    real *table;
    LL vocabSize, vectorSize;
public:
    WordTable() : table(NULL), vocabSize(0), vectorSize(0) { word2Index.clear(); }
    void readTableFile(char *filePath);
    std::vector<std::string> readInput();
    int getWordIndex(std::string word);
    real cosine(int a, int b);
    real cosine(int a, real *vec);
    std::vector< std::pair<LL, real> > sim2vec(real *vec, const int &k=40, \
            LL *ignore=NULL, int n=0);
    std::vector< std::pair<LL, real> > getKNear(std::vector<std::string> &words, const int &k=40);
    std::vector< std::pair<LL, real> > analogy(std::vector<std::string> &words, const int &k=20);

    std::string get_word(LL index) { return index2Word[index]; }
};

#endif

