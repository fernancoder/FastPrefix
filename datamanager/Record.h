/*
 * record.h
 *
 *  Created on: 15/11/2012
 *      Author: fernancoder
 */

#ifndef RECORD_H_
#define RECORD_H_

const int READ_MODE=0;
const int CREATE_MODE=1;

class Record {
	public:
		Record();
		virtual ~Record();

		bool open(string indexFilePath, string dataFilePath, int mode = READ_MODE);
		unsigned int pushBackRecord(string recordContent);
		string getRecord(unsigned int recordIndex);
		unsigned int getSize();
		void close();

	private:
		unsigned int appendRecord(string recordContent);
		unsigned int appendIndex(unsigned int recordContentOffset);

		string readRecord(unsigned int recordContentOffset, unsigned int recordContentLength);
		unsigned int readIndex(unsigned int recordIndex);

	private:
		int accessMode;
		fstream indexFh;
		fstream dataFh;
		unsigned int totalRecords;
};

#endif /* RECORD_H_ */
