// DirectoryBlock.h

#include "DirectoryBlock.h"
#include "debug.h"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

const int DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS = 1024;

DirectoryBlock::DirectoryBlock(int Numero,int Anterior,int Siguiente)
{
	_espacioLibre = TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS;
	_cantVersiones = 0;
	_numero = Numero;
	_anterior = Anterior;
	_siguiente = Siguiente;
	_used = TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS - _espacioLibre;
	_actualOffset = 0;
}

DirectoryBlock::~DirectoryBlock()
{
}

void DirectoryBlock::insertVersion(DirectoryVersion* version)
{
	char* nextByte = _versiones + _used;

	_espacioLibre -= version->tamanioEnDisco();
	version->write(nextByte);
	_cantVersiones++;
	_used = (TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS - _espacioLibre);
	_actualOffset = _used;

	return;
}

void DirectoryBlock::read(char* buffer)
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

	_used = TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS - _espacioLibre;

	//cantidad de versiones
	memcpy(&_cantVersiones,buffer + offset, sizeof(int));
	offset += sizeof(int);

	//el arreglo con las versiones
	memcpy(_versiones,buffer + offset,TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS);
	offset += TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS;

	return;
}

void DirectoryBlock::write(char* buffer)
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
	memcpy(buffer + offset,_versiones, TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS);
	offset += TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS;

	return;
}

bool DirectoryBlock::hayLugar(DirectoryVersion* version)
{
	return (_espacioLibre >= version->tamanioEnDisco());
}

bool DirectoryBlock::searchVersion(int nro,DirectoryVersion** version)
{
	char* nextByte = _versiones;

	for (int i = 0;i < _cantVersiones; ++i) {
		*version = new DirectoryVersion();
		
		(*version) ->read(&nextByte);
		_actualOffset = nextByte - _versiones;
		
		if ((*version)->getNroVersion() == nro){
			return true;
		}

		delete (*version);
	}

	return false;
}

bool DirectoryBlock::searchVersion(int nro)
{
	char* nextByte = _versiones;

	for (int i = 0;i < _cantVersiones; ++i) {
		
		DirectoryVersion* version = new DirectoryVersion();
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

DirectoryVersion* DirectoryBlock::getLastVersion()
{
	DirectoryVersion* ret = new DirectoryVersion();
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

int DirectoryBlock::getFirstVersionNumber()
{
	DirectoryVersion* version = new DirectoryVersion();
	char* nextByte = _versiones;	
	version->read(&nextByte);
	_actualOffset = nextByte - _versiones;
	int ret = version->getNroVersion();
	delete version;
	return ret;
}

void DirectoryBlock::moveFirst()
{
	_actualOffset = 0;
	return;
}

DirectoryVersion* DirectoryBlock::getNext()
{
	DirectoryVersion* ret = new DirectoryVersion();
	char* nextByte = _versiones + _actualOffset;
	ret->read(&nextByte);
	_actualOffset = nextByte - _versiones;
	return ret;
}

bool DirectoryBlock::hasNext()
{
	return (_actualOffset < _used);
}

bool DirectoryBlock::getHistory(std::ifstream& is)
{
    DirectoryVersion* dv = new DirectoryVersion();
    char* nextByte = _versiones;
    
    for(int i = 0; i < _cantVersiones; ++i)
    {
        dv->read(&nextByte);
        std::string tipoVersion = ((dv->getType() == DirectoryVersion::MODIFICACION) ? "modificacion" : "borrado");
        cout << " version: " << dv->getNroVersion() << ", tipo version: " << tipoVersion << ", usuario: " 
             << dv->getUser() << ", fecha: " << asctime(&(dv->getDate()));
    }

    delete dv;
    return true;
}

