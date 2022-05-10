//
// Created by Roman Titkov on 22.03.2022.
//

#ifndef LABS_DICTIONARY_H
#define LABS_DICTIONARY_H

#include <string>
#include <vector>
#include "filemap.h"
#include "set"
#include "unordered_map"

using namespace std;

class Word {
public:
    int totalEntryCount = 0;
    set <string> textEntry;
    wstring word;
    wstring pos;
    wstring anim;
    wstring gender;
    wstring number;
    wstring Case;
    wstring aspc;
    wstring trns;
    wstring pers;
    wstring tens;
    wstring mood;
    wstring invi;
    wstring voic;
    vector<wstring> semantics;
    Word* initWord = nullptr;

    Word();
    Word(const std::__1::basic_string<wchar_t, std::__1::char_traits<wchar_t>, std::__1::allocator<wchar_t>> &str);

    void writeGrammeme(const std::wstring &str);
    void setAttr(const string& key, const wstring& value);
    bool isSuitableWord(const Word *wordToCheck, unordered_map<wstring, vector<wstring>> &semantics) const;
};

wostream& operator<<(wostream& os, const Word& w);
unordered_map <wstring, vector<Word*>> initDictionary(const string& filename);
#endif //LABS_DICTIONARY_H
