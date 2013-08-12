/*
 * genfastprefix.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#include "Tokenizer.h"
#include "Process.h"

int main (int argc, char **argv)
{
	Process *process = new Process(argc, argv);
	if ( !process->verifyParams() )
	{
		cout << endl << process->getHelp() << endl;
		delete process;
		return 1;
	}

	int ret = process->execute();
	delete process;

	return ret;
}

