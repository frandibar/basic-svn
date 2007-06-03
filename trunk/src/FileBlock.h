// Bloque.h

#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include "FileVersion.h"

#include <cstdlib>
#include <string>
#include <iostream>

#define TAMANIO_ARREGLO_BLOQUE_ARCHIVOS 1004

class FileBlock
{
public:
    static const int TAMANIO_BLOQUE_ARCHIVOS;

	FileBlock(int Numero = -1,int Anterior = -1 ,int Siguiente = -1);
	~FileBlock();

	void insertVersion(FileVersion* version);
	bool searchVersion(int nro,FileVersion** version);
	bool searchVersion(int nro);
	void write(char* buffer);
	void read(char* buffer);
	FileVersion* getLastVersion();

	int getSiguiente()			{		return _siguiente;		}
	int getAnterior()			{		return _anterior;		}
	int getNumero()				{		return _numero;			}
	int getCantidadVersiones()	{		return _cantVersiones;	}
	
	int getFirstVersionNumber();

	void setSiguiente(int Siguiente){	_siguiente = Siguiente;	}
	void setAnterior(int Anterior)	{	_anterior = Anterior;	}

	bool hayLugar(FileVersion* version);

	void moveFirst();
	FileVersion* getNext();
	bool hasNext();
	
protected:
	int _siguiente;
	int _anterior;
	int _espacioLibre;
	int _cantVersiones;
	int _used;
	int _actualOffset;
	int _numero;

	char _versiones[TAMANIO_ARREGLO_BLOQUE_ARCHIVOS];
};

#endif


