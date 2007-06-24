// UserBlock.cpp

#include "UserBlock.h"

const int UserBlock::TAMANIO_BLOQUE_USUARIO = 256;

UserBlock::UserBlock(int Numero,int Anterior,int Siguiente)
{
	_espacioLibre = TAMANIO_ARREGLO_BLOQUE_USUARIO;
	_cantReferencias = 0;
	_numero = Numero;
	_anterior = Anterior;
	_siguiente = Siguiente;
	_used = TAMANIO_ARREGLO_BLOQUE_USUARIO - _espacioLibre;
	_actualOffset = 0;
}

UserBlock::~UserBlock()
{
}

bool UserBlock::insertRef(int ref)
{
	char* nextByte = _referencias + _used;
	_espacioLibre -= sizeof(int);
	memcpy(nextByte, &ref, sizeof(int));
	_cantReferencias++;
	_used = (TAMANIO_ARREGLO_BLOQUE_USUARIO - _espacioLibre);
	_actualOffset = _used;
    return true;
}

void UserBlock::read(char* buffer)
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

	_used = TAMANIO_ARREGLO_BLOQUE_USUARIO - _espacioLibre;

	//cantidad de versiones
	memcpy(&_cantReferencias,buffer + offset, sizeof(int));
	offset += sizeof(int);

	//el arreglo con las versiones
	memcpy(_referencias,buffer + offset,TAMANIO_ARREGLO_BLOQUE_USUARIO);
	offset += TAMANIO_ARREGLO_BLOQUE_USUARIO;

	return;
}

void UserBlock::write(char* buffer)
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
	memcpy(buffer + offset, &_cantReferencias, sizeof(int));
	offset += sizeof(int);

	//arreglo con las versiones
	memcpy(buffer + offset,_referencias, TAMANIO_ARREGLO_BLOQUE_USUARIO);
	offset += TAMANIO_ARREGLO_BLOQUE_USUARIO;

	return;
}

bool UserBlock::hayLugar()
{
	return (_espacioLibre >= (int)sizeof(int));
}

void UserBlock::moveFirst()
{
	_actualOffset = 0;
	return;
}

int UserBlock::getNext()
{
   int ret;
	char* nextByte = _referencias + _actualOffset;
   memcpy(&ret, nextByte, sizeof(int));
   nextByte += sizeof(int);
   _actualOffset = nextByte - _referencias;
   return ret;
}

bool UserBlock::hasNext()
{
	return (_actualOffset < _used);
}

bool UserBlock::moveTo(int refNumber)
{
   int desp = refNumber * sizeof(int);

   if(desp < _used) {
      _actualOffset = desp;
      return true;
   }

   return false;   
}  


