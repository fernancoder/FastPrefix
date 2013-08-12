/*
 * tokenizer.h
 *
 *  Created on: 15/11/2012
 *      Author: fernancoder
 */

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "include.h"

const int MAX_MULTITERM_SIZE=1024;
const int MAX_TERM_SIZE=32;

class Tokenizer
{
	protected:
		vector<string> currentWords;
		map<string, int> stopWords;

	public:
		void init(string stopWordsPath);
		string normalize(string name);
		vector<string> *extractWordsAndNormalize(string name, bool flagMarkIncompleteWord=false);

	protected:
		void tokenize(string name, bool flagMarkIncompleteWord = false);
		inline int utf8CharSize(const unsigned char c);
		inline bool isSeparatorChar(const unsigned char c);
		inline bool isStopChar(const unsigned char c);
		inline bool isStopWord(string word);
		string intToString(int i);
		void ltrim(string& str);
		void rtrim(string& str);

};


#endif /* TOKENIZER_H_ */
