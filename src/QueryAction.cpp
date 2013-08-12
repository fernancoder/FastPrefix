/*
 * QueryAction.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#include "QueryAction.h"

struct reverseWeightSort
{
   bool operator()(OcurrenceInfo *a, OcurrenceInfo *b) { return b->weight < a->weight; }
};

QueryAction *QueryAction::createHandler()
{
	return new QueryAction();
}

QueryAction::QueryAction()
{

}

QueryAction::~QueryAction()
{

}

void QueryAction::execute()
{
	string queryExpression = getRequestParam("terms");
	int firstRecord = atoi(getRequestParam("first").c_str());
	int pageSize = atoi(getRequestParam("size").c_str());

	if ( pageSize == 0 )
		pageSize = IniParser::getInstance()->getIntWithDefault("fpx", "response", "size", 20);

	vector<OcurrenceInfo *> *vctrOcurrenceInfo = this->executeQuery(queryExpression);
	if ( vctrOcurrenceInfo == NULL )
	{
	    this->sendError(2, "Cant open data base");
	}
	else
	{
	    string result = this->getData(vctrOcurrenceInfo, firstRecord, pageSize);
	    this->sendSuccess(result);
	    this->deleteOcurrenceInfo(vctrOcurrenceInfo);
    }
	//Cierra la conexión
	this->closeConnection();
}

vector<OcurrenceInfo *> *QueryAction::executeQuery(string queryExpression)
{
	Ocurrence *ocurrence = new Ocurrence();
	if ( !ocurrence->open(IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "terms_index", "/DATA/fastprefix.idt"),
			        IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "terms_data", "/DATA/fastprefix.ocr"),
			        IniParser::getInstance()->getIntWithDefault("fpx", "size", "level_1", 128),
			        IniParser::getInstance()->getIntWithDefault("fpx", "size", "level_2", 128),
			        IniParser::getInstance()->getIntWithDefault("fpx", "size", "level_3", 128) )
	   )
	{
		string logMsg;
		logMsg = "info.action=#openDataBase#;info.error=#Can't open data base#";
		LogManager::getInstance()->write(logMsg, LOG_CRIT);
		return NULL;
	};

	//Completo o prefijo
	if ( queryExpression[queryExpression.length()-1] != ' ' )
		queryExpression += '*';


	Tokenizer *tokenizer = new Tokenizer();
	vector<string> *tokens = tokenizer->extractWordsAndNormalize(queryExpression);

	vector<OcurrenceInfo *> *vctrCurrentOcurrenceInfo;
	vector<OcurrenceInfo *> *vctrTotalOcurrenceInfo = NULL;

	for(vector<string>::const_iterator term=tokens->begin(); term!=tokens->end(); term++)
	{
		if ( (*term)[term->length()-1] == '*' )	//Prefijo
			vctrCurrentOcurrenceInfo = ocurrence->findPrefix(term->substr(0,(term->length()-1)));
		else								//Término
			vctrCurrentOcurrenceInfo = ocurrence->findTerm(*term);

		/*
		for(vector<OcurrenceInfo *>::const_iterator it=vctrCurrentOcurrenceInfo->begin(); it!=vctrCurrentOcurrenceInfo->end(); it++)
			cout << (*it)->id << " peso " << (*it)->weight << endl;

		cout << "*************************" << endl;
		*/

		if ( vctrTotalOcurrenceInfo == NULL )
		{
			vctrTotalOcurrenceInfo = vctrCurrentOcurrenceInfo;
		}
		else
		{
			vector<OcurrenceInfo *> *vctrWorkOcurrenceInfo = ocurrence->operateAnd(vctrTotalOcurrenceInfo,vctrCurrentOcurrenceInfo);
			ocurrence->deleteOcurrenceInfo(vctrTotalOcurrenceInfo);
			ocurrence->deleteOcurrenceInfo(vctrCurrentOcurrenceInfo);
			vctrTotalOcurrenceInfo = vctrWorkOcurrenceInfo;
		}
	}

	delete ocurrence;

	return vctrTotalOcurrenceInfo;
}


string QueryAction::getData(vector<OcurrenceInfo *> *vctrOcurrenceInfo, int from, int size)
{

	std::sort(vctrOcurrenceInfo->begin(), vctrOcurrenceInfo->end(),reverseWeightSort());

	Record *record = new Record();
	record->open(IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "record_index", "/DATA/fastprefix.idx"),
	             IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "record_data", "/DATA/fastprefix.dat"));

	string res = "\"totalResults\" : ";
    res += Util::intToString(vctrOcurrenceInfo->size());
	res += ", \"records\" : [";

	//ESTA FORMA DE RECORRER EL VECTOR ES POCO EFICIENTE
	unsigned int curPos = 0;
	bool isFirstResult = true;
	for(vector<OcurrenceInfo *>::const_iterator it=vctrOcurrenceInfo->begin(); it!=vctrOcurrenceInfo->end(); it++)
	{
		if ( curPos >= from && curPos < from+size )
		{
			if ( !isFirstResult )
				res += ", ";
			else
				isFirstResult = false;
			res += "{"+record->getRecord((*it)->id) + "}";
		}

		curPos++;
		if ( curPos >= from+size )
			break;
	}

	delete record;

	res += "]";

	return res;
}

void QueryAction::deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo)
{
	for(vector<OcurrenceInfo *>::const_iterator it=vctrOcurrenceInfo->begin(); it!=vctrOcurrenceInfo->end(); it++)
		delete *it;
	delete vctrOcurrenceInfo;
}

