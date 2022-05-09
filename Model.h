//
// Created by Roman Titkov on 08.05.2022.
//

#ifndef LAB_2_MODEL_H
#define LAB_2_MODEL_H
#include <string>
#include <utility>
#include <vector>
#include "dictionary.h"

using namespace std;
class Model {
public:
    string modelName;
    vector <wstring> textEntries;
    vector <Word*> words;

    Model(string modelName) {
        this->modelName = std::move(modelName);
    }
};

#endif //LAB_2_MODEL_H
