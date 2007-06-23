// FileBlock.cpp

#include "FileBlock.h"
#include "debug.h"

#include <iostream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const int FileBlock::TAMANIO_BLOQUE_ARCHIVOS = 1024;

FileBlock::FileBlock(int Numero,int Anterior,int Siguiente)
{
	_espacioLibre = TAMANIO_ARREGLO_BLOQUE_ARCHIVOS;
	_cantVersiones = 0;
	_numero = Numero;
	_anterior = Anterior;
	_siguiente = Siguiente;
	_used = TAMANIO_ARREGLO_BLOQUE_ARCHIVOS - _espacioLibre;
	_actualOffset = 0;
}

FileBlock::~FileBlock()
{
}

bool FileBlock::insertVersion(FileVersion* version)
{
	char* nextByte = _versiones + _used;
	_espacioLibre -= version->tamanioEnDisco();
	version->write(nextByte);
	_cantVersiones++;
	_used = (TAMANIO_ARREGLO_BLOQUE_ARCHIVOS - _espacioLibre);
	_actualOffset = _used;
    return true;
}

void FileBlock::read(char* buffer)
{
	int offset = 0;

	//numero del bloque
	memcpy(&_numero,buffer + offset, sizeof(int));
	offset += sizeof(int);

	//anterior
	memcpy(&_anterior,buffer + offset, sizeof(int));	
	offset += sizeof(int);

	//siguiente
	memcpy(&_siguiente,buffer + offset, sizeof(int));
	offset += sizeof(int);

	//espacio libre
	memcpy(&_espacioLibre,buffer + offset, sizeof(int));
	offset += sizeof(int);

	_used = TAMANIO_ARREGLO_BLOQUE_ARCHIVOS - _espacioLibre;

	//cantidad de versiones
	memcpy(&_cantVersiones,buffer + offset, sizeof(int));
	offset += sizeof(int);

	//el arreglo con las versiones
	memcpy(_versiones,buffer + offset,TAMANIO_ARREGLO_BLOQUE_ARCHIVOS);
	offset += TAMANIO_ARREGLO_BLOQUE_ARCHIVOS;

	return;
}

void FileBlock::write(char* buffer)
{
	int offset = 0;

	//numero del bloque
	memcpy(buffer + offset, &_numero, sizeof(int));
	offset += sizeof(int);

	//anterior
	memcpy(buffer + offset, &_anterior, sizeof(int));
	offset += sizeof(int);

	//siguiente
	memcpy(buffer + offset, &_siguiente, sizeof(int));
	offset += sizeof(int);

	//espacio libre
	memcpy(buffer + offset, &_espacioLibre, sizeof(int));
	offset += sizeof(int);

	//cantidad de versiones
	memcpy(buffer + offset, &_cantVersiones, sizeof(int));
	offset += sizeof(int);

	//arreglo con las versiones
	memcpy(buffer + offset,_versiones, TAMANIO_ARREGLO_BLOQUE_ARCHIVOS);
	offset += TAMANIO_ARREGLO_BLOQUE_ARCHIVOS;

	return;
}

bool FileBlock::hayLugar(FileVersion* version)
{
	return (_espacioLibre >= version->tamanioEnDisco());
}

bool FileBlock::searchVersion(int nro,FileVersion** version)
{
	char* nextByte = _versiones;

	for (int i = 0;i < _cantVersiones; ++i) {
		*version = new FileVersion();
		
		(*version) ->read(&nextByte);
		_actualOffset = nextByte - _versiones;
		
		if ((*version)->getNroVersion() == nro){
			return true;
		}

		delete (*version);
	}

	return false;
}

bool FileBlock::searchVersion(int nro)
{
	char* nextByte = _versiones;

	for (int i = 0;i < _cantVersiones; ++i) {
		
		FileVersion* version = new FileVersion();
		version->read(&nextByte);

		_actualOffset = nextByte - _versiones;
		
		if(version->getNroVersion() == nro) {
			delete version;
			return true;
		}

		delete version;
	}

	return false;
}

FileVersion* FileBlock::getLastVersion()
{
	FileVersion* ret = new FileVersion();
	char* nextByte = _versiones;
	int i = 0;
	while (i < _cantVersiones)
	{
		ret->read(&nextByte);
		_actualOffset = nextByte - _versiones;
		i++;
	}
	
	return ret;
}

int FileBlock::getFirstVersionNumber()
{
	FileVersion* version = new FileVersion();
	char* nextByte = _versiones;	
	version->read(&nextByte);
	_actualOffset = nextByte - _versiones;
	int ret = version->getNroVersion();
	delete version;
	return ret;
}

void FileBlock::moveFirst()
{
	_actualOffset = 0;
	return;
}

FileVersion* FileBlock::getNext()
{
	FileVersion* ret = new FileVersion();
	char* nextByte = _versiones + _actualOffset;
	ret->read(&nextByte);
	_actualOffset = nextByte - _versiones;
	return ret;
}

bool FileBlock::hasNext()
{
	return (_actualOffset < _used);
}

bool FileBlock::getHistory(std::ifstream& is)
{
    FileVersion* fv = new FileVersion();
    char* nextByte = _versiones;

    for (int i = 0; i < _cantVersiones; ++i) {
        fv->read(&nextByte);
        string tipoVersion = ((fv->getVersionType() == FileVersion::MODIFICACION) ? "modificacion" : "borrado");
        cout << " version: " << fv->getNroVersion() << ", tipo version: " << tipoVersion << ", usuario: " 
             << fv->getUser() << ", fecha: " << asctime(&(fv->getFecha()));
    }
    delete fv;
    return true;
}

