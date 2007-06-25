
#include "DirectoryVersion.h"
#include "debug.h"

using std::list;
using std::string;

DirectoryVersion::DirectoryVersion()
{
	_fileLst.clear();
	_user = 0;
	_nroVersion = -1;
	_type = MODIFICACION;
}

DirectoryVersion::DirectoryVersion(int NroVersion,const char* User,tm Date,t_versionType Type)
{
	_fileLst.clear();
	int tam = strlen(User);
	_nroVersion = NroVersion;
	_user = new char[(tam + 1) * sizeof(char)];
	memcpy(_user,User,tam);
	_user[tam] = 0;
	_date = Date;
	_type = Type;
}

DirectoryVersion::~DirectoryVersion()
{
	_fileLst.clear();
	
	if (_user)
		delete _user;
}

void DirectoryVersion::addFile(const char* fileName, int versionNumber,char type)
{
	File* newFile = new File(fileName, versionNumber,type);

	_fileLst.push_back(*newFile);
}

void DirectoryVersion::write(char* buffer)
{
	memcpy(buffer,&_nroVersion,sizeof(int));	// version number
	buffer += sizeof(int);

	int length = strlen(_user);
	memcpy(buffer,&length,sizeof(int));		//length of user
	buffer += sizeof(int);

	memcpy(buffer,_user,length * sizeof(char));	//user
	buffer += sizeof(char) * length;

	memcpy(buffer,&_date,sizeof(tm));	//date
	buffer += sizeof(tm);

	memcpy(buffer,&_type,sizeof(t_versionType));	//type
	buffer += sizeof(t_versionType);

	int files = _fileLst.size();
	memcpy(buffer,&files,sizeof(int));	//size of list
	buffer += sizeof(int);

	list<File>::iterator it;

	int offset = 0;

	for (it = _fileLst.begin();it != _fileLst.end(); ++it) {
		it->write(buffer + offset);	// each file
		offset += it->getTamanioEnDisco();
	}
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

	memcpy(&_type,*buffer,sizeof(t_versionType));	//type
	*buffer += sizeof(t_versionType);

	_fileLst.clear();

	int files;
	memcpy(&files,*buffer,sizeof(int));				//cantidad de archivos
	*buffer += sizeof(int);

	for (int i = 0; i < files; ++i) {
		File* newFile = new File();					//read each file
		newFile->read(buffer);
		_fileLst.push_back(*newFile);
	}
}

long int DirectoryVersion::tamanioEnDisco()
{
	long int size = 0;

	size += sizeof(int);	//_versionNumber
	size += sizeof(int);	//el indicador de longitud del usuario
	int length = strlen(_user);
	size += length * sizeof(char);	//_user
	size += sizeof(tm);	//_date
	size += sizeof(t_versionType);	//_type
	size += sizeof(int);	//list size
	list<File>::iterator it;
	for (it = _fileLst.begin();it != _fileLst.end(); ++it)
		size += it->getTamanioEnDisco();		//each element of the list

	return size;
}

void DirectoryVersion::update(const char* fileName, int versionNumber, char type)
{
	list<File>::iterator it;

	bool modified = false;

	for (it = _fileLst.begin(); it != _fileLst.end(); ++it) {
		int cmp = strcmp(it->getName(),fileName);
		if ((cmp == 0) && (it->getType() == type)) {
			it->setVersion(versionNumber);
			modified = true;
		}
	}
	
	if (!modified)
        addFile(fileName, versionNumber, type);
}

bool DirectoryVersion::searchFile(const char* filename, File** file)
{
	list<File>::iterator it;

	for (it = _fileLst.begin(); it != _fileLst.end(); ++it) {
		int cmp = strcmp(filename, it->getName());
		if(cmp == 0) {
			*file = new File(filename, it->getVersion(),it->getType());
			return true;
		}
	}
	return false;
}

bool DirectoryVersion::searchFile(const char* filename)
{
	list<File>::iterator it;

	for (it = _fileLst.begin(); it != _fileLst.end(); ++it) {
		int cmp = strcmp(filename, it->getName());
		if(cmp == 0)
			return true;
	}
	return false;
}
