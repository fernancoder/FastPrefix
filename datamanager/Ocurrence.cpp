/*
 * Ocurrence.cpp
 *
 *  Created on: 19/11/2012
 *      Author: fernancoder
 */

#include "Tokenizer.h"
#include "Ocurrence.h"

Ocurrence::Ocurrence()
{


}

Ocurrence::~Ocurrence()
{
	this->ClearBlockContent(&this->level1BlockContent);
	this->close();
}

bool Ocurrence::open(string indexFilePath, string ocurrenceFilePath, int level1Size, int level2Size, int level3Size)
{
	this->level1Size = level1Size;
	this->level2Size = level2Size;
	this->level3Size = level3Size;

	this->indexFh.open (indexFilePath.c_str(), std::ios_base::in | std::ios_base::binary);
	this->ocurrenceFh.open (ocurrenceFilePath.c_str(), std::ios_base::in | std::ios_base::binary);

	if ( this->indexFh.fail() || this->ocurrenceFh.fail() )
	{
		this->close();
		return false;
	}

	this->indexFh.read((char *)&this->totalTerms, sizeof(unsigned int));

	unsigned int level3Blocks = floor(this->totalTerms/this->level3Size) +1;
	unsigned int level2Blocks = floor(level3Blocks/this->level2Size) +1;

	for ( unsigned int idx=0; idx < level2Blocks; idx++ )
	{
		IndexRecord *indexRecord = new IndexRecord();
		this->indexFh.read((char *)indexRecord, sizeof(IndexRecord));
		this->level1BlockContent.push_back(indexRecord);
	}

	return true;
}

void Ocurrence::close()
{
	if ( this->indexFh.is_open() )
		this->indexFh.close();

	if ( this->ocurrenceFh.is_open() )
		this->ocurrenceFh.close();
}

vector<OcurrenceInfo *> *Ocurrence::findPrefix(string term)
{
	IntervalInfo intervalInfo = this->getIntervalTerms(term, false);
	return this->getOcurrences(&intervalInfo);
}

vector<OcurrenceInfo *> *Ocurrence::findTerm(string term)
{
	IntervalInfo intervalInfo = this->getIntervalTerms(term, true);
	if ( intervalInfo.isExactTerm )
	{
		return this->getOcurrences(&intervalInfo);
	}

	return NULL;
}

IntervalInfo Ocurrence::getIntervalTerms(string term, bool isExactTerm)
{
	IntervalInfo intervalInfo;
	intervalInfo.isExactTerm = false;
	strcpy(intervalInfo.leftTerm,"");
	strcpy(intervalInfo.rightTerm,"");
	intervalInfo.offset = 0;
	intervalInfo.size = 0;

	IndexInfo leftIndexInfo = this->getLeftTerm(term);
	strcpy(intervalInfo.leftTerm, leftIndexInfo.term);
	intervalInfo.offset = leftIndexInfo.offset;

	if ( isExactTerm )
	{
		if ( !term.compare(leftIndexInfo.term) )
		{
			strcpy(intervalInfo.rightTerm, leftIndexInfo.term);
			intervalInfo.size = leftIndexInfo.size;
			intervalInfo.isExactTerm = true;

			return intervalInfo;
		}
		else
			return intervalInfo;
	}

	IndexInfo rightIndexInfo = this->getRightTerm(term);
	strcpy(intervalInfo.rightTerm, rightIndexInfo.term);
	intervalInfo.size = (rightIndexInfo.offset - leftIndexInfo.offset);

	return intervalInfo;
}

IndexInfo Ocurrence::getLeftTerm(string term)
{
	return this->getNearestTerm(term);
}

IndexInfo Ocurrence::getRightTerm(string term)
{
	term += 0xFF;
	return this->getNearestTerm(term);
}

IndexInfo Ocurrence::getNearestTerm(string term)
{
	unsigned int level2Block = 0;

	IndexInfo indexInfo;
	strcpy(indexInfo.term,"");
	indexInfo.offset = 0;
	indexInfo.size = 0;

	for(vector<IndexRecord *>::const_iterator it=this->level1BlockContent.begin(); it!=this->level1BlockContent.end(); it++)
	{
		//cout << "LEVEL-1 " << (*it)->term << endl;;
		if ( term.compare((*it)->term) <= 0 )
		{
			break;
		}

		level2Block++;
	}

	this->indexFh.seekg(sizeof(unsigned int) +
			            this->level1Size * sizeof(IndexRecord) +
			            level2Block * this->level2Size * sizeof(IndexRecord), ios_base::beg);

	unsigned int level3Block = 0;

	for ( int idx=0; idx < this->level2Size; idx++ )
	{
		IndexRecord indexRecord;
		this->indexFh.read((char *)&indexRecord, sizeof(IndexRecord));
		//cout << "LEVEL-2 " << indexRecord.term << endl;

		if ( term.compare(indexRecord.term) <= 0 )
		{
			break;
		}

		level3Block++;
	}

	this->indexFh.seekg(sizeof(unsigned int) +
			            this->level1Size * sizeof(IndexRecord) +
			            this->level1BlockContent.size() * this->level2Size * sizeof(IndexRecord) +
			            level2Block * this->level2Size * this->level3Size * sizeof(IndexRecord) +
			            level3Block * this->level3Size * sizeof(IndexRecord), ios_base::beg);

	for ( int idx=0; idx < this->level3Size; idx++ )
	{
		IndexRecord indexRecord;
		this->indexFh.read((char *)&indexRecord, sizeof(IndexRecord));
		//cout << "LEVEL-3 " << indexRecord.term << endl;

		if ( term.compare(indexRecord.term) <= 0 )
		{
			strcpy(indexInfo.term, indexRecord.term);
			indexInfo.offset = indexRecord.offset;

			IndexRecord nextIndexRecord;
			this->indexFh.read((char *)&nextIndexRecord, sizeof(IndexRecord));
			indexInfo.size = (nextIndexRecord.offset - indexRecord.offset);

			break;
		}
	}

	return indexInfo;
}

vector<OcurrenceInfo *> *Ocurrence::getOcurrences(IntervalInfo *intervalInfo)
{
	map<int, OcurrenceInfo *> mapOcurrenceInfo;

	this->ocurrenceFh.seekg(intervalInfo->offset*sizeof(OcurrenceInfo), ios_base::beg);
	for ( unsigned int idx=0; idx < intervalInfo->size; idx++ )
	{
		OcurrenceInfo *ocurrenceInfo = new OcurrenceInfo();
		this->ocurrenceFh.read((char *)ocurrenceInfo, sizeof(OcurrenceInfo));
		if ( mapOcurrenceInfo.find(ocurrenceInfo->id) != mapOcurrenceInfo.end() )
		{
			delete ocurrenceInfo;
		}
		else
		{
			mapOcurrenceInfo[ocurrenceInfo->id] = ocurrenceInfo;
		}
	}

	vector<OcurrenceInfo *> *vctrOcurrenceInfo = new vector<OcurrenceInfo *>;
	for ( map<int, OcurrenceInfo *>::const_iterator itr = mapOcurrenceInfo.begin(); itr != mapOcurrenceInfo.end(); ++itr )
	{
		vctrOcurrenceInfo->push_back((*itr).second);
	}

	return vctrOcurrenceInfo;
}


//UTILIDADES
void Ocurrence::ClearBlockContent(vector<IndexRecord *> *blockContent)
{
	for(vector<IndexRecord *>::const_iterator it=blockContent->begin(); it!=blockContent->end(); it++)
	{
		delete (*it);
	}
}

void Ocurrence::deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo)
{
	for(vector<OcurrenceInfo *>::const_iterator it=vctrOcurrenceInfo->begin(); it!=vctrOcurrenceInfo->end(); it++)
		delete *it;
	delete vctrOcurrenceInfo;
}

vector<OcurrenceInfo *> *Ocurrence::operateAnd(vector<OcurrenceInfo *> *op1, vector<OcurrenceInfo *> *op2)
{
	vector<OcurrenceInfo *> *result = new vector<OcurrenceInfo *>;

	vector<OcurrenceInfo *>::const_iterator ocuOp1=op1->begin();
	vector<OcurrenceInfo *>::const_iterator ocuOp2=op2->begin();

	while ( ocuOp1 != op1->end() && ocuOp2 != op2->end() )
	{
		if ( (*ocuOp1)->id == (*ocuOp2)->id )
		{
			OcurrenceInfo *wrkOcurrenceInfo = new OcurrenceInfo();
			wrkOcurrenceInfo->id = (*ocuOp1)->id;
			wrkOcurrenceInfo->weight = (*ocuOp1)->weight;
			result->push_back(wrkOcurrenceInfo);
			ocuOp1++;
			ocuOp2++;
		}
		else if ( (*ocuOp1)->id <  (*ocuOp2)->id )
			ocuOp1++;
		else
			ocuOp2++;
	}

	return result;
}

