// DirectoryVersion.h

#ifndef DIRECTORY_VERSION_H_INCLUDED
#define DIRECTORY_VERSION_H_INCLUDED

#include "File.h"

#include <list>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iostream>

class DirectoryVersion
{
public:

	enum t_versionType { MODIFICACION = 0, BORRADO};

	DirectoryVersion();
	DirectoryVersion(int NroVersion,const char* User,tm Date,t_versionType Type);

	~DirectoryVersion();

	void read(char** buffer);
	void write(char* buffer);

	std::list<File>*	getFilesList()	{	return	&_fileLst;		}
	
	int					getNroVersion()	{	return	_nroVersion;	}
	char*				getUser()		{	return	_user;			}
	tm					getDate()		{	return	_date;			}
 	size_t				getCantFile()	{	return	_fileLst.size();}
	t_versionType		getType()		{	return _type;			}

	void addFile(const char* fileName,int versionNumber,char type);
	void update(const char* fileName,int versionNumber,char type);

	long int tamanioEnDisco();

	bool searchFile(const char* filename,File** file);

private:
	int					_nroVersion;	// version number
	char*				_user;			// user
	tm					_date;			// date
	std::list<File>		_fileLst;		// list of filenames
	t_versionType		_type;			// version type
};

#endif
