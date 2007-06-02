#include "DirectoryVersion.h"

using std::list;

using namespace std;

DirectoryVersion::DirectoryVersion()
{
	_fileLst.clear();
	_user = 0;
	_nroVersion = -1;
}

DirectoryVersion::DirectoryVersion(int NroVersion,const char* User,tm Date)
{
	_fileLst.clear();
	
	int tam = strlen(User);

	_nroVersion = NroVersion;

	_user = new char[(tam + 1) * sizeof(char)];
	strcpy(_user,User);

	_date = Date;	
}

DirectoryVersion::~DirectoryVersion()
{
	_fileLst.clear();
	
	if(_user)
		delete _user;
}

void DirectoryVersion::addFile(char* fileName,int versionNumber,char type)
{
	File* newFile = new File(fileName,versionNumber,type);

	_fileLst.push_back(*newFile);
}

void DirectoryVersion::write(char** buffer)
{
	memcpy(*buffer,&_nroVersion,sizeof(int));	// version number
	*buffer += sizeof(int);

	int length = strlen(_user);
	memcpy(*buffer,&length,sizeof(int));		//length of user
	*buffer += sizeof(int);

	memcpy(*buffer,_user,length * sizeof(char));	//user
	*buffer += sizeof(char) * length;

	memcpy(*buffer,&_date,sizeof(tm));	//date
	*buffer += sizeof(tm);

	int files = _fileLst.size();
	memcpy(*buffer,&files,sizeof(int));	//size of list
	*buffer += sizeof(int);

	list<File>::iterator it;

	for(it = _fileLst.begin();it != _fileLst.end();it++)
		it->write(buffer);	// each file
}

void DirectoryVersion::read(char** buffer)
{
	memcpy(&_nroVersion,*buffer,sizeof(int));	//version number
	*buffer += sizeof(int);

	int length;			
	memcpy(&length,*buffer,sizeof(int));		//length of user
	*buffer += sizeof(int);

	if(_user)
		delete _user;

	_user = new char[(length + 1) * sizeof(char)];
	memcpy(_user,*buffer,sizeof(char) * length);	//user
	_user[length] = 0;
	*buffer += sizeof(char) * length;

	memcpy(&_date,*buffer,sizeof(tm));				//date
	*buffer += sizeof(tm);

	_fileLst.clear();

	int files;
	memcpy(&files,*buffer,sizeof(int));				//cantidad de archivos
	*buffer += sizeof(int);

	for(int i = 0; i < files;i++)
	{
		File* newFile = new File();					//read each file

		newFile->read(buffer);

		_fileLst.push_back(*newFile);
	}
}

long int DirectoryVersion::tamanioEnDisco()
{
	long int size = 0;

	list<File>::iterator it;

	for(it = _fileLst.begin();it != _fileLst.end();it++)
		size += it->getTamanioEnDisco();		//tamanio de cada file

	size += sizeof(int);	//el indicador de longitud del usuario

	int length = strlen(_user);

	size += length * sizeof(char);	//_user

	size += sizeof(tm);	//_date

	size += sizeof(int);	//_versionNumber

	return size;
}
