#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

class Version
{
public:
	Version();
	Version(int NroVersion, int Original, long int Fecha, char* User, 
		long int Offset, char Tipo);
	~Version();

	//empaquetadores y desempaquetadores
	void read(char* buffer);
	void write(char* buffer);

	//getters - no hay setters porque no hay modificacion de la info de las versiones
	int getNroVersion()		const { return _nroVersion; }
	int getOriginal()		const { return _original;	}
	long int getFecha()		const { return _fecha;		}
	char* getUser()			const { return _user;		}
	long int getOffset()	const {	return _offset;		}
	char getTipo()			const {	return _tipo;		}

	int tamanioEnDisco();

protected:
	// el numero de la version
	int			_nroVersion;		
	// el numero de la version que es el ultimo original de este archivo
	int			_original;			
	// la fecha en que se establecio la version
	long int	_fecha;				
	// el usuario que establecio la version
	char*		_user;				
	// el offset donde se encuentra la version en el archivo que contiene originales y diffs
	long int	_offset;
	// el tipo de archivo del que se trata la version "b" = binario, "t" = texto
	char		_tipo;
};

#endif

