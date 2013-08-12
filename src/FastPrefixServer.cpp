/*
 * FastPrefixServer.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernando
 */

#include "FastPrefixServer.h"

void FastPrefixServer::init()
{
	cout << "Inicio servidor " << endl;

	this->dispatcherObject = (Dispatcher *)(new FastPrefixDispatcher());
}

Thread *FastPrefixServer::getThreadObject()
{
	return (Thread *)(new FastPrefixThread());
}

Dispatcher *FastPrefixServer::getDispatcherObject()
{
	return this->dispatcherObject;
}

