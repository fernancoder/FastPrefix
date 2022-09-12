/*
 * QueryAction.h
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#ifndef QUERYACTION_H_
#define QUERYACTION_H_

#include <daework-include.h>
#include <daework-kernel.h>
#include <daework-support.h>
#include <DwmMemcached.h>
#include "Tokenizer.h"
#include "Ocurrence.h"
#include "Record.h"
#include "FastPrefixServer.h"

class QueryAction : public Action
{

	public:
		static QueryAction *createHandler();

		QueryAction();
		~QueryAction();

		void execute();

	protected:
		vector<OcurrenceInfo *> *executeQuery(vector<string> *tokens);
		string getData(vector<OcurrenceInfo *> *vctrOcurrenceInfo, int from, int size, int *totalResults);
		string getMemCachedData(string queryNormalized);
		void setMemCachedData(string queryNormalized, string result);
		string normalizeQuery(vector<string> *tokens);
		void logAction(string queryExpression, int totalResults, bool isCached);

		void deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo);

};

#endif /* QUERYACTION_H_ */
