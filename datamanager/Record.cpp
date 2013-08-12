/*
 * record.cpp
 *
 *  Created on: 15/11/2012
 *      Author: fernancoder
 */

#include "include.h"
#include "Record.h"

Record::Record()
{
	this->totalRecords = 0;
}

Record::~Record() {
	this->close();
}

bool Record::open(string indexFilePath, string dataFilePath, int mode)
{
	this->accessMode = mode;

	ios_base::openmode realMode;
	switch (mode)
	{
		case READ_MODE:
			realMode = std::ios_base::in | std::ios_base::binary;
		break;
		case CREATE_MODE:
			remove(indexFilePath.c_str());
			remove(dataFilePath.c_str());
			realMode = std::ios_base::out | std::ios_base::app | std::ios_base::binary;
		break;
	}

	this->indexFh.open (indexFilePath.c_str(), realMode);
	this->dataFh.open (dataFilePath.c_str(), realMode);

	if ( this->indexFh.fail() || this->dataFh.fail() )
	{
		this->close();
		return false;
	}

	if ( mode == READ_MODE )
	{
		this->indexFh.seekg( 0, ios::end );
		this->totalRecords = this->indexFh.tellg()/sizeof(unsigned int);
	}

	return true;
}

unsigned int Record::getSize()
{
	return this->totalRecords;
}

unsigned int Record::pushBackRecord(string recordContent)
{
	if ( this->accessMode == READ_MODE )
		return 0;

	unsigned int recordOffset = this->appendRecord(recordContent);
	return this->appendIndex(recordOffset);
}

string Record::getRecord(unsigned int recordIndex)
{
	if ( this->accessMode == CREATE_MODE)
		return string("");

	if ( recordIndex > this->totalRecords )
		return string("");

	unsigned int recordContentOffset = this->readIndex(recordIndex);
	unsigned int recordLength;
	if ( recordIndex < this->totalRecords )
	{
		recordLength = this->readIndex(recordIndex+1) - recordContentOffset;
	}
	else
	{
		this->dataFh.seekg( 0, ios::end );
		unsigned int fileSize = this->dataFh.tellg();
		recordLength = fileSize - recordContentOffset;
	}

	return this->readRecord(recordContentOffset, recordLength);
}

void Record::close()
{
	if ( this->indexFh.is_open() )
		this->indexFh.close();

	if ( this->dataFh.is_open() )
		this->dataFh.close();
}

unsigned int Record::appendRecord(string recordContent)
{
	unsigned int recordContentOffset = this->dataFh.tellp();
	this->dataFh.write(recordContent.c_str(), recordContent.length());

	this->totalRecords++;

	return recordContentOffset;
}

unsigned int Record::appendIndex(unsigned int recordContentOffset)
{
	this->indexFh.write((char *)&recordContentOffset, sizeof(unsigned int));
	return this->indexFh.tellp()/sizeof(unsigned int)+1;
}

string Record::readRecord(unsigned int recordContentOffset, unsigned int recordContentLength)
{
	char *szBuffer = (char *)malloc((recordContentLength+1) * sizeof(char));

	this->dataFh.seekg(recordContentOffset, ios::beg);
	this->dataFh.read(szBuffer, recordContentLength);

	szBuffer[recordContentLength] = '\0';
	string recordContent = string(szBuffer);
	free(szBuffer);

	return recordContent;
}

unsigned int Record::readIndex(unsigned int recordIndex)
{
	unsigned int indexOffset = (recordIndex-1) * sizeof(unsigned int);
	unsigned int recordContentOffset;

	this->indexFh.seekg(indexOffset, ios::beg);
	this->indexFh.read((char *)&recordContentOffset, sizeof(unsigned int));

	return recordContentOffset;
}
