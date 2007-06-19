// DirectoryBlock.h

#ifndef DIRECTORY_BLOCK_INCLUDED
#define DIRECTORY_BLOCK_INCLUDED

#include "DirectoryVersion.h"

#include <cstdlib>
#include <string>
#include <iostream>

#define TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS 1004

class DirectoryBlock
{
public:
	static const int TAMANIO_BLOQUE_DIRECTORIOS;
	
	DirectoryBlock(int Numero = -1, int Anterior = -1, int Siguiente = -1);

	~DirectoryBlock();

	void insertVersion(DirectoryVersion* version);
	bool searchVersion(int nro,DirectoryVersion** version);
	bool searchVersion(int nro);
	void write(char* buffer);
	void read(char* buffer);
	DirectoryVersion* getLastVersion();

	int getSiguiente()			{		return _siguiente;		}
	int getAnterior()			{		return _anterior;		}
	int getNumero()				{		return _numero;			}
	int getCantidadVersiones()	{		return _cantVersiones;	}
	
	int getFirstVersionNumber();

	void setSiguiente(int Siguiente){	_siguiente = Siguiente;	}
	void setAnterior(int Anterior)	{	_anterior = Anterior;	}

	bool hayLugar(DirectoryVersion* version);

	void moveFirst();
	DirectoryVersion* getNext();
	bool hasNext();
	
    bool getHistory(std::ifstream& is);

protected:
	int _siguiente;
	int _anterior;
	int _espacioLibre;
	int _cantVersiones;
	int _used;
	int _actualOffset;
	int _numero;

	char _versiones[TAMANIO_ARREGLO_BLOQUE_DIRECTORIOS];
};

#endif
