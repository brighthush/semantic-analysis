/*************************************************************************
	> File Name: main.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Sun 04 Jan 2015 04:32:15 PM CST
 ************************************************************************/

#include <iostream>
#include <algorithm>

#include <cstdio>
#include <cstring>
#include <cmath>

#include "glMember.h"
#include "VocabWord.h"
#include "Vocab.h"
#include "Model.h"

using namespace std;

int main()
{
    initExpTable();
    //strcpy(train_file, "./train_file_bak");
    strcpy(train_file, "./text8");
    strcpy(output_file, "./word_vectors");
    
    trainModel();
    //Vocab vocabulary;
    //vocabulary.learnVocabFromTrainFile();
    
    //vocabulary.display();
    //vocabulary.createBinaryTree();

    return 0;
}

