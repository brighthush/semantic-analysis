# semantic-analysis
Semantic analysis for text, including word2vec and text2vec which are implemented in c++ by myself.  

## word2vec  
This project is in directory named cppword2vec, I implemented word2vec in C++.  
In Model.cpp, I implemented CBOW model with Hierarchical Softmax and Negative Sampling.  
The main idea of Hierarchical Softmax is to model the output of Neural Network as a Huffman Tree. In this way, we only calculate the probability of one term on the Huffman Path. This means we do not need to calculate softmax for all words in the plain Neural Network Language Model. This techinique can speed up the train process.  
The other way of train language model is to do Negative Sampling. In this way, each word has a parameter theta. The output of the three level neural network, regared as an input of a Logistic Classification. But the situation is that we only have the context of each word in the text, it means we only have Positive Samples. So we need to get Negative Samples, this is also why this method called Negative Sampling. When we do negative sampling, we only sample words which are not in context of
input word w.  
Another input architecture of word2vec neural network is Skip-gram. In this structure of neural network, we use word w to predict the context of w. This cause the input of neural network difference. When training model, we can also use Hierarchical Softmax and Negative Sampling.

## text2vec  
This project is in directory named text2vec. Code in this project are chaged from cppword2vec. It means these code are based on the previous project named cppword2vec.  
So in text2vec, I only implemented CBOW with Hierarchical Softmax and Negative Sampling. Text vectors are trained with word vectors. In the previous architecture of word2vec, text vector is added in the input layer. But one thing need to be noticed is text vector is unique for one text.  
So this method can be used to train paragraph semantic vector by only keep the vector of a paragraph to be unique.  
After train this model, we get text semantic vector for every text. I use these vectors as features of a document to do classification. The classification model is a three level neural network.  
My training corpus is Open Sogou Text Classification Corpus. So every document is one class of 10 types.  

