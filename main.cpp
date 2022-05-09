#include <iostream>
#include <string>
#include <vector>
#include "filesystem"
#include <codecvt>
#include <fstream>
#include <sys/mman.h>
#include "dictionary.h"
#include "filemap.h"
#include "WordContext.h"
#include <chrono>
#include "json.hpp"
#include "Model.h"

using namespace std;
using recursive_directory_iterator = std::__fs::filesystem::recursive_directory_iterator;
using json = nlohmann::json;

int OUTPUT_COUNT = 50;

unordered_map <wstring, vector<WordContext*>> leftExt;
unordered_map <wstring, vector<WordContext*>> rightExt;
unordered_map <string, vector<int>> newNGramsPositions;
unordered_map <string, vector<int>> oldNGramsPositions;

vector<string> getFilesFromDir(const string& dirPath) {
    vector<string> files;
    for (const auto& dirEntry : recursive_directory_iterator(dirPath))
        if (!(dirEntry.path().filename() == ".DS_Store"))
            files.push_back(dirEntry.path());
    return files;
}

WordContext* processContext(const vector<Word*>& wordVector, int windowSize,
                            unordered_map <wstring, WordContext*> *NGrams) {
    wstring normalizedForm;
    normalizedForm.reserve(windowSize * 8);
    wstring leftNormalizedExt;
    leftNormalizedExt.reserve(windowSize * 8);
    wstring rightNormalizedExt;
    rightNormalizedExt.reserve(windowSize * 8);

    vector<Word*> contextVector;
    for (int i = 0; i < windowSize; i++) {
        Word* word = wordVector[i];
        contextVector.push_back(word);
        if (i == 0) {
            rightNormalizedExt += word->word + L" ";
        } else if (i == wordVector.size() - 1) {
            leftNormalizedExt += word->word + L" ";
        } else {
            leftNormalizedExt += word->word + L" ";
            rightNormalizedExt += word->word + L" ";
        }
        normalizedForm += word->word + L' ';
    }

    leftNormalizedExt.pop_back();
    rightNormalizedExt.pop_back();
    normalizedForm.pop_back();

    if (windowSize == 2 ||
        NGrams->find(leftNormalizedExt) != NGrams->end() ||
        NGrams->find(rightNormalizedExt) != NGrams->end()) {

        auto *wc = new WordContext(normalizedForm,contextVector);
        if (leftExt.find(leftNormalizedExt) == leftExt.end())
            leftExt.emplace(leftNormalizedExt, vector<WordContext *>{});
        if (rightExt.find(rightNormalizedExt) == rightExt.end())
            rightExt.emplace(rightNormalizedExt, vector<WordContext *>{});
        vector<WordContext*> &wordContextsLeft = leftExt.at(leftNormalizedExt);
        vector<WordContext*> &wordContextsRight = rightExt.at(rightNormalizedExt);
        wordContextsLeft.push_back(wc);
        wordContextsRight.push_back(wc);
        return wc;
    } else {
        return nullptr;
    }
}

void addContextToSet(WordContext* context, unordered_map <wstring, WordContext*>* NGramms, const string& filePath) {
    if (NGramms->find(context->normalizedForm) == NGramms->end())
        NGramms->emplace(context->normalizedForm, context);

    auto &ngram= NGramms->at(context->normalizedForm);
    ngram->totalEntryCount++;
    ngram->textEntry.insert(filePath);
}

void handleWord(Word* word,
                vector<Word*>* leftWordContext,
                unordered_map <wstring, WordContext*> *NGrams,
                int windowSize, const string& filePath,
                const int wordPosition) {
    for (int i = windowSize; i <= windowSize + 1; i++) {
        if (i <= leftWordContext->size()) {
            bool isCloseToNGramm = false;
            if (windowSize > 2) {
                if (oldNGramsPositions.find(filePath) == newNGramsPositions.end())
                    continue;
                vector<int> positions = oldNGramsPositions.at(filePath);
                for (int position : positions)
                    if (abs(wordPosition - position) <= 0) {
                        isCloseToNGramm = true;
                        break;
                    }
            } else
                isCloseToNGramm = true;

            if (!isCloseToNGramm)
                continue;

            WordContext* phraseContext = processContext(*leftWordContext, i, NGrams);
            if (phraseContext != nullptr) {
                addContextToSet(phraseContext, NGrams, filePath);
                if (newNGramsPositions.find(filePath) == newNGramsPositions.end())
                    newNGramsPositions.emplace(filePath, vector<int>{});
                newNGramsPositions.at(filePath).push_back(wordPosition);
            }
        }
    }
    if (leftWordContext->size() == (windowSize + 1)) {
        leftWordContext->erase(leftWordContext->begin());
    }

    leftWordContext->push_back(word);
}

void handleFile(const string& filepath, const vector<vector<Word*>>& fileContent,
                unordered_map <wstring, vector<Word*>> *dictionary,
                vector<Model*> &models) {
    vector<Word*> leftWordContext;

    for (int wordPosition = 0; wordPosition < fileContent.size(); wordPosition++) {
        for (auto model : models) {
            vector<Word*> suitableWords;
            bool correctModel = true;
            if (wordPosition + model->words.size() >= fileContent.size())
                continue;

            for (int modelWordPosition = 0; modelWordPosition < model->words.size(); modelWordPosition++) {
                bool correctModelWord = false;

                for (Word *word: fileContent[wordPosition + modelWordPosition]) {
                    if (word->partOfSpeech == model->words[modelWordPosition]->partOfSpeech) {
//                        wcout << word->word << ": " << word->partOfSpeech << " - "
//                              << model->words[modelWordPosition]->partOfSpeech << endl;
                        suitableWords.push_back(word);
                        correctModelWord = true;
                        break;
                    }
                }
                if (!correctModelWord) {
                    correctModel = false;
                    break;
                }
            }
            if (correctModel) {
                cout << model->modelName << ": " << endl;
                for (Word* textWord : suitableWords) {
                    wcout << " - " << textWord->word << ": " << textWord->partOfSpeech << endl;
                }
                cout << endl;
            }
        }
    }

//    for (const vector<Word*>& wordForms : fileContent) {
//        for (Word *word: wordForms) {
//        handleWord(word,&leftWordContext,
//                   NGrams, windowSize, filepath, wordPosition);
//        }
//        wordPosition++;
//    }
}

unordered_map <string, vector<vector<Word*>>> readAllTexts(const vector<string>& files,
                                                   unordered_map <wstring, vector<Word*>> *dictionary) {
    auto &f = std::use_facet<std::ctype<wchar_t>>(std::locale());
    unordered_map <string, vector<vector<Word*>>> fileContents;

    for (const auto& filepath : files) {
        fileContents.emplace(filepath, vector<vector<Word*>>{});
        vector<vector<Word*>> fileContent = fileContents.at(filepath);

        size_t length;
        auto filePtr = map_file(filepath.c_str(), length);
        auto firstChar = filePtr;
        auto lastChar = filePtr + length;

        while (filePtr && filePtr != lastChar) {
            auto stringBegin = filePtr;
            filePtr = static_cast<char *>(memchr(filePtr, '\n', lastChar - filePtr));

            wstring line = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(stringBegin, filePtr);
            wstring const delims{L" :;.,!?() \r\n"};

            size_t beg, pos = 0;
            while ((beg = line.find_first_not_of(delims, pos)) != string::npos) {
                pos = line.find_first_of(delims, beg + 1);
                wstring wordStr = line.substr(beg, pos - beg);

                f.toupper(&wordStr[0], &wordStr[0] + wordStr.size());

                if (dictionary->find(wordStr) == dictionary->end()) {
                    Word *newWord = new Word();
                    newWord->word = wordStr;
                    newWord->partOfSpeech = L"UNKW";
                    dictionary->emplace(newWord->word, vector<Word *>{newWord});
                }

                vector<Word*> &words = dictionary->at(wordStr);
                fileContent.push_back(words);

                if (line[pos] == L'.' || line[pos] == L',' ||
                    line[pos] == L'!' || line[pos] == L'?') {
                    wstring punct;
                    punct.push_back(line[pos]);
                    if (dictionary->find(punct) == dictionary->end()) {
                        Word *newWord = new Word();
                        newWord->word = punct;
                        newWord->partOfSpeech = L"UNKW";
                        dictionary->emplace(newWord->word, vector<Word *>{newWord});
                    }

                    vector<Word *> &wordsPunct = dictionary->at(punct);
                    fileContent.push_back(wordsPunct);
                }
            }
            if (filePtr)
                filePtr++;
        }
        munmap(firstChar, length);
        fileContents.at(filepath) = fileContent;
    }
    return fileContents;
}

void analyzeCorpusWithModels(const string& dictPath, const string& corpusPath,
                             vector<Model*> &models, const string& outputFile) {
    auto dictionary = initDictionary(dictPath);
    vector<string> files = getFilesFromDir(corpusPath);

    vector<WordContext *> stableNGrams;
    cout << "Start handling files\n";
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    unordered_map <string, vector<vector<Word*>>> filesContent = readAllTexts(files, &dictionary);

    for (const string &filepath: files) {
        handleFile(filepath, filesContent.at(filepath), &dictionary, models);
    }

    struct {
        bool operator()(const WordContext* a, const WordContext* b) const { return a->stability < b->stability; }
    } comp;
    sort(stableNGrams.begin(), stableNGrams.end(), comp);

    ofstream outFile (outputFile.c_str());

    cout << "\n";
    int printCount = 0;
    for (const WordContext* context : stableNGrams) {
        string line = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(context->normalizedForm);
        outFile << "<\"" << line << "\", " << context->totalEntryCount << ">\n";

        wcout << "<\"" << context->normalizedForm
        << "\", Total entries: " << context->totalEntryCount
        << ", TF: " << context->textEntry.size()
        << ", Stability: " << context->stability
        << ">\n";

        printCount++;
        if (printCount >= OUTPUT_COUNT)
            break;
    }

    outFile.close();
    cout << "End handling files\n";
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
}

int main() {
    OUTPUT_COUNT = 100;

    locale::global(locale("ru_RU.UTF-8"));
    wcout.imbue(locale("ru_RU.UTF-8"));

    string dictPath = "dict_opcorpora_clear.txt";
    string corpusPath = "/Users/titrom/Desktop/Computational Linguistics/Articles";
    string outputFile = "ngrams.txt";
    string modelsPath = "/Users/titrom/Desktop/Computational Linguistics/rules";

//    auto dictionary = initDictionary(dictPath);

    vector<string> modelsFiles = getFilesFromDir(modelsPath);
    vector<Model*> models;

    for (const auto& filepath : modelsFiles) {
        json modelJson;
        ifstream modelStream(filepath);
        modelStream >> modelJson;
        string modelName = modelJson["ModelName"];
        auto model = new Model(modelName);
        for (const auto& word : modelJson["Words"]) {
            auto modelWord = new Word();
            if (word.contains("pos")) {
                string str = word["pos"].get<string>();
                wstring attr = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str.data());
                modelWord->writeGrammeme(attr);
            }
            model->words.push_back(modelWord);
        }
        models.push_back(model);
    }

    for (auto model : models) {
        cout << model->modelName << ": " << model->textEntries.size() << " entries" << endl;
        for (auto word: model->words)
            wcout << " - " << word->partOfSpeech << endl;
        cout << endl;
    }

    analyzeCorpusWithModels(dictPath, corpusPath, models, outputFile);

    return 0;
}
