//
// Created by Roman Titkov on 22.03.2022.
//

#include "dictionary.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include "unordered_map"
#include "filesystem"
#include <codecvt>
#include <string>

using recursive_directory_iterator = std::__fs::filesystem::recursive_directory_iterator;

Word::Word() {}

Word::Word(const wstring& str) {
//    this->initWord = nullptr;
    int begin = 0;
    for (int pos = 0; pos < str.size(); pos++) {
        if (str[pos] == L' ') {
            if (begin == 0)
                this->word = str.substr(begin, pos - begin);
            else
                this->writeGrammeme(str.substr(begin, pos - begin));
            begin = pos + 1;
        }
    }
}

void Word::writeGrammeme(const wstring& str) {
    if (str == L"NOUN" || str == L"ADJF" || str == L"ADJS" || str == L"COMP" ||
        str == L"VERB" || str == L"INFN" || str == L"PRTF" || str == L"PRTS" ||
        str == L"GRND" || str == L"NUMR" || str == L"ADVB" || str == L"NPRO" ||
        str == L"PRED" || str == L"PREP" || str == L"CONJ" || str == L"PRCL" ||
        str == L"INTJ") {
        this->pos = str;
    } else
    if (str == L"anim" || str == L"inan") {
        this->anim = str;
    } else
    if (str == L"masc" || str == L"femn" || str == L"neut" || str == L"ms-f" ||
        str == L"GNdr") {
        this->gender = str;
    } else
    if (str == L"sing" || str == L"plur" || str == L"Sgtm" || str == L"Pltm" || str == L"Fixd") {
        this->number = str;
    } else
    if (str == L"nomn" || str == L"gent" || str == L"datv" || str == L"accs" ||
        str == L"ablt" || str == L"loct" || str == L"voct" || str == L"gen1" ||
        str == L"gen2" || str == L"acc2" || str == L"loc1" || str == L"loc2" ||
        str == L"Abbr" || str == L"Name" || str == L"Surn" || str == L"Patr" ||
        str == L"Geox" || str == L"Orgn" || str == L"Trad" || str == L"Subx" ||
        str == L"Supr" || str == L"Qual" || str == L"Apro" || str == L"Anum" ||
        str == L"Poss" || str == L"V-ey" || str == L"V-oy" || str == L"Cmp2" ||
        str == L"V-ej") {
        this->Case = str;
    } else
    if (str == L"perf" || str == L"impf") {
        this->aspc = str;
    } else
    if (str == L"tran" || str == L"intr" || str == L"Impe" || str == L"Impx" ||
        str == L"Mult" || str == L"Refl") {
        this->trns = str;
    } else
    if (str == L"1per" || str == L"2per" || str == L"3per") {
        this->pers = str;
    } else
    if (str == L"pres" || str == L"past" || str == L"futr") {
        this->tens = str;
    } else
    if (str == L"indc" || str == L"impr") {
        this->mood = str;
    } else
    if (str == L"incl" || str == L"excl") {
        this->invi = str;
    } else
    if (str == L"actv" || str == L"pssv" || str == L"Infr" || str == L"Slng" ||
        str == L"Arch" || str == L"Litr" || str == L"Erro" || str == L"Dist" ||
        str == L"Ques" || str == L"Dmns" || str == L"Prnt" || str == L"V-be" ||
        str == L"V-en" || str == L"V-ie" || str == L"V-bi" || str == L"Fimp" ||
        str == L"Prdx" || str == L"Coun" || str == L"Coll" || str == L"V-sh" ||
        str == L"Af-p" || str == L"Inmx" || str == L"Vpre" || str == L"Anph" ||
        str == L"Init" || str == L"Adjx" || str == L"Ms-f" || str == L"Hypo") {
        this->voic = str;
    }
}

void Word::setAttr(const string& key, const wstring& value) {
    if (key == "word") this->word = value;
    else if (key == "pos" || key == "anim" || key == "gender" ||
        key == "number" || key == "Case" || key == "aspc" ||
        key == "trns" || key == "pers" || key == "tens" ||
        key == "mood" || key == "invi" || key == "voic")
        this->writeGrammeme(value);
    else {
        this->semantics.push_back(value);
    }
}

bool Word::isSuitableWord(const Word* wordToCheck, unordered_map <wstring, vector<wstring>> &semantics) const {
    if (!this->word.empty() && !wordToCheck->word.empty())
        if (this->word != wordToCheck->word) return false;
    if (!this->pos.empty())
        if (this->pos != wordToCheck->pos) return false;
    if (!this->anim.empty())
        if (this->anim != wordToCheck->anim) return false;
    if (!this->gender.empty())
        if (this->gender != wordToCheck->gender) return false;
    if (!this->number.empty())
        if (this->number != wordToCheck->number) return false;
    if (!this->Case.empty())
        if (this->Case != wordToCheck->Case) return false;
    if (!this->aspc.empty())
        if (this->aspc != wordToCheck->aspc) return false;
    if (!this->trns.empty())
        if (this->trns != wordToCheck->trns) return false;
    if (!this->pers.empty())
        if (this->pers != wordToCheck->pers) return false;
    if (!this->tens.empty())
        if (this->tens != wordToCheck->tens) return false;
    if (!this->mood.empty())
        if (this->mood != wordToCheck->mood) return false;
    if (!this->invi.empty())
        if (this->invi != wordToCheck->invi) return false;
    if (!this->voic.empty())
        if (this->voic != wordToCheck->voic) return false;

    if (!this->semantics.empty()) {
        for (const wstring& semantic : this->semantics) {
            bool correctSemantic = false;
            vector<wstring> &semanticWords = semantics.at(semantic);
            for (const wstring& semanticWord : semanticWords) {
                wstring wordForm = wordToCheck->word;
                if (wordToCheck->initWord != nullptr)
                    wordForm = wordToCheck->initWord->word;

                if (wordForm == semanticWord) {
                    correctSemantic = true;
                    break;
                }
            }
            if (!correctSemantic) return false;
        }
    }

    return true;
}
wostream& operator<<(wostream& os, const Word& w)
{
    if (!w.word.empty())
        os << "word: \"" << w.word << "\" | ";
    if (!w.pos.empty())
        os << "pos: \"" << w.pos << "\" | ";
    if (!w.anim.empty())
        os << "anim: \"" << w.anim << "\" | ";
    if (!w.gender.empty())
        os << "gender: \"" << w.gender << "\" | ";
    if (!w.number.empty())
        os << "number: \"" << w.number << "\" | ";
    if (!w.Case.empty())
        os << "Case: \"" << w.Case << "\" | ";
    if (!w.aspc.empty())
        os << "aspc: \"" << w.aspc << "\" | ";
    if (!w.trns.empty())
        os << "trns: \"" << w.trns << "\" | ";
    if (!w.pers.empty())
        os << "pers: \"" << w.pers << "\" | ";
    if (!w.tens.empty())
        os << "tens: \"" << w.tens << "\" | ";
    if (!w.mood.empty())
        os << "mood: \"" << w.mood << "\" | ";
    if (!w.invi.empty())
        os << "invi: \"" << w.invi << "\" | ";
    if (!w.voic.empty())
        os << "voic: \"" << w.voic << "\" | ";
    if (!w.semantics.empty()) {
        os << "semantics: ";
        for (const wstring& semantic: w.semantics)
            os << "\"" << semantic << "\" | ";
    }
    if (w.initWord != nullptr && !w.initWord->word.empty()) {
        os << "init: \"" << w.initWord->word << "\" | ";
    }

    if (w.word.empty() && w.pos.empty() && w.anim.empty() && w.gender.empty() && w.number.empty() && w.Case.empty() &&
        w.aspc.empty() && w.trns.empty() && w.pers.empty() && w.tens.empty() && w.mood.empty() && w.invi.empty() &&
        w.voic.empty() && w.semantics.empty()) {
        os << "ANY";
    }

    return os;
}

wstring getWord(const wstring &line) {
    int endPosition = 0;
    while (line.at(endPosition) != L' ')
        endPosition++;

    return line.substr(0, endPosition);
}

unordered_map <wstring, vector<Word*>> initDictionary(const string& filename) {
    cout << "Initializing dictionary...\n";
    unordered_map <wstring, vector<Word*>> dictionary;
    wifstream infile(filename);
    int wordCount = 0;
    Word *initWord = nullptr;

    size_t length;
    auto filePtr = map_file(filename.c_str(), length);
    auto lastChar = filePtr + length;

    while (filePtr && filePtr != lastChar) {
        auto stringBegin = filePtr;
        filePtr = static_cast<char *>(memchr(filePtr, '\n', lastChar - filePtr));
        if (filePtr)
            filePtr++;
        else
            break;

        if (filePtr - stringBegin < 10) {
            initWord = nullptr;
            continue;
        }

        wstring line = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(stringBegin, filePtr - 1);

        Word *currentWord;
        if (initWord == nullptr) {
            initWord = new Word(line);
            currentWord = initWord;
        } else {
            currentWord = new Word(line);
        }

        currentWord->initWord = initWord;
        wstring word = getWord(line);

        if (dictionary.find(word) != dictionary.end()) {
            dictionary[word].push_back(currentWord);
        } else {
            dictionary.emplace(word, vector<Word*>{currentWord});
        }
        if (wordCount % 500000 == 0) {
            cout << "Words inserted: " << wordCount << endl;
        }
        wordCount++;
    }

    cout << "Dictionary initialized.\n";
    return dictionary;
}