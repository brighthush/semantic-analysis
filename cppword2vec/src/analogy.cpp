/*************************************************************************
	> File Name: analogy.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Thu 08 Jan 2015 05:07:02 PM CST
 ************************************************************************/

#include <iostream>
#include <string>

#include "WordTable.h"

using namespace std;

int main()
{
    WordTable wordTable;
    wordTable.readTableFile("./word_vectors");
    while(true)
    {
        vector<string> words = wordTable.readInput();
        for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it)
            printf("%s ", (*it).c_str());
        printf("\n");
        if(words.size()==1 && words[0].compare("EXIT")==0) break;
        if(words.size() != 3) { printf("pleas input 3 words seperated by space.\n"); continue; }
        vector< pair<LL, real> > ans = wordTable.analogy(words);
        if(ans.size() == 0) { printf("input words not in vocabually.\n"); continue; }
        printf("analogy results is listed below.\n");
        for(vector< pair<LL, real> >::iterator it=ans.begin(); \
                it!=ans.end(); ++it)
        {
            printf("%s %lf\n", wordTable.get_word((*it).first).c_str(), (*it).second);
        }
        printf("\n");
    }
    return 0;
}

