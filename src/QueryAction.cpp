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

	//Completo o prefijo
	if ( queryExpression[queryExpression.length()-1] != ' ' )
	    queryExpression += '*';

	Tokenizer *tokenizer = new Tokenizer();
	vector<string> *tokens = tokenizer->extractWordsAndNormalize(queryExpression);

	string queryNormalized;
	if ( firstRecord == 0 ) //tira de memcached
	{
		queryNormalized = this->normalizeQuery(tokens);
	    string result = this->getMemCachedData(queryNormalized);
	    if ( result.length() != 0 )
	    {
	        this->sendSuccess(result);

	        //Cierra la conexión
	        this->closeConnection();
	        delete tokenizer;
	        this->logAction(queryExpression,0,true);
	        return;
	    }
	}

	int totalResults;
	vector<OcurrenceInfo *> *vctrOcurrenceInfo = this->executeQuery(tokens);
	if ( vctrOcurrenceInfo == NULL )
	{
        this->sendError(2, "Cant open data base");
	}
	else
	{
        string result = this->getData(vctrOcurrenceInfo, firstRecord, pageSize, &totalResults);
	    this->sendSuccess(result);
	    this->deleteOcurrenceInfo(vctrOcurrenceInfo);
	    if ( firstRecord == 0 && totalResults > 0 ) //al memcached
	        this->setMemCachedData(queryNormalized,result);
	}
	//Cierra la conexión
	this->closeConnection();
	delete tokenizer;
	this->logAction(queryExpression,totalResults,false);
}

vector<OcurrenceInfo *> *QueryAction::executeQuery(vector<string> *tokens)
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


string QueryAction::getData(vector<OcurrenceInfo *> *vctrOcurrenceInfo, int from, int size, int *totalResults)
{

	std::sort(vctrOcurrenceInfo->begin(), vctrOcurrenceInfo->end(),reverseWeightSort());

	Record *record = new Record();
	record->open(IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "record_index", "/DATA/fastprefix.idx"),
	             IniParser::getInstance()->getStringWithDefault("fpx", "filepath", "record_data", "/DATA/fastprefix.dat"));

	*totalResults = vctrOcurrenceInfo->size();

	string res = "\"totalResults\" : ";
    res += Util::intToString(vctrOcurrenceInfo->size());
	res += ", \"records\" : [";

	//ESTA FORMA DE RECORRER EL VECTOR ES POCO EFICIENTE
	unsigned int curPos = 0;
	bool isFirstResult = true;
	for(vector<OcurrenceInfo *>::const_iterator it=vctrOcurrenceInfo->begin(); it!=vctrOcurrenceInfo->end(); it++)
	{
		if ( curPos >= (unsigned int)from && curPos < (unsigned int)(from+size) )
		{
			if ( !isFirstResult )
				res += ", ";
			else
				isFirstResult = false;
			res += "{"+record->getRecord((*it)->id) + "}";
		}

		curPos++;
		if ( curPos >= (unsigned int)(from+size) )
			break;
	}

	delete record;

	res += "]";

	return res;
}

string QueryAction::getMemCachedData(string queryNormalized)
{
	memcached_st *Mch = DwmMemcached::getInstance()->getMemcachedAccess();

	memcached_return rc;
	char *retvalue = NULL;
    size_t retlength;
	uint32_t flags;

	string response;

	retvalue = memcached_get(Mch, queryNormalized.c_str(), queryNormalized.length(), &retlength, &flags, &rc);
	if (rc == MEMCACHED_SUCCESS)
	{
		response = string(retvalue);
		free(retvalue);
	}
	else
		response = string("");

	DwmMemcached::getInstance()->releaseMemcachedAccess(Mch);

	return response;
}

void QueryAction::setMemCachedData(string queryNormalized, string result)
{
	memcached_st *Mch = DwmMemcached::getInstance()->getMemcachedAccess();
	memcached_return rc = memcached_set(Mch, queryNormalized.c_str(), queryNormalized.length(), result.c_str(), result.length(), (time_t)0, (uint32_t)0);
	DwmMemcached::getInstance()->releaseMemcachedAccess(Mch);
}

string QueryAction::normalizeQuery(vector<string> *tokens)
{
	string response = string("");
	for(vector<string>::const_iterator term=tokens->begin(); term!=tokens->end(); term++)
	{
		if ( response.length() != 0 )
			response += '_';
		response += *term;
	}
	return response;
}

void QueryAction::logAction(string queryExpression, int totalResults, bool isCached)
{
	string logMsg = string("info.action=#query#;info.terms=#");
	logMsg.append(queryExpression);
	logMsg.append("#;info.totalResults=#");
	logMsg.append(Util::intToString(totalResults));
	logMsg.append("#;info.fromCahe=#");
	if ( isCached )
		logMsg.append("true#");
	else
		logMsg.append("false#");

	LogManager::getInstance()->write(logMsg, LOG_INFO);
}

void QueryAction::deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo)
{
	for(vector<OcurrenceInfo *>::const_iterator it=vctrOcurrenceInfo->begin(); it!=vctrOcurrenceInfo->end(); it++)
		delete *it;
	delete vctrOcurrenceInfo;
}

