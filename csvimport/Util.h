/*
 * Util.h
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "include.h"

class Util {
public:
	Util();
	virtual ~Util();

	static vector<string> split(string buffer, char separator);
	static string intToString(int i);
	static void ltrim(std::string &s);
	static void rtrim(std::string &s);
	static string escape(string line);
	static string jsonizeString(vector<string> *vFiledNames, string line, char separator);
	static map<string, string> desjsonize(string recordContent);


};

#endif /* UTIL_H_ */
