#include <iostream>
#include <pthread.h>
#include <unordered_map>
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <vector>
using namespace std;

struct symbolInfo
{
public:
    double *probability;
    double *cumulativeProb;
    char *symbol;
    pthread_mutex_t *bsem;
    pthread_cond_t *waitTurn;
    int *printTurn;
    int *index;
};

// Primary function to execute, passes an argument and casting it to the symbolInfo struct
void *encoderFunc(void *arg)
{
    // Casts void *arg to symbolInfo struct
    symbolInfo *workingVar = (symbolInfo *)arg;
    // Critical section
    pthread_mutex_lock(workingVar->bsem);
    // Copy all the shared variables to a local var, and change the data holding them
    int index = *(workingVar->index);
    *(workingVar->index) += 1;
    double probability = *(workingVar->probability);
    double encodedProbability = *workingVar->cumulativeProb + (.5 * probability);
    char symbol = *(workingVar->symbol);
    workingVar->symbol++;
    workingVar->probability++;
    *(workingVar->cumulativeProb) += probability;
    pthread_mutex_unlock(workingVar->bsem);
    // Exit critical section
    // Work outside critical section
    int codeLength = ceil(log2(1 / probability)) + 1;
    int count = 1;
    string convertedCode;
    for (int i = 0; i < codeLength; i++)
    {
        if (pow(2, -count) <= encodedProbability)
        {
            encodedProbability -= pow(2, -count);
            convertedCode.append("1");
            count++;
        }
        else
        {
            convertedCode.append("0");
            count++;
        }
    }
    // Print critical section
    pthread_mutex_lock(workingVar->bsem);
    while (*(workingVar->printTurn) != index)
        pthread_cond_wait(workingVar->waitTurn, workingVar->bsem);
    cout << "Symbol " << symbol << ", Code: " << convertedCode << endl;
    *(workingVar->printTurn) += 1;
    pthread_cond_broadcast(workingVar->waitTurn);
    pthread_mutex_unlock(workingVar->bsem);

    return nullptr;
}

int main()
{
    // Initialize mutex and condition variable
    pthread_mutex_t bsem;
    pthread_mutex_init(&bsem, NULL);
    pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER;
    string inputChars;
    symbolInfo mainStruct;
    vector<double> probabilities;
    vector<char> symbols;
    int printTurn = 0, index = 0;
    // Initialize main struct to all the shared variables it's going to use
    mainStruct.printTurn = &printTurn;
    mainStruct.index = &index;
    cin >> inputChars;
    mainStruct.bsem = &bsem;
    mainStruct.waitTurn = &waitTurn;
    // Create hash table for frequencies
    unordered_map<char, int> symbolMap;
    // Increments frequency of every char in for loop
    for (char i : inputChars)
    {
        symbolMap[i]++;
    }
    double cumulativeProb = 0;
    mainStruct.cumulativeProb = &cumulativeProb;
    int fillCount = 0;
    for (char i : inputChars)
    {
        // Fills prob and char vector
        if (symbolMap[i] != 0)
        {
            probabilities.push_back((double)symbolMap[i] / inputChars.length());
            symbols.push_back(i);
            fillCount++;
            symbolMap[i] = 0;
        }
    }
    pthread_t tid[symbols.size()];
    mainStruct.probability = &probabilities[0];
    mainStruct.symbol = &symbols[0];
    // Initial title for expected output
    cout << "SHANNON-FANO-ELIAS Codes:" << endl
         << endl;
    // Making all the threads
    for (int i = 0; i < probabilities.size(); i++)
    {
        if (pthread_create(&tid[i], nullptr, encoderFunc, &mainStruct))
        {
            cout << "Error making threads\n";
            return 1;
        }
    }
    // Waiting for all the threads to end
    for (int i = 0; i < probabilities.size(); i++)
        pthread_join(tid[i], NULL);
}