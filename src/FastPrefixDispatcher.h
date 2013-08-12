/*
 * FastPrefixDispatcher.h
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#ifndef FASTPREFIXDISPATCHER_H_
#define FASTPREFIXDISPATCHER_H_

#include <daework-kernel.h>

class FastPrefixDispatcher : public Dispatcher
{
	public:
		Action *getHandler(string action);

};

#endif /* FASTPREFIXDISPATCHER_H_ */
