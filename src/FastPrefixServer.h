/*
 * FastPrefixServer.h
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#ifndef FASTPREFIXSERVER_H_
#define FASTPREFIXSERVER_H_

#include <daework-include.h>
#include <daework-kernel.h>

#include "FastPrefixThread.h"
#include "FastPrefixDispatcher.h"

class FastPrefixServer  : public Server
{
	public:

		Dispatcher *dispatcherObject;

	public:
		FastPrefixServer(const char *cfgFilePath, const char *env, const char *module):Server(cfgFilePath, env, module){};

		void init();
		Thread *getThreadObject();
		Dispatcher *getDispatcherObject();

};
#endif /* FASTPREFIXSERVER_H_ */
