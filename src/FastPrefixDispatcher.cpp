/*
 * FastPrefixDispatcher.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernando
 */

#include "FastPrefixDispatcher.h"
#include "QueryAction.h"
#include "ReloadAction.h"


Action *FastPrefixDispatcher::getHandler(string action)
{

	//Esto será una tabla o una clase o similar
	if ( action.compare("query") == 0 )
		return QueryAction::createHandler();
	if ( action.compare("reload") == 0 )
		return ReloadAction::createHandler();

	return defaultAction(action);

}

