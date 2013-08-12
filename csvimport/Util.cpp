/*
 * Util.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#include "Tokenizer.h"
#include "Util.h"

Util::Util() {
	// TODO Auto-generated constructor stub

}

Util::~Util() {
	// TODO Auto-generated destructor stub
}

vector<string> Util::split(string buffer, char separator)
{
	vector<string> parts;
	int iPos=0;
	int bufferLength = buffer.length();

	while ( iPos < bufferLength )
	{
		string part("");

		while ( iPos < bufferLength && buffer[iPos] != separator )
		{
			part += buffer[iPos];
			iPos++;
		}

		iPos++;
		parts.push_back(part);
	}

	return parts;
}

string Util::intToString(int i)
{
    stringstream ss;
    string s;
    ss << i;
    s = ss.str();

    return s;
}

// trim from start
void Util::ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end
void Util::rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

string Util::escape(string line)
{
	const char *ptrLine = line.c_str();
	char *response = (char *)malloc(line.length()*2 + 1);
	char *ptrResponse = response;

	while ( *ptrLine )
	{
		if ( *ptrLine == '\\' || *ptrLine == '"' )
		    *ptrResponse++ = '\\';
        *ptrResponse++ = *ptrLine++;
	}
	*ptrResponse = '\0';

	string srtResponse = string(response);
	free(response);

	return srtResponse;
}

string Util::jsonizeString(vector<string> *vFiledNames, string line, char separator)
{
	string response = string("");

	vector<string> vFieldValues = Util::split(line, separator);

	vector<string>::const_iterator iName=vFiledNames->begin();
	vector<string>::const_iterator iValue=vFieldValues.begin();
	for(; iName!=vFiledNames->end() && iValue!=vFieldValues.end(); iName++, iValue++)
	{
		if ( response.compare("") )
			response += ",";
        response += "\"" + *iName + "\":\"" + *iValue + "\"";
	}

	return response;
}

map<string, string> Util::desjsonize(string recordContent)
{
	char strKey[MAX_TERM_SIZE];
	char strValue[MAX_MULTITERM_SIZE];
    char *ptrStrKey = strKey;
    char *ptrStrValue = strValue;

    map<string, string> mapRecord;

	const char *curPtr = recordContent.c_str();
	int stat = 0;
	while ( *curPtr )
	{
		switch (stat)
		{
		    case 0: //Una "
		        stat = 1;
		    break;
		    case 1: //En la clave
		    	if ( *curPtr == '\\')
		    	{
		            stat = 2;
		    	}
		    	else if ( *curPtr == '"')
		        {
		        	*ptrStrKey = '\0';
		        	stat = 3;
		        }
		        else
		        	*ptrStrKey++ = *curPtr;
		    break;
		    case 2: //Caracter escapado
		    	*ptrStrKey++ = *curPtr;
		    	stat = 1;
		    break;
		    case 3: //Entre medias
		        if ( *curPtr == '"')
		        	stat = 4;
		    break;
		    case 4: //En el valor
		    	if ( *curPtr == '\\')
                {
                    stat = 5;
		    	}
                else if ( *curPtr == '"')
		        {
		        	*ptrStrValue = '\0';
		        	mapRecord[strKey] = strValue;
		            ptrStrKey = strKey;
		            ptrStrValue = strValue;
		            stat = 6;
		        }
		        else
		        	*ptrStrValue++ = *curPtr;
		    break;
		    case 5: //Caracter escapado
		        *ptrStrValue++ = *curPtr;
		    	stat = 4;
		    break;
		    case 6: //Entre medias
		        if ( *curPtr == '"')
		            stat = 1;
		    break;
		}
		curPtr++;
    }

    return mapRecord;
}
