#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

#include "version.h"

#define TAMANIOARREGLOBLOQUE 1004

class Bloque 
{
public:
    static const int TAMANIOBLOQUE;

	Bloque(int Numero = -1,int Anterior = -1 ,int Siguiente = -1);
	~Bloque();

	void insertVersion(Version* version);

	bool searchVersion(int nro,Version** version);

	bool searchVersion(int nro);

	void write(char* buffer);

	void read(char* buffer);

	Version* getLastVersion();

	int getSiguiente()			{		return _siguiente;		}
	int getAnterior()			{		return _anterior;		}
	int getNumero()				{		return _numero;			}
	int getCantidadVersiones()	{		return _cantVersiones;	}

	void setSiguiente(int Siguiente){	_siguiente = Siguiente;	}
	void setAnterior(int Anterior)	{	_anterior = Anterior;	}

	bool hayLugar(Version* version);
protected:
	int _siguiente;
	int _anterior;
	int _espacioLibre;
	int _cantVersiones;
	int _offset;
	int _numero;

	char _versiones[TAMANIOARREGLOBLOQUE];
};

#endif


