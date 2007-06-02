#include "file.h"

File::File()
{
	_name = 0;
	_versionNumber = -1;
	_type = 0;
}

File::File(char* name, int versionNumber,char type)
{
	_type = type;
	_versionNumber = versionNumber;

	int tam = strlen(name);
	_name = (char*) malloc((tam+1) * sizeof(char));
	strcpy(_name,name);
}

File::~File()
{
	if(_name)
		delete _name;
}

int File::getTamanioEnDisco()
{
	int ret = 0;

	ret += sizeof(int);				//indicador de longitud del nombre
	
	int length = strlen(_name);
	
	ret += length * sizeof(char);	//el nombre

	ret += sizeof(int);		//version
	
	ret += sizeof(char);			//tipo

	return ret;
}

void File::write(char** buffer)
{
	int length = strlen(_name);

	memcpy(*buffer,&length,sizeof(int));	//indicador del longitud de _name
	*buffer += sizeof(int);

	memcpy(*buffer,_name,length * sizeof(char));	// _name
	*buffer += length * sizeof(char);

	memcpy(*buffer,&_versionNumber,sizeof(int));	//_versionNumber
	*buffer += sizeof(int);

	memcpy(*buffer,&_type,sizeof(char));	//_type
	*buffer += sizeof(char);
}

void File::read(char** buffer)
{
	int length;

	memcpy(&length,*buffer,sizeof(int));	//indicador de longitud de _name
	*buffer += sizeof(int);

	if(_name)
		delete _name;

	_name = (char*) malloc((length + 1) * sizeof(char)); //inicializo _name
	memcpy(_name,*buffer,length * sizeof(char));	//_name
	_name[length] = 0;
	*buffer += length * sizeof(char);

	memcpy(&_versionNumber,*buffer,sizeof(int));	//_versionNumber
	*buffer += sizeof(int);

	memcpy(&_type,*buffer,sizeof(char));	//_type
	*buffer += sizeof(char);
}