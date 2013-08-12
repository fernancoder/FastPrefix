/*
 * Process.h
 *
 *  Created on: 14/11/2012
 *      Author: fernando
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "include.h"

const int ORIGEN_TYPE_NONE=0;
const int ORIGEN_TYPE_CSV=1;
const int ORIGEN_TYPE_CONFIG=2;

const int MAX_COMMAND=1024;
const int MAX_FILE_PATH=256;

typedef struct RegIdx
{
	char term[MAX_TERM_SIZE+1];
	unsigned int offset;
} _RegIdx;

class Process {
	public:
		Process(int argc, char **argv);
		virtual ~Process();

		bool verifyParams();
		string getHelp();
		int execute();

	protected:
		int numParams;
		string programName;
		int origenType;
		string filePath;
		string configPath;
		string workPath;
		char fieldSeparator;
		string keyField;
		string weightField;
		long indexLevel1Size;
		long indexLevel2Size;
		long indexLevel3Size;

	protected:
		void generateOrigin();
		void generateData();
		void generateIndex();

		void generateTerms();
		void sortTerms();
		long generateTermIndexAndOcurrences();
		void generateTermIndex(unsigned int totalTerms);
};

#endif /* PROCESS_H_ */
