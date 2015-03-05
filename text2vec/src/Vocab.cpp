/*************************************************************************
	> File Name: Vocab.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Mon 05 Jan 2015 02:33:31 PM CST
 ************************************************************************/

#include "Vocab.h"

#include <iostream>
#include <cstring>
#include <algorithm>

#include "glMember.h"

using namespace std;

int Vocab::searchVocab(char *word)
{
    int hash = getWordHash(word);
    while(true)
    {
        if(vocabHash[hash]==-1) return -1;
        if(!strcmp(word, vocab[vocabHash[hash]].get_word())) return vocabHash[hash];
        hash = (hash+1)%VOCAB_HASH_SIZE;
    }
    return -1;
}

int Vocab::getWordHash(char *word)
{
    unsigned long long  hash=0;
    for(int i=0; i<strlen(word); ++i) { hash = hash*257 + word[i]; }
    hash %= VOCAB_HASH_SIZE;
    return hash;
}

int Vocab::readWord2Index(FILE *fin)
{
    char word[MAX_STRING];
    readWord(word, fin);
    if(feof(fin)) return -1;
    return searchVocab(word);
}

int Vocab::addWord2Vocab(char *word)
{
    int hash;
    vocab[vocabSize].set_word(word);
    vocab[vocabSize].set_cn(0);
    ++vocabSize;
    // realloc memory for vocab if needed
    if(vocabSize+1 >= vocabMaxSize)
    {
        vocabMaxSize += 1000;
        vocab = (VocabWord *)realloc(vocab, vocabMaxSize*sizeof(VocabWord));
    }
    hash = getWordHash(word);
    while(vocabHash[hash] != -1) hash = (hash+1)%VOCAB_HASH_SIZE;
    vocabHash[hash] = vocabSize - 1;
    return vocabSize - 1;
}

void Vocab::addText2Vocab(char *text)
{
    LL size = strlen(text);
    printf("text size %lld\n", size);
    char *word = (char *)malloc(sizeof(char) * MAX_STRING);
    if(word == NULL) { 
        printf("allocate memory for word failed in Vocab::addText2Vocab.\n");
        exit(-1);
    }
    LL i=0, j;
    while(i < size)
    {
        while(i<size && (text[i] == ' ' || text[i] == '\n')) ++i;
        j = 0;
        while(i<size && !(text[i]==' ' || text[i]=='\n')) {
            word[j++] = text[i];
            ++i;
        }
        if(j != 0) {
            word[j] = '\0';
            j = searchVocab(word);
            if(j == -1) j = addWord2Vocab(word);
            ++vocab[j];
        }
    }
    free(word);
}

int Vocab::text2Index(char *text, int *&indexs, int &indexsSize)
{
    LL size=strlen(text), i=0, j;
    char *word = (char *)malloc(sizeof(char) * MAX_STRING);
    int wordCount = 0;
    while(i < size)
    {
        while(i<size && (text[i] == ' ' || text[i] == '\n')) ++i;
        j = 0;
        while(i<size && !(text[i]==' ' || text[i]=='\n')) {
            word[j++] = text[i];
            ++i;
        }
        if(j != 0) {
            word[j] = '\0';
            j = searchVocab(word);
            if(wordCount + 2 >= indexsSize)
            {
                indexsSize += 1000;
                indexs = (int *)realloc(indexs, indexsSize * sizeof(int));
                if(indexs == NULL) { printf("realloc indexs in text2Index failed.\n"); exit(-1); }
            }
            if(j != -1)
                indexs[wordCount++] = j;
        }
    }
    free(word);
    return wordCount;
}

void Vocab::sortVocab()
{
    sort(vocab+2, vocab+vocabSize);
    for(int i=0; i<VOCAB_HASH_SIZE; ++i) vocabHash[i] = -1;
    trainWordCnt = 0;
    int n = vocabSize;
    for(int i=0; i<n; ++i)
    {
        if(vocab[i].get_cn() < min_count) { --vocabSize; free(vocab[i].get_word()); }
        else
        {
            LL hash = getWordHash(vocab[i].get_word());
            while(vocabHash[hash]!=-1) hash = (hash+1)%VOCAB_HASH_SIZE;
            vocabHash[hash] = i;
            trainWordCnt += vocab[i].get_cn();
        }
    }
    vocab = (VocabWord *)realloc(vocab, (vocabSize+10)*sizeof(VocabWord));
    for(int i=0; i<vocabSize; ++i)
    {
        vocab[i].calloc_code();
        vocab[i].calloc_point();
    }
}

void Vocab::createBinaryTree()
{
    long long point[MAX_CODE_LENGTH];
    char code[MAX_CODE_LENGTH];
    long long *count = (long long *)calloc((vocabSize<<1)+1, sizeof(long long));
    char *binary = (char *)calloc((vocabSize<<1)+1, sizeof(char));
    long long *parent = (long long *)calloc((vocabSize<<1)+1, sizeof(long long));
    for(LL i=0; i<vocabSize; ++i) count[i] = vocab[i].get_cn();
    for(LL i=vocabSize; i<(vocabSize<<1); ++i) count[i] = 1e15;
    // begin construct huffman tree
    LL min1, min2, pos1=vocabSize-1, pos2=vocabSize;
    for(LL i=0; i<vocabSize-1; ++i)
    {
        if(pos1>=0)
        {
            if(count[pos1]<count[pos2]) { min1 = pos1, --pos1; }
            else { min1 = pos2, ++pos2; }
        }
        else { min1 = pos2, ++pos2; }
        if(pos1>=0)
        {
            if(count[pos1]<count[pos2]) { min2 = pos1, --pos1; }
            else { min2 = pos2, ++pos2; }
        }
        else { min2=pos2, ++pos2; }
        count[vocabSize+i] = count[min1] + count[min2];
        parent[min1] = vocabSize + i;
        parent[min2] = vocabSize + i;
        binary[min2] = 1;
#ifdef __DEBUG__INFO__
        printf("Huffman min1:(%lld, %lld, %d) min2:(%lld, %d, %d) parent:(%lld, %lld)\n", \
                min1, count[min1], binary[min1], min2, count[min2], binary[min2], \
                vocabSize+i, count[vocabSize+i]);
#endif
    }
    //assign path for root to leaf for each word
    for(LL i=0; i<vocabSize; ++i)
    {
        LL cur = i;
        char codeLength = 0;
        LL root = (vocabSize<<1) - 2;
        while(cur!=root)
        {
            code[codeLength] = binary[cur];
            point[codeLength] = cur;
            cur = parent[cur];
            ++codeLength;
        }
        // update code, point and codelen in vocab
        vocab[i].set_codelen(codeLength);
        LL *vocab_point = vocab[i].get_point();
        char *vocab_code = vocab[i].get_code();
        vocab_point[0] = (vocabSize<<1) - 2 - vocabSize;
        for(int j=0; j<codeLength; ++j)
        {
            vocab_code[codeLength-j-1] = code[j];
            vocab_point[codeLength-j] = point[j] - vocabSize;
        }
#ifdef DEBUG_INFO
        printf("code from root to leaf %lld, codelen %d\n", i, vocab[i].get_codelen());
        for(int j=0; j<codeLength; ++j)
            printf("(%lld, %d) ", vocab_point[j], vocab_code[j]);
        printf("\n");
#endif
    }
    free(count);
    free(binary);
    free(parent);
}

void Vocab::learnVocabFromTrainFile()
{
    char word[MAX_STRING];
    FILE *fin;
    for(int i=0; i<VOCAB_HASH_SIZE; ++i) vocabHash[i] = -1;
    fin = fopen(train_file, "rb");
    if(fin == NULL) { printf("open training file filed."); exit(-1); }
    //begin to read word into vocab array
    vocabSize = 0;
    addWord2Vocab((char *)"</s>");
    while(true)
    {
        readWord(word, fin);
        if(feof(fin)) break;
        ++trainWordCnt;
        if(debug_mode && (trainWordCnt%100000==0))
        {
            printf("%lldK%c", trainWordCnt/1000, 13); fflush(stdout);
        }
        int index = searchVocab(word);
        if(index==-1) index = addWord2Vocab(word);
        ++vocab[index];
#ifdef __DEBUG__INFO__
        printf("%s %d\n", vocab[index].get_word(), vocab[index].get_cn());
#endif
        //To Do
        //if(vocabSize > vocabHashSize*0.7) reduceVocab();
    }
    sortVocab();
    if(debug_mode)
    {
        printf("Vocab size: %lld\n", vocabSize);
        printf("train words in training file: %lld\n", trainWordCnt);
    }
    file_size = ftell(fin);
    fclose(fin);
}

LL Vocab::getWordCnt(char *word)
{
    LL wordIndex = searchVocab(word);
    return getWordCnt(wordIndex);
}

LL Vocab::getWordCnt(LL wordIndex)
{
    if(wordIndex < 0) return 0;
    else if(wordIndex > vocabSize) return 0;
    else return vocab[wordIndex].get_cn();
}


int Vocab::getDocHash(string &docName)
{
    if(docHash.find(docName) == docHash.end()) return -1;
    else return docHash[docName];
}

int Vocab::addDoc(string &docName)
{
    int docNameHash = getDocHash(docName);
    if(docNameHash == -1) { docs.push_back(docName); docHash[docName] = trainDocCnt++; }
    return getDocHash(docName);
}

int Vocab::chage2Test(vector<string> &paths)
{
    docs.clear(); docHash.clear(); trainDocCnt = 0;
    for(int i=0; i<paths.size(); ++i)
        addDoc(paths[i]);
    return trainDocCnt;
}

void Vocab::display()
{
    printf(".......display vocab info......\n");
    for(LL i=0; i<vocabSize; ++i)
    {
        printf("%s: %d\n", vocab[i].get_word(), vocab[i].get_cn());
    }
}
