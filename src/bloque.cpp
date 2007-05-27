#include "bloque.h"

#include <iostream>

// CONSTS
const int Bloque::TAMANIOBLOQUE    = 1024;

Bloque::Bloque(int Numero,int Anterior,int Siguiente)
{
	_espacioLibre = TAMANIOARREGLOBLOQUE;

	_cantVersiones = 0;

	_numero = Numero;

	_anterior = Anterior;

	_siguiente = Siguiente;

	_offset = TAMANIOARREGLOBLOQUE - _espacioLibre;
}

Bloque::~Bloque(){
}

void Bloque::insertVersion(Version* version){

	char* nextByte = _versiones + _offset;

	version->write(nextByte);

	_cantVersiones++;

	_espacioLibre -= version->tamanioEnDisco();

	_offset = TAMANIOARREGLOBLOQUE - _espacioLibre;

	return;
}

void Bloque::read(char* buffer){
	int offset = 0;

	//numero del bloque
	memcpy(&_numero,buffer + offset,sizeof(int));
	offset += sizeof(int);

	//anterior
	memcpy(&_anterior,buffer + offset,sizeof(int));	
	offset += sizeof(int);

	//siguiente
	memcpy(&_siguiente,buffer + offset,sizeof(int));
	offset += sizeof(int);

	//espacio libre
	memcpy(&_espacioLibre,buffer + offset,sizeof(int));
	offset += sizeof(int);

	_offset = TAMANIOARREGLOBLOQUE - _espacioLibre;

	//cantidad de versiones
	memcpy(&_cantVersiones,buffer + offset,sizeof(int));
	offset += sizeof(int);

	//el arreglo con las versiones
	memcpy(_versiones,buffer + offset,TAMANIOARREGLOBLOQUE);
	offset += TAMANIOARREGLOBLOQUE;

	return;
}

void Bloque::write(char* buffer){
	int offset = 0;

	//numero del bloque
	memcpy(buffer + offset,&_numero,sizeof(int));
	offset += sizeof(int);

	//anterior
	memcpy(buffer + offset,&_anterior,sizeof(int));
	offset += sizeof(int);

	//siguiente
	memcpy(buffer + offset,&_siguiente,sizeof(int));
	offset += sizeof(int);

	//espacio libre
	memcpy(buffer + offset,&_espacioLibre,sizeof(int));
	offset += sizeof(int);

	//cantidad de versiones
	memcpy(buffer + offset,&_cantVersiones,sizeof(int));
	offset += sizeof(int);

	//arreglo con las versiones
	memcpy(buffer + offset,_versiones,TAMANIOARREGLOBLOQUE);
	offset += TAMANIOARREGLOBLOQUE;

	return;
}

bool Bloque::hayLugar(Version* version){
	return (_espacioLibre >= version->tamanioEnDisco());
}

bool Bloque::searchVersion(int nro,Version** version){

	char* nextByte = _versiones;

	*version = new Version();

	for(int i = 0;i < _cantVersiones;i++){
		
		(*version) ->read(nextByte);

		if((*version)->getNroVersion() == nro)
			return true;
	}

	delete (*version);
	return false;
}

bool Bloque::searchVersion(int nro){

	char* nextByte = _versiones;

	Version* version = new Version();

	for(int i = 0;i < _cantVersiones;i++){
		
		version->read(nextByte);

		if(version->getNroVersion() == nro){
			delete version;
			return true;
		}
	}

	delete version;
	return false;
}
