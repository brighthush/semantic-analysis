/*************************************************************************
	> File Name: distance.cpp
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
        if(words.size()==1 && words[0].compare("EXIT")==0) break;
        for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it)
            cout << *it << endl;
        vector< pair<LL, real> > ans = wordTable.getKNear(words);
        if(ans.size() == 0) { printf("input words not in vocabually.\n"); continue; }
        printf("nearest words for the input words is listed below.\n");
        for(vector< pair<LL, real> >::iterator it=ans.begin(); \
                it!=ans.end(); ++it)
        {
            printf("%s %lf\n", wordTable.get_word((*it).first).c_str(), (*it).second);
        }
        printf("\n");
    }
    return 0;
}
