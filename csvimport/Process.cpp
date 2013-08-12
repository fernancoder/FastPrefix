/*
 * Process.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#include "Tokenizer.h"
#include "Process.h"
#include "Util.h"
#include "Record.h"


Process::Process(int argc, char **argv)
{
	this->numParams = argc;
	this->programName = argv[0];
	this->origenType = ORIGEN_TYPE_NONE;
	this->filePath = "";
	this->configPath = "";
	this->workPath = "";
	this->fieldSeparator = '|';
	this->keyField = "title";
	this->weightField = "weight";
	this->indexLevel1Size = 128;
	this->indexLevel2Size = 128;
	this->indexLevel3Size = 128;

	for ( int i=1; i< argc; i++ )
	{
		if ( strcmp(argv[i],"-o") == 0 || strcmp(argv[i],"-origen") == 0)
		{
			if ( argc > i+1 && strcmp(argv[i+1],"csv") == 0 )
			{
				this->origenType = ORIGEN_TYPE_CSV;
			}
			else if ( argc > i+1 && strcmp(argv[i+1],"config") == 0 )
			{
				this->origenType = ORIGEN_TYPE_CONFIG;
			}
		}
		else if  ( strcmp(argv[i],"-f") == 0 || strcmp(argv[i],"-file") == 0)
		{
			if ( argc > i+1 )
				this->filePath = string(argv[i+1]);
		}
		else if  ( strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"-config") == 0)
		{
			if ( argc > i+1 )
				this->configPath = string(argv[i+1]);
		}
		else if  ( strcmp(argv[i],"-w") == 0 || strcmp(argv[i],"-work") == 0)
		{
			if ( argc > i+1 )
				this->workPath = string(argv[i+1]);
		}
	}
}

Process::~Process()
{
	// TODO Auto-generated destructor stub
}

bool Process::verifyParams()
{
	if ( this->numParams != 7 )
		return false;

	if ( this->origenType == ORIGEN_TYPE_CSV )
	{
		if ( this->filePath.empty() )
			return false;
	}
	else if ( this->origenType == ORIGEN_TYPE_CONFIG )
	{
		if ( this->configPath.empty() )
			return false;
	}
	else
		return false;

	return true;
}

string Process::getHelp()
{
	string response = "Uso: " + programName;
	response += " -o[rigen] ORIGEN TYPE [-f[ile] FILE PATH] [-c[onfig] CONFIG PATH] -w[ork] WORK PATH [-f[field-separator] FIELD SEPARATOR] [-k[ey-field] KEY FIELD] [-r[elevance] WEIGHT FIELD]\n";
	response += "\tORIGEN TYPE: modo de acceso al origen de datos\n";
	response += "\t\tconfig\n";
	response += "\t\tcsv\n";
	response += "\tFILE PATH: ruta fichero con los datos de entrada (-origen csv)\n";
	response += "\tCONFIG PATH: ruta fichero configuracion (-origen config)\n";
	response += "\tWORK PATH: directorio temporal\n";
	response += "\tFIELD SEPARATOR: caracter separador entre campos (defecto |)\n";
	response += "\tKEY FIELD: campo búsqueda (defecto title)\n";
	response += "\tWEIGHT FIELD: campo peso (defecto weight)\n";

	return string(response);
}

int Process::execute()
{
	this->generateOrigin();
	this->generateData();
	this->generateIndex();

	return 1;
}

void Process::generateOrigin()
{
	if ( this->origenType == ORIGEN_TYPE_CSV )
	{
		char command[MAX_COMMAND];
		strcpy(command,"cp ");
		strcat(command,this->filePath.c_str());
		strcat(command," ");
		strcat(command,this->workPath.c_str());
		strcat(command,"/fastprefix.fase1");
		std::system(command);
	}
}

void Process::generateData()
{
	char orgFilePath[MAX_FILE_PATH];
	strcpy(orgFilePath,this->workPath.c_str());
	strcat(orgFilePath,"/fastprefix.fase1");

	char idxFilePath[MAX_FILE_PATH];
	strcpy(idxFilePath,this->workPath.c_str());
	strcat(idxFilePath,"/fastprefix.idx");

	char regFilePath[MAX_FILE_PATH];
	strcpy(regFilePath,this->workPath.c_str());
	strcat(regFilePath,"/fastprefix.dat");

	ifstream ifOrg;
	ifOrg.open (orgFilePath,ios_base::in);
	string line;

	Record *record = new Record();
	record->open(idxFilePath, regFilePath, CREATE_MODE);

	string fieldNames;
	getline(ifOrg, fieldNames, '\n');
	vector<string> vFieldNames = Util::split(fieldNames, this->fieldSeparator);

	while (getline(ifOrg, line, '\n'))
	{
		string rec = Util::jsonizeString(&vFieldNames, Util::escape(line), this->fieldSeparator);
		record->pushBackRecord(rec);
	}

	record->close();

	delete record;
}

void Process::generateIndex()
{
	this->generateTerms();
	this->sortTerms();
	long totalTerms = this->generateTermIndexAndOcurrences();
	this->generateTermIndex(totalTerms);
}


void Process::generateTerms()
{
	Record *record = new Record();
	Tokenizer *tokenizer = new Tokenizer();

	char idxFilePath[MAX_FILE_PATH];
	strcpy(idxFilePath,this->workPath.c_str());
	strcat(idxFilePath,"/fastprefix.idx");

	char regFilePath[MAX_FILE_PATH];
	strcpy(regFilePath,this->workPath.c_str());
	strcat(regFilePath,"/fastprefix.dat");

	char phase2FilePath[MAX_FILE_PATH];
	strcpy(phase2FilePath,this->workPath.c_str());
	strcat(phase2FilePath,"/fastprefix.fase2");
	ofstream offase2;
	offase2.open (phase2FilePath,ios_base::out);

	record->open(idxFilePath, regFilePath);

	long totalRecords = record->getSize();

	tokenizer->init("stop.txt");

	string recordContent;
	for ( long idx=1; idx <= totalRecords; idx++)
	{
		recordContent = record->getRecord(idx);
		map<string, string> recordParts = Util::desjsonize(recordContent);

		vector<string> *tokens = tokenizer->extractWordsAndNormalize(recordParts[this->keyField]);
		for(vector<string>::const_iterator it=tokens->begin(); it!=tokens->end(); it++)
		{
			offase2 << right << setw(8) << idx << '|';
			offase2 << left << setw(MAX_TERM_SIZE) << *it << '|';
			offase2 << right << setw(10) << recordParts[this->weightField] << '|';
			offase2 << endl;
		}
	}

	record->close();
	offase2.close();

	delete tokenizer;
	delete record;
}


void Process::sortTerms()
{
	char command[MAX_COMMAND];
	strcpy(command,"cat ");
	strcat(command,this->workPath.c_str());
	strcat(command,"/fastprefix.fase2 | LC_ALL=es_ES.ISO-8859-1 sort -t'|' -k 2,2 -k 1,1n > ");
	strcat(command,this->workPath.c_str());
	strcat(command,"/fastprefix.fase3");
	std::system(command);
}

long Process::generateTermIndexAndOcurrences()
{
	char phase3FilePath[MAX_FILE_PATH];
	strcpy(phase3FilePath,this->workPath.c_str());
	strcat(phase3FilePath,"/fastprefix.fase3");
	ifstream iffase3;
	iffase3.open (phase3FilePath,ios_base::in);

	char phase4FilePath[MAX_FILE_PATH];
	strcpy(phase4FilePath,this->workPath.c_str());
	strcat(phase4FilePath,"/fastprefix.fase4");
	ofstream offase4;
	offase4.open (phase4FilePath,ios_base::out);

	char ocurFilePath[MAX_FILE_PATH];
	strcpy(ocurFilePath,this->workPath.c_str());
	strcat(ocurFilePath,"/fastprefix.ocr");
	ofstream ofOcur;
	ofOcur.open (ocurFilePath, ios_base::out | ios_base::binary);

	string termAnt = string("");
	string line;
	long ocurCount = 0;
	long ocurPosition = 0;
	long termCount = 0;

	while ( getline(iffase3, line, '\n') )
	{
		vector<string> lineParts = Util::split(line, '|');

		if ( lineParts[1].compare(termAnt) != 0 )
		{
			if ( !termAnt.empty() )
			{
				offase4 << termAnt << "|" << ocurPosition << endl;
				termCount++;
				ocurPosition += ocurCount;
				ocurCount = 0;
			}

			termAnt = lineParts[1];
		}

		ocurCount++;

		unsigned int idx = atol(lineParts[0].c_str());
		unsigned int weight = atol(lineParts[2].c_str());

		ofOcur.write((char *)&idx, sizeof(unsigned int));
		ofOcur.write((char *)&weight, sizeof(unsigned int));
	}

	offase4 << termAnt << "|" << ocurPosition << endl;
	termCount++;

	iffase3.close();
	offase4.close();
	ofOcur.close();

	return termCount;
}

void Process::generateTermIndex(unsigned int totalTerms)
{
	char phase4FilePath[MAX_FILE_PATH];
	strcpy(phase4FilePath,this->workPath.c_str());
	strcat(phase4FilePath,"/fastprefix.fase4");
	ifstream iffase4;
	iffase4.open (phase4FilePath,ios_base::in);

	char termIdxFilePath[MAX_FILE_PATH];
	strcpy(termIdxFilePath,this->workPath.c_str());
	strcat(termIdxFilePath,"/fastprefix.idt");
	ofstream ofTermIdx;
	ofTermIdx.open (termIdxFilePath, ios_base::out | ios_base::binary);

	ofTermIdx.write((char *)&totalTerms,sizeof(unsigned int));

	RegIdx regIdx;
	int curLevel3Position = 0;
	int curLevel2Position = 0;

	long seekLevel1 = sizeof(unsigned int);
	long seekLevel2 = seekLevel1 + this->indexLevel1Size * sizeof(RegIdx);
	long seekLevel3 = seekLevel2 + ((((totalTerms / this->indexLevel3Size) + 1)/this->indexLevel2Size) +1) * this->indexLevel2Size * sizeof(RegIdx);

	string line;
	vector<string> lineParts;

	ofTermIdx.seekp(seekLevel3, ios_base::beg);

	string trimedTerm;

	while ( getline(iffase4, line, '\n') )
	{
		lineParts = Util::split(line, '|');

		trimedTerm  = lineParts[0];
		Util::rtrim(trimedTerm);

		strcpy(regIdx.term, trimedTerm.c_str());
		regIdx.offset = atol(lineParts[1].c_str());
		ofTermIdx.write((char *)&regIdx,sizeof(RegIdx));

		seekLevel3 += sizeof(RegIdx);
		curLevel3Position++;


		if ( curLevel3Position >= this->indexLevel3Size )
		{

			strcpy(regIdx.term, trimedTerm.c_str());
			regIdx.offset = 0;

			ofTermIdx.seekp(seekLevel2, ios_base::beg);
			ofTermIdx.write((char *)&regIdx,sizeof(RegIdx));

			seekLevel2 += sizeof(RegIdx);
			curLevel2Position++;
			curLevel3Position = 0;

			if ( curLevel2Position  >= this->indexLevel2Size )
			{
				strcpy(regIdx.term, trimedTerm.c_str());
				regIdx.offset = 0;

				ofTermIdx.seekp(seekLevel1, ios_base::beg);
				ofTermIdx.write((char *)&regIdx,sizeof(RegIdx));

				seekLevel1 += sizeof(RegIdx);
				curLevel2Position = 0;
			}

			ofTermIdx.seekp(seekLevel3, ios_base::beg);
		}
	}

	if ( curLevel3Position != 0 )
	{
		strcpy(regIdx.term, trimedTerm.c_str());
		regIdx.offset = 0;

		ofTermIdx.seekp(seekLevel2, ios_base::beg);
		ofTermIdx.write((char *)&regIdx,sizeof(RegIdx));

		regIdx.offset = 0;

		ofTermIdx.seekp(seekLevel1, ios_base::beg);
		ofTermIdx.write((char *)&regIdx,sizeof(RegIdx));
	}

	iffase4.close();
	ofTermIdx.close();
}
