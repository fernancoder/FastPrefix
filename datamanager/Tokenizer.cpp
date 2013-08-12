/*
 * tokenizer.cpp
 *
 *  Created on: 15/11/2012
 *      Author: fernancoder
 */

#include "Tokenizer.h"
#include "Normalization.h"

vector<string> *Tokenizer::extractWordsAndNormalize(string name, bool flagMarkIncompleteWord)
{
	currentWords.clear();

	ltrim(name);

	if ( name.length() != 0 )	//Si viene vacío no hace nada
	{
		name = normalize(name);	//Normaliza caracteres
		tokenize(name,flagMarkIncompleteWord);			//Separa en palabras
	}
	return &currentWords;
}

string Tokenizer::normalize(string name)
{
	unsigned char NormalizedName[MAX_MULTITERM_SIZE];
	unsigned char *pNormalizedName = NormalizedName;
	int curSize = 0;

	const unsigned char *pName = (const unsigned char *)name.c_str();
	while ( *pName )
	{
		switch ( utf8CharSize(*pName) )
		{
			case 0:	//Caracter no admitido
				pName++;
			break;
			case 1:
				*pNormalizedName++ = oneCharNorm[(int)*pName++];
				curSize++;
			break;
			case 2:
				if ( (int)*pName == 0xC3 && ((int)*(pName+1)) >= 0x80 )
				{
					*pNormalizedName++ = twoCharNorm[((int)*(pName+1))-0x80];
					curSize++;
				}
				pName++;
				if ( *pName )
					pName++;
			break;
			case 3:
				pName += 3;
			break;
		}
		//Control tamaño del string (con un factor de seguridad de 5)
		if ( curSize + 5 >= MAX_MULTITERM_SIZE )
			break;
	}

	*pNormalizedName = '\0';
	return string((const char*)NormalizedName);
}

void Tokenizer::tokenize(string name, bool flagMarkIncompleteWord)
{
	unsigned char currToken[MAX_TERM_SIZE];
	unsigned char *pCurrToken = currToken;

	int curTermSize = 0;

	const unsigned char *pName = (const unsigned char *)name.c_str();
	int status = 0;
	while ( *pName )
	{
		switch ( status )
		{
			case 0:
				if ( isSeparatorChar(*pName) )
				{
					status = 1;
					if ( pCurrToken != currToken )
					{
						*pCurrToken = '\0';
						if ( !isStopWord(string((const char*)currToken)) )
							currentWords.push_back(string((const char*)currToken));
						pCurrToken = currToken;
					}
				}
				else
				{
					if ( !isStopChar(*pName) )
					{
						curTermSize++;
						if ( curTermSize >= MAX_TERM_SIZE )
							return;
						*pCurrToken++ = *pName;
					}
				}
			break;
			case 1:
				if ( !isSeparatorChar(*pName) )
				{
					if ( !isStopChar(*pName) )
						*pCurrToken++ = *pName;
					curTermSize = 1;
					status = 0;
				}
			break;
		}
		pName++;
	}
	if ( status == 0 && pCurrToken != currToken )
	{
		*pCurrToken = '\0';
		if ( !isStopWord(string((const char*)currToken)) )
		{
			if (flagMarkIncompleteWord)
			{
				*pCurrToken++ = '*';
				*pCurrToken = '\0';
			}
			currentWords.push_back(string((const char*)currToken));
		}
	}
}


int Tokenizer::utf8CharSize (const unsigned char c)
{
	if ((c & 0x80) == 0x00) return 1;
	if ((c & 0xE0) == 0xC0) return 2;
	if ((c & 0xF0) == 0xE0) return 3;
	return 0;
}


bool Tokenizer::isSeparatorChar(const unsigned char c)
{
	for ( int i=0; separatorChar[i]; i++)
	{
		if (separatorChar[i] == c )
			return true;
	}
	return false;
}

bool Tokenizer::isStopChar(const unsigned char c)
{
	for ( int i=0; stopChar[i]; i++)
	{
		if (stopChar[i] == c )
			return true;
	}
	return false;
}

void Tokenizer::init(string stopWordsPath)
{
	std::string line;

	ifstream ifStopWords;
	ifStopWords.open (stopWordsPath.c_str(),ios_base::in);

	while (getline(ifStopWords, line, '\n'))
	{
		if ( line.length() > 0 && line.at(0) == '#' )
			continue;	//Comentario
		stopWords[line] = 1;
	}
	ifStopWords.close();
}

bool Tokenizer::isStopWord(string word)
{
    if (stopWords.find(word) == stopWords.end())
		return false;

    return true;
}

std::string Tokenizer::intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}


// trim from start
void Tokenizer::ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end
void Tokenizer::rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}


