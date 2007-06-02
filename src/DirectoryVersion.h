#ifndef DIRECTORYVERSION_H_INCLUDED
#define DIRECTORYVERSION_H_INCLUDED

#include <list>
#include <string>
#include <ctime>
#include <stdlib.h>
#include <iostream>
#include "file.h"

class DirectoryVersion{
public:
	DirectoryVersion();
	DirectoryVersion(int NroVersion,const char* User,tm Date);

	~DirectoryVersion();

	void read(char** buffer);
	void write(char** buffer);

	std::list<File>*	getFilesList()	{	return	&_fileLst;		}
	
	int					getNroVersion()	{	return	_nroVersion;	}
	char*				getUser()		{	return	_user;			}
	tm					getDate()		{	return	_date;			}
	int					getCantFile()	{	return	_fileLst.size();}

	void addFile(char* fileName,int versionNumber,char type);

	long int tamanioEnDisco();

private:
	int					_nroVersion;	//version number
	char*				_user;			//user
	tm					_date;			//date
	std::list<File>		_fileLst;		// list of filenames
};

#endif