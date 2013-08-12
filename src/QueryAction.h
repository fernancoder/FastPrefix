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
		vector<OcurrenceInfo *> *executeQuery(string queryExpression);
		string getData(vector<OcurrenceInfo *> *vctrOcurrenceInfo, int from, int size);

		void deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo);

};

#endif /* QUERYACTION_H_ */
