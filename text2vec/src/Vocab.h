/*************************************************************************
	> File Name: Vocab.h
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Mon 05 Jan 2015 12:01:49 PM CST
 ************************************************************************/

#ifndef __VOCAB__H__
#define __VOCAB__H__

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>
#include <map>

#include "VocabWord.h"

class Vocab
{
private:
    const static long long VOCAB_HASH_SIZE = 30000000;
    VocabWord *vocab;
    LL vocabMaxSize, vocabSize;
    int *vocabHash;
    LL trainWordCnt; // the number of word in train file

    std::vector<std::string> docs;
    std::map<std::string, int> docHash;
    int trainDocCnt;

public:
    Vocab()
    {
        vocabMaxSize = 1000, vocabSize = 0, trainWordCnt = 0;
        vocab = (VocabWord *)calloc(vocabMaxSize, sizeof(VocabWord));
        if(vocab == NULL) { printf("calloc vocab failed."); exit(-1); } 
        vocabHash = (int *)malloc(VOCAB_HASH_SIZE * sizeof(int));
        if(vocabHash == NULL) { printf("malloc vocabHash failed."); exit(-1); }
        for(LL i=0; i<VOCAB_HASH_SIZE; ++i) vocabHash[i] = -1;
        docs.clear(); docHash.clear(); trainDocCnt = 0;
    }

    int getWordHash(char *word);
    int searchVocab(char *word);
    int readWord2Index(FILE *fin);
    int addWord2Vocab(char *word);
    void addText2Vocab(char *text);
    void sortVocab();
    void createBinaryTree();
    void learnVocabFromTrainFile();

    LL getWordCnt(char *word);
    LL getWordCnt(LL wordIndex);

    LL get_vocabSize() { return vocabSize; }
    LL get_trainWordCnt() { return trainWordCnt; }
    int get_codelen(LL index) { return vocab[index].get_codelen(); }
    LL* get_point(LL index) { return vocab[index].get_point(); }
    char* get_code(LL index) { return vocab[index].get_code(); }
    char* get_word(LL index) { return vocab[index].get_word(); }

    // indexs need to be allocated before call this function
    int text2Index(char *text, int *&indexs, int &indexsSize);
    int getDocHash(std::string &docName);
    std::string getDocName(int index) { return docs[index]; }
    int addDoc(std::string &docName);
    int get_trainDocCnt() { return trainDocCnt; }
    int chage2Test(std::vector<std::string> &paths);
    
    void display();
};

#endif
