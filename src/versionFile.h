#ifndef VERSIONFILE_H_INCLUDED
#define VERSIONFILE_H_INCLUDED

#include <fstream>

#include "version.h"
#include "bloque.h"

class VersionFile{
public:
	// constructor & desctructor
	VersionFile();
	~VersionFile();

	// crea el archivo
	bool create(char* fileName);

	bool open(char* fileName);

	bool close();
	
	// trata de insertar las version en el bloque recibido con referencia:
	// - si lo inserta en el bloque -> devuelve 1
	// - si lo inserta en otro bloque porque el bloque cuyo nro se recibe 
	//	 como referencia se desborda -> devuelve 2
	// - si no lo inserta porque esa version ya estaba en el bloque -> devuelve 0
	int insertVersion(Version* version, int bloque,int* nroBloqueNuevo);

	// crea un nuevo bloque para insertar la version, es la 1º version de un archivo nuevo
	// en la variable nroBloqueNuevo se devuelve el nro del bloque que se creo para poder
	// ingresarlo en el indice
	void insertVersion(Version* version,int* nroBloqueNuevo);

	bool searchVersion(Version** version, int nroVersion,int bloque);

protected:
	// file descriptor
	std::fstream _filestr;
	
	// cantidad de bloques
	int _cantBloques;

	// el bloque actual
	Bloque* _bloqueActual;

	// buffer de lectura-escritura
	char* _buffer;

	bool readBloque(int nroBloque);

	bool writeBloque();

	bool crearBloque(int Anterior = -1, int Siguiente = -1);

	bool readHeader();

	bool writeHeader();
};	

#endif