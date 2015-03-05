/*************************************************************************
	> File Name: Measure.cpp
	> Author: bright
	> Mail: luzihao@software.ict.ac.cn
	> Created Time: Fri 09 Jan 2015 05:03:28 PM CST
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>

#include "glMember.h"
#include "WordTable.h"

using namespace std;

WordTable wordTable;

int evaluate(string line)
{
    vector<string> words;
    int n = line.size();
    int pre = 0;
    for(int i=0; i<n; ++i)
    {
        if(line[i]>='A' && line[i]<='Z') line[i] = line[i] - 'A' + 'a';
        if(line[i]==' ')
        {
            words.push_back(line.substr(pre, i-pre));
            pre = i + 1;
        }
        else if(i==n-1)
            words.push_back(line.substr(pre, i-pre+1));
    }
    if(words.size() != 4) { printf("this line is not contain four words.\n"); return -1; }
    int k = 1;
    vector< pair<LL, real> > ans = wordTable.analogy(words, k);
    if(ans.size() == 0) return -1;
    string guessed = wordTable.get_word(ans[0].first);
    if(words[3].compare(guessed) == 0)
        return 1;
    return 0;
}

vector< pair<string, real> > evaluate(char *filePath)
{
    vector< pair<string, real> > ans;
    string topicName;
    int total = 0, totalRight = 0;
    int topicTotal = 0, topicRight = 0;
    ifstream fin;
    fin.open(filePath, ios::in);
    string line;
    real precision = 0;
    while(getline(fin, line))
    {
        if(line[0] == '/') continue;
        else if(line[0] == ':')
        {
            if(topicTotal != 0)
            {
                precision = (real)topicRight / (real)topicTotal;
                ans.push_back(make_pair(topicName, precision));
            }
            topicName = line;
            topicTotal = 0;
            topicRight = 0;
        }
        else
        {
            int temp = evaluate(line);
            if(temp == -1) { printf("%s\n", line.c_str()); continue; }
            else if(temp == 1) { ++topicRight; ++totalRight; }
            ++topicTotal;
            ++total;
        }
    }
    precision = (real)topicRight / (real)topicTotal;
    ans.push_back(make_pair(topicName, precision));
    precision = (real)totalRight / (real)total;
    ans.push_back(make_pair(string("total_precision"), precision));
    return ans;
}

int main()
{
    wordTable.readTableFile("./word_vectors");
    vector< pair<string, real> > ans = evaluate("./word-test.v1.txt");
    int n = ans.size();
    for(int i=0; i<n; ++i) { printf("%s %lf\n", ans[i].first.c_str(), ans[i].second); }
    return 0;
}
