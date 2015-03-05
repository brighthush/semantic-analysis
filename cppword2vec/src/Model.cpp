/*************************************************************************
	> File Name: Model.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Tue 06 Jan 2015 01:12:45 PM CST
 ************************************************************************/

#include "Model.h"

#include <pthread.h>

#include "Vocab.h"

Vocab vocabulary;
real *syn0, *syn1;

void initNet()
{
    LL vocabSize = vocabulary.get_vocabSize();
    ULL next_random = 1;
    int flag = posix_memalign((void **)&syn0, 128, (LL)vocabSize * vectorSize * sizeof(real));
    if(flag!=0) { printf("syn0 allocation failed.\n"); exit(-1); }
    if(hs_flag>0)
    {
        flag = posix_memalign((void **)&syn1, 128, (LL)vocabSize * vectorSize * sizeof(real));
        if(flag!=0) { printf("syn1 allocation failed.\n"); exit(-1); }
        for(LL i=0; i<vocabSize; ++i) for(LL j=0; j<vectorSize; ++j)
            syn1[i*vectorSize + j] = 0;
    }
    // initialize word vectors to random small real numbers
    for(LL i=0; i<vocabSize; ++i) for(LL j=0; j<vectorSize; ++j)
    {
        next_random = next_random * (unsigned LL)25214903917 + 11;
        real r = ((next_random & 0xFFFF)/(real)(0xFFFF) - 0.5) / (real)vectorSize;
        syn0[i * vectorSize + j] = r;
    }
}

void *trainModelThread(void *id)
{
    LL trainWordCount = vocabulary.get_trainWordCnt();
    ULL next_random = (ULL)id;
    LL localIter = numIteration;
    real localLearningRate = learningRate;
    LL wordCount = 0, wordTrainedCount = 0, lastWordCount = 0;
    LL sentenceLength = 0, sentencePosition = 0, sen[MAX_SENTENCE_LENGTH + 10];
    LL word, contextWord;
    FILE *fi = fopen(train_file, "rb");
    if(fi == NULL) { printf("In thread %lld, open %s failed.\n", train_file); pthread_exit(NULL); }
    fseek(fi, file_size/(LL)num_threads*(LL)id, SEEK_SET);
    real *neu1 = (real *)calloc(vectorSize, sizeof(real));
    real *neu1e = (real *)calloc(vectorSize, sizeof(real));
    clock_t begin = clock();
    while(true)
    {
        if(wordCount - lastWordCount > 10000)
        {
            word_count_actual += wordCount - lastWordCount;
            lastWordCount = wordCount;
            localLearningRate = learningRate * \
                                (1.0 - (real)word_count_actual / (real)(numIteration * trainWordCount));
            if(localLearningRate < learningRate * 0.0001) localLearningRate = learningRate * 0.0001;
        }
        if(sentenceLength==0)
        {
            while(true)
            {
                word = vocabulary.readWord2Index(fi);
                if(feof(fi)) break;
                if(word==-1) continue;
                ++wordCount;
                if(word == 0) break;
                // To Do : subsampling randomly
                // if(sample > 0)
                sen[sentenceLength++] = word;
                if(sentenceLength >= MAX_SENTENCE_LENGTH) break;
            }
            sentencePosition = 0;
        }
        if(sentenceLength > 0) while(true)
        {
            if(sentencePosition >= sentenceLength)
            {
                sentenceLength = sentencePosition = 0;
                break;
            }
            word = sen[sentencePosition];
            if(word == -1) { ++sentencePosition; continue; }
            // add sampling technique
            
            for(LL i=0; i<sentenceLength; ++i) { neu1[i]=0, neu1e[i]=0; }
            next_random = next_random*(ULL)25214903917 + 11;
            LL winSize = next_random%window + 1;
            if(cbow_flag) // train continuous bag of words
            {
                int cw = 0;
                for(LL i=-winSize; i<=winSize; ++i) if(i!=0)
                {
                    contextWord = sentencePosition + i;
                    if(contextWord<0) continue;
                    if(contextWord>=sentenceLength) continue;
                    contextWord = sen[contextWord];
                    if(contextWord == -1) continue;
                    for(LL j=0; j<vectorSize; j++) neu1[j] += syn0[contextWord*vectorSize + j];
                    ++cw;
                }
                if(cw>0)
                {
                    for(LL j=0; j<vectorSize; ++j) neu1[j] /= (real)cw;
                    int codelen = vocabulary.get_codelen(word);
                    LL *point = vocabulary.get_point(word);
                    char *code = vocabulary.get_code(word);
                    if(hs_flag) for(int i=0; i<codelen; ++i)
                    {
                        real sum = 0;
                        LL cnu = point[i] * vectorSize; // classifer neural unit
                        for(LL j=0; j<vectorSize; ++j) sum += (neu1[j] * syn1[cnu + j]);
                        if(sum <= -MAX_EXP) continue;
                        if(sum >= MAX_EXP) continue;
                        else sum = getSigmoid(sum);
                        real g = (1 - code[i] - sum) * localLearningRate;
                        // propagate errors from output to hidden
                        for(LL j=0; j<vectorSize; ++j) neu1e[j] += g*syn1[cnu+j];
                        // update parameters for each classifier node in Huffman Tree
                        for(LL j=0; j<vectorSize; ++j) syn1[cnu+j] += g*neu1[j];
                    }
                    // To do: Negative sampling
                    // if(ns_flag > 0) 
                    // hidden to input
                    for(LL i=-winSize; i<=winSize; ++i) if(i!=0)
                    {
                        contextWord = sentencePosition + i;
                        if(contextWord < 0) continue;
                        if(contextWord >= sentenceLength) continue;
                        contextWord = sen[contextWord];
                        if(contextWord == -1) continue;
                        for(LL j=0; j<vectorSize; ++j) syn0[contextWord*vectorSize + j] += neu1e[j];
                    }
                }
            }
            // To do else // train skip-gram
            //{
                //To do
            //}
            ++sentencePosition;
        }
        if(feof(fi) || (wordCount > (trainWordCount / num_threads)))
        {
            word_count_actual += wordCount - lastWordCount;
            clock_t now = clock();
            real cost = (now - begin + 1) / (real)CLOCKS_PER_SEC; 
            printf("thread %lld finished iteration %lld cost %lf seconds.\n", id, localIter, cost);
            --localIter;
            if(localIter == 0) break;
            wordCount = 0;
            lastWordCount = 0;
            fseek(fi, file_size / (LL)num_threads * (LL)id, SEEK_SET);
            continue;
        }
    }
    int temp = fclose(fi);
#ifdef __DEBUG__INFO__
    if(temp == 0) printf("fclose successed.\n");
    else if(temp == EOF) printf("fclose failed.\n");
#endif
    free(neu1);
    free(neu1e);
    pthread_exit(NULL);
}

void trainModel()
{
    vocabulary.learnVocabFromTrainFile();
#ifdef __DEBUG__INFO__
    vocabulary.display();
#endif
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
    FILE *fo;
    fo = fopen(output_file, "wb");
    if(fo == NULL) { printf("open output file %s failed.\n", output_file); exit(-1); }
    LL vocabSize = vocabulary.get_vocabSize(); 
    fprintf(fo, "%lld %lld\n", vocabSize, vectorSize);
    for(LL i=0; i<vocabSize; ++i)
    {
        fprintf(fo, "%s\t", vocabulary.get_word(i));
        for(LL j=0; j<vectorSize; ++j) fprintf(fo, "%.08lf\t", syn0[i*vectorSize + j]);
        fprintf(fo, "\n");
    }
    fclose(fo);
}

