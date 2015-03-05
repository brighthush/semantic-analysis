/*************************************************************************
	> File Name: WordTable.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Thu 08 Jan 2015 04:32:31 PM CST
 ************************************************************************/

#include "WordTable.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

using namespace std;

void WordTable::readTableFile(char *filePath)
{
    char *word = NULL;
    word = (char *)malloc(MAX_STRING * sizeof(char));
    if(word==NULL) { printf("allocate memory for word failed.\n"); exit(-1); }
    FILE *fi = fopen(filePath, "rb");
    if(fi==NULL) { printf("open file %s failed.\n", filePath); exit(-1); }
    fscanf(fi, "%lld", &vocabSize); fscanf(fi, "%lld", &vectorSize);
    printf("vocabSize: %lld, vectorSize: %lld\n", vocabSize, vectorSize);
    index2Word = new string[vocabSize];
    if(index2Word==NULL) { printf("allocate memory for index2Word failed.\n"); exit(-1); }
    table = (real *)malloc((LL)sizeof(real) * vocabSize * vectorSize);
    if(table==NULL) { printf("allocate memory for table failed.\n"); exit(-1); }
    for(LL i=0; i<vocabSize; ++i)
    {
        LL j=0;
        while(true)
        {
            word[j] = fgetc(fi);
            if(feof(fi) || word[j]=='\t' || word[j]==' ') break;
            if(j<MAX_STRING-1 && (word[j]!='\n')) ++j;
        }
        word[j] = 0;
        index2Word[i] = string(word);
        word2Index[string(word)] = i;
        for(j=0; j<vectorSize; ++j) fscanf(fi, "%lf", &table[i*vectorSize + j]);
        real sum = 0;
        for(j=0; j<vectorSize; ++j) sum += table[i*vectorSize+j] * table[i*vectorSize+j];
        sum = sqrt(sum);
        if(sum == 0.0) continue;
        for(j=0; j<vectorSize; ++j) table[i*vectorSize+j] /= sum;
        if(i%1000 == 0) printf("read %lld word_vectors\n", i);
    }
    fclose(fi);
}

vector<string> WordTable::readInput()
{
    vector<string> words;
    printf("please input word or sentence (EXIT to break): ");
    char *word = (char *)malloc(MAX_STRING * sizeof(char));
    if(word==NULL) { printf("allocate memory for word failed.\n"); exit(-1); }
    void *flag = gets(word);
    if(flag==NULL) { printf("gets(word) failed.\n"); exit(-1); }
    if(!strcmp(word, "EXIT")) { words.push_back(string("EXIT")); return words; }
    int i=0, j=0;
    while(true)
    {
        if(word[j] == ' ' || word[j] == 0)
        {
            words.push_back(string(&word[i], j-i));
            i = j + 1;
            if(word[j]==0) break;
        }
        ++j;
    }
    free(word);
    return words;
}

int WordTable::getWordIndex(string word)
{
    if(word2Index.find(word) == word2Index.end()) return -1;
    return word2Index[word];
}

real WordTable::cosine(int a, int b)
{
    real sum = 0;
    for(LL i=0; i<vectorSize; ++i)
        sum += (table[a * vectorSize + i] * table[b * vectorSize + i]);
    return sum;
}

real WordTable::cosine(int index, real *vec)
{
    real sim = 0;
    for(LL i=0; i<vectorSize; ++i) sim += (vec[i] * table[index * vectorSize + i]);
    return sim;
}

vector< pair<LL, real> > WordTable::sim2vec(real *vec, const int &k, \
        LL *ignore, int n) {
    LL *point = (LL *)malloc(sizeof(LL) * (LL)k);
    real *sim = (real *)malloc(sizeof(real) * (LL)k);
    memset(sim, 0, sizeof(real) * k);
    for(LL i=0; i<vocabSize; ++i) {
        bool flag = false;
        for(int j=0; j<n; ++j) if(i == ignore[j]) { flag = true; break; }
        if(flag) continue;
        real similarity = cosine(i, vec);
        int pos;
        for(pos=0; pos<k; ++pos) if(similarity > sim[pos]) break;
        if(pos >= k) continue;
        for(int j=k-1; j>pos; --j) {
            sim[j] = sim[j-1];
            point[j] = point[j-1];
        }
        sim[pos] = similarity;
        point[pos] = i;
    }
    vector< pair<LL, real> > ans;
    for(int i=0; i<k; ++i) {
        ans.push_back(make_pair(point[i], sim[i]));
    }
    free(point);
    free(sim);
    return ans;
}

vector< pair<LL, real> > WordTable::getKNear(vector<string> &words, const int &k)
{
    vector< pair<LL, real> > ans;
    real *vec = (real *)malloc(vectorSize * sizeof(real));
    for(LL i=0; i<vectorSize; ++i) vec[i] = 0;
    LL *wordIndex = (LL *)malloc((LL)words.size() * sizeof(LL));
    int inCnt = 0;
    for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it)
    {
        int index = getWordIndex(*it);
        if(index == -1) continue;
        wordIndex[inCnt] = index;
        ++inCnt;
        for(LL i=0; i<vectorSize; ++i) vec[i] += table[index*vectorSize+i];
    }
    if(inCnt == 0) return ans;
    real sum = 0;
    for(LL i=0; i<vectorSize; ++i){ vec[i] /= (real)inCnt; sum += (vec[i]*vec[i]); }
    sum = sqrt(sum);
    for(LL i=0; i<vectorSize; ++i) vec[i] /= sum;
    LL *point = (LL *)malloc(k * sizeof(LL));
    real *sim = (real *)malloc(k * sizeof(real));
    for(LL i=0; i<k; ++i) sim[i] = -1;
    for(LL i=0; i<vocabSize; ++i)
    {
        bool flag = false;
        for(int j=0; j<inCnt; ++j) if(wordIndex[j] == i) { flag = true; break; }
        if(flag) continue;
        real similarity = cosine(i, vec);
        int pos = 0;
        for(; pos<k; ++pos) if(similarity > sim[pos]) break;
        //cout << similarity << " " << pos << endl;
        if(pos < k) for(int j=k-1; j>pos; --j) { sim[j] = sim[j-1]; point[j] = point[j-1]; }
        if(pos < k) { sim[pos] = similarity; point[pos] = i; }
    }
    for(int i=0; i<k; ++i)
        ans.push_back(make_pair(point[i], sim[i]));
    return ans;
    free(vec); free(wordIndex);
    free(point); free(sim);
}

vector< pair<LL, real> > WordTable::analogy(vector<string> &words, const int &k)
{
    vector< pair<LL, real> > ans;
    if(words.size() < 3) { return ans; }
    LL wordIndex[3];
    for(int i=0; i<3; ++i) { 
        wordIndex[i] = getWordIndex(words[i]);
        if(wordIndex[i] == -1) { printf("word %s is not in vocabually.\n", words[i]); return ans; }
    }
    real *vec = (real *)malloc(sizeof(real) * (LL)vectorSize);
    memset(vec, 0, sizeof(vec));
    real sum = 0;
    for(LL i=0; i<vectorSize; ++i) {
        vec[i] = table[wordIndex[2] * vectorSize + i] - \
                 table[wordIndex[0] * vectorSize + i] + table[wordIndex[1] * vectorSize + i];
        sum += (vec[i] * vec[i]);
    }
    sum = sqrt(sum);
    for(LL i=0; i<vectorSize; ++i) vec[i] /= sum;
    ans = sim2vec(vec, k, wordIndex, 3);
    //printf("finished calculate sim2vec\n");
    free(vec);
    return ans;
}

