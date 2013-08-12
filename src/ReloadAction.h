/*
 * ReloadAction.h
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#ifndef RELOADACTION_H_
#define RELOADACTION_H_

#include "daework-include.h"
#include "daework-kernel.h"
#include "FastPrefixServer.h"

class ReloadAction : public Action
{

	public:
		static ReloadAction *createHandler();
		void execute();

};

#endif /* RELOADACTION_H_ */
