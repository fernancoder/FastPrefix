/*
 * Ocurrence.h
 *
 *  Created on: 19/11/2012
 *      Author: fernancoder
 */

#ifndef OCURRENCE_H_
#define OCURRENCE_H_

typedef struct OcurrenceInfo
{
	unsigned int id;
	unsigned int weight;
} _OcurrenceInfo;

typedef struct IndexRecord
{
	char term[MAX_TERM_SIZE+1];
	unsigned int offset;
} _IndexRecord;

typedef struct IndexInfo
{
	char term[MAX_TERM_SIZE+1];
	unsigned int offset;
	unsigned int size;
} _IndexInfo;

typedef struct IntervalInfo
{
	char leftTerm[MAX_TERM_SIZE+1];
	char rightTerm[MAX_TERM_SIZE+1];
	bool isExactTerm;
	unsigned int offset;
	unsigned int size;
} _IntervalInfo;

class Ocurrence {
	public:
		Ocurrence();
		virtual ~Ocurrence();

		bool open(string indexFilePath, string ocurrenceFilePath, int level1Size, int level2Size, int level3Size);
		void close();

		vector<OcurrenceInfo *> *findPrefix(string term);
		vector<OcurrenceInfo *> *findTerm(string term);
		vector<OcurrenceInfo *> *operateAnd(vector<OcurrenceInfo *> *op1, vector<OcurrenceInfo *> *op2);
		void deleteOcurrenceInfo(vector<OcurrenceInfo *> *vctrOcurrenceInfo);

	private:
		IntervalInfo getIntervalTerms(string term, bool isExactTerm);

		IndexInfo getLeftTerm(string term);
		IndexInfo getRightTerm(string term);
		IndexInfo getNearestTerm(string term);

		vector<OcurrenceInfo *> *getOcurrences(IntervalInfo *intervalInfo);

	private:
		void ClearBlockContent(vector<IndexRecord *> *blockContent);



	private:
		unsigned int totalTerms;
		int level1Size;
		int level2Size;
		int level3Size;

		fstream indexFh;
		fstream ocurrenceFh;

		vector<IndexRecord *> level1BlockContent;

};

#endif /* OCURRENCE_H_ */
