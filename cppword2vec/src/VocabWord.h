/*************************************************************************
	> File Name: VocabWord.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Mon 05 Jan 2015 09:47:17 AM CST
 ************************************************************************/

#ifndef __VOCAB_WORD__H__
#define __VOCAB_WORD__H__

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "glMember.h"

class VocabWord
{
private:
    long long cn;  //occurrence count for a word
    long long *point;
    char *word, *code, codelen;

public:
    VocabWord():cn(0), point(NULL), word(NULL), code(NULL), codelen(0) {}
    bool operator<(const VocabWord &b) const
    {
        if(cn < b.cn) return false;
        return true;
    }

    void set_cn(long long cn) { this->cn = cn; }
    long long get_cn() { return cn; }

    void set_word(char *word)
    {
        if(this->word!=NULL) free(this->word);
        this->word = (char *)calloc(strlen(word)+1, sizeof(char));
        strcpy(this->word, word);
    }
    char* get_word() { return word; }

    LL* calloc_point() { point = (LL *)calloc(MAX_CODE_LENGTH, sizeof(LL)); return point; }
    char* calloc_code() { code = (char *)calloc(MAX_CODE_LENGTH, sizeof(char)); return code; } 
    LL* get_point() { return point; }
    char* get_code() { return code; }

    void set_codelen(int codelen) { this->codelen = codelen; }
    int get_codelen() { return codelen; }

    VocabWord& operator++() { ++cn; return *this; }
};

#endif
