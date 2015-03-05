/*************************************************************************
	> File Name: Model.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Tue 06 Jan 2015 01:12:45 PM CST
 ************************************************************************/

#include "Model.h"

#include <string>
#include <pthread.h>

#include "Vocab.h"
using namespace std;

Vocab vocabulary;
// input layer
real *syn0, *syn1, *syn1sg;
real *docvecs;

// projection layer
real *neu1, *neu1e;
real *neu1sg, *neu1esg;

/*
 * init syn1 which corresponding to each inner node of huffman tree in
 * continuous bags of words model.
 */
void initCBOW()
{
    LL vocabSize = vocabulary.get_vocabSize();
    int flag, projSize = docvecSize + window * 2 * vectorSize;
    if(hs_flag>0)
    {
        flag = posix_memalign((void **)&syn1, 128, (LL)vocabSize * projSize * sizeof(real));
        if(flag!=0) { printf("syn1 allocation failed.\n"); exit(-1); }        
        if(!glPredictStage)
        {
            for(LL i=0; i<vocabSize; ++i) for(LL j=0; j<projSize; ++j)
                syn1[i * projSize + j] = 0;
        }
        else
        {
            //printf("read inner node vector from parameters.out\n");
            readParameter((char *)"./parameters.out");
        }
    }
    else { syn1 = NULL; }
}

/*
 * init syn1sg which corresponding to each inner node of huffman tree in 
 * skip gram model.
 */
void initSG()
{
    LL vocabSize = vocabulary.get_vocabSize();
    int flag = 0, projSize = docvecSize + vectorSize;
    if(hs_flag>0)
    {
        flag = posix_memalign((void **)&syn1sg, 128, (LL)vocabSize * projSize * sizeof(real));
        if(flag!=0) { printf("syn1sg allocation failed.\n"); exit(-1); }
        if(!glPredictStage)
        {
            for(LL i=0; i<vocabSize; ++i) for(LL j=0; j<projSize; ++j)
                syn1sg[i * projSize + j] = 0;
        }
        else
        {
            printf("read syn1sg from file.\n");
            exit(-1);
        }
    }
    else { syn1sg = NULL; } 
}

void initNet()
{
    LL vocabSize = vocabulary.get_vocabSize();
    LL trainDocCnt = vocabulary.get_trainDocCnt();
    ULL next_random = 1;
    int flag = posix_memalign((void **)&syn0, 128, (LL)vocabSize * vectorSize * sizeof(real));
    if(flag!=0) { printf("syn0 allocation failed.\n"); exit(-1); }
    flag = posix_memalign((void **)&docvecs, 128, (LL)trainDocCnt * docvecSize * sizeof(real));
    if(flag!=0) { printf("docvecs allocation failed.\n"); exit(-1); }
    // activate continuous bags of words architecture
    if(cbow_flag) { initCBOW(); }
    // activate skip gram architecture
    if(sg_flag>0) { initSG(); }

    if(!glPredictStage)
    {
        // initialize word vectors to random small real numbers
        for(LL i=0; i<vocabSize; ++i) for(LL j=0; j<vectorSize; ++j)
        {
            next_random = next_random * (ULL)25214903917 + 11;
            real r = ((next_random & 0xFFFF)/(real)(0xFFFF) - 0.5) / (real)vectorSize;
            syn0[i * vectorSize + j] = r;
        }
    }
    else
    {
        //printf("read word vectors from wordvecs.out");
        readWordvec((char *)"./wordvecs.out");
    }

    for(LL i=0; i<trainDocCnt; ++i) for(LL j=0; j<docvecSize; ++j)
    {
        next_random = next_random * (ULL)25214903917 + 11;
        real r = ((next_random & 0xFFFF)/(real)(0xFFFF) - 0.5) / (real)docvecSize;
        docvecs[i * docvecSize + j] = r;
    }
}

void *trainModelThread(void *id)
{
    LL trainDocCnt = vocabulary.get_trainDocCnt();
    LL trainWordCnt = vocabulary.get_trainWordCnt();
    LL leftIndex = (trainDocCnt / (LL)num_threads) * (LL)id;
    LL rightIndex = min((trainDocCnt / (LL)num_threads) * ((LL)id + 1), trainDocCnt);
    //printf("thread %lld: processing files [%lld, %lld)]\n", (LL)id, \
            leftIndex, rightIndex);
    int projSize = docvecSize + window * 2 * vectorSize; // continuous bags of words
    int projSizesg = docvecSize + vectorSize;            // skip gram
    if(cbow_flag > 0)
    {
        real *neu1 = (real *)calloc(projSize, sizeof(real));
        real *neu1e = (real *)calloc(projSize, sizeof(real));
    }
    if(sg_flag > 0)
    {
        real *neu1sg = (real *)calloc(projSizesg, sizeof(real));
        real *neu1esg = (real *)calloc(projSizesg, sizeof(real));
    }
    char *content = (char *)malloc(sizeof(char) * MAX_TEXT_LENGTH);
    LL contentSize = MAX_TEXT_LENGTH;
    LL length = 0;
    int *indexs = NULL;
    int indexsSize = MAX_TEXT_LENGTH / 6;
    indexs = (int *)malloc((LL)indexsSize * sizeof(int));
    
    LL localIter = 0, localTrainedWordCnt = 0;
    real localLearningRate = learningRate;
    while(localIter < numIteration)
    {
        //printf("thread: %lld, iteration: %lld\n", (LL)id, localIter);
        for(LL doc=leftIndex; doc<rightIndex; ++doc)
        {
            string docName = vocabulary.getDocName(doc);
            length = readFile((char *)docName.c_str(), content, contentSize);
            //printf("thread %lld, %s\n", (LL)id, filePath);
            int wordCnt = vocabulary.text2Index(content, indexs, indexsSize);
            //printf("thread %lld, wordCnt %d\n", (LL)id, wordCnt);
            if(wordCnt == 0) continue;
            // every windows is a sample
            for(int word=window-1; word<=wordCnt-window; ++word)
            {
                int shift = 0;
                for(int i=0; i<projSize; ++i) { neu1[i] = 0; neu1e[i] = 0; }
                for(int i=0; i<docvecSize; ++i) neu1[shift++] = docvecs[doc*docvecSize + i];
                for(int offset=-window; offset<=window; ++offset) {
                    if(offset == 0) continue;
                    int temp = word+offset;
                    if(temp == -1) temp = vocabulary.searchVocab((char *)"</beg>");
                    else if(temp == wordCnt) temp = vocabulary.searchVocab((char *)"</end>");
                    else temp = indexs[temp];
                    if(temp == -1) printf("oh my god.\n");
                    for(int i=0; i<vectorSize; ++i) neu1[shift++] = syn0[temp * vectorSize + i];
                }
                if(cbow_flag) // train continuous bag of words
                {
                    int center = indexs[word];
                    int codelen = vocabulary.get_codelen(center);
                    LL *point = vocabulary.get_point(center);
                    char *code = vocabulary.get_code(center);
                    if(hs_flag) for(int i=0; i<codelen; ++i)
                    {
                        real sum = 0;
                        LL cnu = point[i] * projSize; // classifer neural unit
                        for(LL j=0; j<projSize; ++j) sum += (neu1[j] * syn1[cnu + j]);
                        if(sum <= -MAX_EXP) sum = 0;
                        else if(sum >= MAX_EXP) sum = 1;
                        else sum = getSigmoid(sum);
                        real g = (1 - code[i] - sum) * localLearningRate;
                        // propagate errors from output to hidden
                        for(LL j=0; j<projSize; ++j) neu1e[j] += g*syn1[cnu+j];
                        // update parameters for each classifier node in Huffman Tree
                        if(!glPredictStage)
                            for(LL j=0; j<projSize; ++j) syn1[cnu+j] += g*neu1[j];
                    }
                    // To do: Negative sampling
                    // if(ns_flag > 0) 
                    // hidden to input
                    shift = 0;
                    for(int i=0; i<docvecSize; ++i) docvecs[doc*docvecSize + i] += neu1e[shift++];
                    if(!glPredictStage) for(int offset=-window; offset<=window; ++offset) {
                        if(offset == 0) continue;
                        int temp = word+offset;
                        if(temp == -1) temp = vocabulary.searchVocab((char *)"</beg>");
                        else if(temp == wordCnt) temp = vocabulary.searchVocab((char *)"</end>");
                        else temp = indexs[temp];
                        for(int i=0; i<vectorSize; ++i) syn0[temp * vectorSize + i] += neu1e[shift++];
                    }
                }
                ++localTrainedWordCnt; ++word_count_actual;
                if(!glPredictStage) if(localTrainedWordCnt % 10000 == 0)
                {
                    localLearningRate = learningRate * \
                                        (1 - word_count_actual / (real)(numIteration * trainWordCnt + 1));
                    if(localLearningRate < learningRate * 0.0001) 
                        localLearningRate = learningRate * 0.0001;
                    printf("thread: %lld, localLearningRate: %.8lf\n", (LL)id, localLearningRate);
                }
                if(glPredictStage) { 
                    localLearningRate = (1.0 - localIter/(real)numIteration) * learningRate; 
                    if(localLearningRate < 0.0001 * learningRate) 
                        localLearningRate = learningRate * 0.0001;
                }
            }
        }
        printf("thread %lld, finished iteration %lld, localLearningRate: %.8lf for next iteration\n", \
                (LL)id, localIter, localLearningRate);
        ++localIter;
    }
    printf("thread %lld finished.\n", (LL)id);
    free(content);
    free(indexs);
    free(neu1);
    free(neu1e);
    pthread_exit(NULL);
}

void trainModel()
{
    printf("begin to create huffman tree.\n");
    vocabulary.createBinaryTree();
    printf("finished create huffman tree.\n");
    // create threads for training
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
#ifdef __DEBUG__INFO__
    printf("begin initNet\n");
#endif
    initNet();
    printf("starting training model using file %s\n", train_file);
    for(LL i=0; i<num_threads; ++i) 
    {
        threads[i] = NULL;
        int temp = pthread_create(&threads[i], NULL, trainModelThread, (void *)i);
        if(temp != 0) { printf("create thread %d failed.", i); exit(-1); }
    }
    for(int i=0; i<num_threads; ++i) pthread_join(threads[i], NULL);
    //free(syn0);
    //free(syn1);
}

void runCBOW(LL doc, int *sen, int senSize, int word, 
        real localLearningRate, real *neu1, real *neu1e, int projSize)
{
    int *indexs = sen;
    int wordCnt = senSize;
    int shift = 0;
    for(int i=0; i<projSize; ++i) { neu1[i] = 0; neu1e[i] = 0; }
    for(int i=0; i<docvecSize; ++i) neu1[shift++] = docvecs[doc*docvecSize + i];
    for(int offset=-window; offset<=window; ++offset) {
        if(offset == 0) continue;
        int temp = word+offset;
        if(temp == -1) temp = vocabulary.searchVocab((char *)"</beg>");
        else if(temp == wordCnt) temp = vocabulary.searchVocab((char *)"</end>");
        else temp = indexs[temp];
        if(temp == -1) { printf("oh my god.\n"); exit(-1); }
        for(int i=0; i<vectorSize; ++i) neu1[shift++] = syn0[temp * vectorSize + i];
    }
    if(hs_flag) // training by hierarchical softmax algorithm
    {
        int center = indexs[word];
        int codelen = vocabulary.get_codelen(center);
        LL *point = vocabulary.get_point(center);
        char *code = vocabulary.get_code(center);
        if(hs_flag) for(int i=0; i<codelen; ++i)
        {
            real sum = 0;
            LL cnu = point[i] * projSize; // classifer neural unit
            for(LL j=0; j<projSize; ++j) sum += (neu1[j] * syn1[cnu + j]);
            if(sum <= -MAX_EXP) sum = 0;
            else if(sum >= MAX_EXP) sum = 1;
            else sum = getSigmoid(sum);
            real g = (1 - code[i] - sum) * localLearningRate;
            // propagate errors from output to hidden
            for(LL j=0; j<projSize; ++j) neu1e[j] += g*syn1[cnu+j];
            // update parameters for each classifier node in Huffman Tree
            if(!glPredictStage)
                for(LL j=0; j<projSize; ++j) syn1[cnu+j] += g*neu1[j];
        }
        // To do: Negative sampling
        // if(ns_flag > 0) 
        // hidden to input
        shift = 0;
        for(int i=0; i<docvecSize; ++i) docvecs[doc*docvecSize + i] += neu1e[shift++];
        if(!glPredictStage) for(int offset=-window; offset<=window; ++offset) {
            if(offset == 0) continue;
            int temp = word+offset;
            if(temp == -1) temp = vocabulary.searchVocab((char *)"</beg>");
            else if(temp == wordCnt) temp = vocabulary.searchVocab((char *)"</end>");
            else temp = indexs[temp];
            for(int i=0; i<vectorSize; ++i) syn0[temp * vectorSize + i] += neu1e[shift++];
        }
    }
}

void runSkipGram()
{

}

/*
 * description: write word vector for each word in vocabulary, so the row number
 * is the same with vocabulary size.
 * In the output file, the first row is two numbers cressponding to vocabulary size
 * and size of word vector.
 * Then for each row, each row corresponding to one word vector, the first string 
 * is the word, then followed by the corresponding word vector.
 */
void writeWordvec(char *filePath)
{
    FILE *fo;
    fo = fopen(filePath, "wb");
    if(fo == NULL) { printf("open output file %s failed.\n", filePath); exit(-1); }
    LL vocabSize = vocabulary.get_vocabSize(); 
    fprintf(fo, "%lld %lld\n", vocabSize, vectorSize);
    for(LL i=0; i<vocabSize; ++i)
    {
        fprintf(fo, "%s ", vocabulary.get_word(i));
        for(LL j=0; j<vectorSize; ++j) 
            fprintf(fo, "%.08lf%c", syn0[i*vectorSize + j], j==vectorSize-1?'\n':' ');
    }
    fclose(fo);
}

void readWordvec(char *filePath)
{
    printf("begin to read word vectors from file %s ...\n", filePath);
    FILE *fin;
    fin = fopen(filePath, "r");
    if(fin == NULL) { printf("open file %s failed.\n", filePath); exit(-1); }
    LL vocabSize = vocabulary.get_vocabSize();
    LL temp;
    fscanf(fin, "%lld", &temp); if(temp != vocabSize) {
        printf("read parameter vocabSize not the same with current model.\n"); exit(-1);
    }
    fscanf(fin, "%lld", &temp); if(temp != vectorSize) {
        printf("read parameter vectorSize not the same with current model.\n"); exit(-1);
    }
    char word[MAX_STRING];
    real realTemp;
    for(LL i=0; i<vocabSize; ++i) {
        fscanf(fin, "%s", word);
        LL wordIndex = vocabulary.searchVocab(word);
        if(wordIndex == -1) {
            printf("word %s is not in current vocabulary.\n", word);
            for(LL j=0; j<vectorSize; ++j) fscanf(fin, "%lf", &realTemp);
            continue;
        }
        else{
            for(LL j=0; j<vectorSize; ++j) 
                fscanf(fin, "%lf", &syn0[wordIndex * vectorSize + j]);
        }
    }
    fclose(fin);
    printf("finished read word vectors from file.\n");
}

/*
 * parameter output file format:
 * windowSize vectorSize docvecSize vocabSize trainDocCnt
 * vocabSize-1 projSize
 * col_0_0 col_0_1 ... col_0_{projSize-1}
 * ...
 * col_{vocabSize-2}_0 col_{vocabSize-2}_1 ... col_{vocabSize-2}_{projSize-1}
 * 
 * note: The number of inner node for Hierarchical Softmax algorithm is vocabSize-1.
 * The size of projection layer is docvecSize+2*window*vectorSize.
 * The variable in program represent inner node is syn1.
 */
void writeParameter(char *filePath)
{
    if(strlen(filePath) == 0) { return ; }
    FILE *fo;
    fo = fopen(filePath, "wb");
    if(fo == NULL) { printf("open parameters file failed.\n"); exit(-1); }
    LL vocabSize = vocabulary.get_vocabSize();
    LL trainDocCnt = vocabulary.get_trainDocCnt();
    int projSize = docvecSize + window * 2 * vectorSize;
    
    // windowSize vectorSize docvecSize vocabSize trainDocCnt
    fprintf(fo, "%lld %lld %lld %lld %lld\n", \
            (LL)window, (LL)vectorSize, (LL)docvecSize, (LL)vocabSize, (LL)trainDocCnt);
    
    // inner node of huffman tree, each node represents a logistic classification
    fprintf(fo, "%lld %lld\n", (LL)vocabSize - 1, (LL)projSize);
    for(LL i=0; i<vocabSize-1; ++i) for(LL j=0; j<projSize; ++j)
        fprintf(fo, "%.8lf%c", syn1[i * projSize + j], j==projSize-1?'\n':' '); 
    fclose(fo);
}

/*
 * Read parameters for syn1, more information see comment for function writeParameter.
 */
void readParameter(char *filePath)
{
    printf("begin to read parameters, espically for huffman tree inner nodes.\n");
    if(strlen(filePath) == 0) { printf("parameter file is not specified.\n"); exit(-1); }
    FILE *fin;
    fin = fopen(filePath, "r");
    if(fin == NULL) { printf("open parameter file faild in readParamter function.\n"); exit(-1); }

    LL vocabSize = vocabulary.get_vocabSize();
    LL trainDocCnt = vocabulary.get_trainDocCnt();
    LL projSize = docvecSize + 2 * window * vectorSize;
    LL temp;
    fscanf(fin, "%lld", &temp); if(temp != window) { 
        printf("read parameter window is not same with current model.\n"); exit(-1); 
    }
    fscanf(fin, "%lld", &temp); if(temp != vectorSize) { 
        printf("read parameter vectorSize is not same with current model.\n"); exit(-1); 
    }
    fscanf(fin, "%lld", &temp); if(temp != docvecSize) {
        printf("read parameter docvecSize is not same with current model.\n"); exit(-1);
    }
    fscanf(fin, "%lld", &temp); if(temp != vocabSize) {
        printf("read parameter vocabSize is not same with current model.\n"); exit(-1);
    }
    fscanf(fin, "%lld", &temp); // This is parameter trainDocCnt.
    LL rowNum, colNum;
    fscanf(fin, "%lld", &rowNum); if( rowNum != vocabSize - 1) {
        printf("read parameter inner node number faild.\n"); exit(-1);
    }
    fscanf(fin, "%lld", &colNum); if( colNum != projSize) {
        printf("read parameter projection size failed.\n"); exit(-1);
    }
    for(LL i=0; i<rowNum; ++i) for(LL j=0; j<colNum; ++j)
    {
        fscanf(fin, "%lf", &syn1[i * projSize + j]);
    } 
    fclose(fin);
    printf("finished read parameters for huffman tree inner nodes.\n");
}

void writeDocvec(char *filePath)
{
    if(strlen(filePath) == 0) return;
    FILE *fo;
    fo = fopen(filePath, "wb");
    if(fo == NULL) { printf("open docvec file failed.\n"); exit(-1); }
    LL trainDocCnt = vocabulary.get_trainDocCnt();
    fprintf(fo, "%lld %lld\n", trainDocCnt, (LL)docvecSize);
    for(LL i=0; i<trainDocCnt; ++i) 
    {
        string fileName = vocabulary.getDocName(i);
        fprintf(fo, "%s ", fileName.c_str());
        for(LL j=0; j<docvecSize; ++j)
            fprintf(fo, "%.8lf%c", docvecs[i * docvecSize + j], j==docvecSize-1?'\n':' ');
    }
    fclose(fo);
}

