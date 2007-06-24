// DirectoryVersionsFile.h

#ifndef DIRECTORY_VERSIONS_FILE_INCLUDED
#define DIRECTORY_VERSIONS_FILE_INLCUDED

#include "DirectoryVersion.h"
#include "DirectoryBlock.h"
#include "debug.h"

#include <fstream>
#include <list>
#include <string>

using std::string;

class DirectoryVersionsFile
{
public:
    enum t_status { ERROR = 0, OK, OVERFLOW };

	DirectoryVersionsFile();
	~DirectoryVersionsFile();

	bool create(const string& a_Filename); // crea el archivo
	bool open(const string& a_Filename);
	bool close();
	bool destroy();
	
	// trata de insertar las version en el bloque recibido con referencia:
	// - si lo inserta en el bloque -> devuelve OK
	// - si lo inserta en otro bloque porque el bloque cuyo nro se recibe 
	//	 como referencia se desborda -> devuelve OVERFLOW
	// - si no lo inserta porque esa version ya estaba en el bloque -> devuelve ERROR
	t_status insertVersion(DirectoryVersion* newVersion,int bloque,int* nroBloqueNuevo);

	// crea un nuevo bloque para insertar la version, es la 1? version de un archivo nuevo
	// en la variable nroBloqueNuevo se devuelve el nro del bloque que se creo para poder
	// ingresarlo en el indice
	void insertVersion(DirectoryVersion* newVersion, int* nroBloqueNuevo);
	
	bool searchVersion(DirectoryVersion** version, int nroVersion,int bloque);
	bool getVersion(int versionNumber,int bloque,DirectoryVersion** version);
	int getLastVersionNumber(int bloque);
    bool getHistory(std::ostream& os, int block);

protected:
	// file descriptor
	std::fstream _filestr;
	
	int _cantBloques;
	DirectoryBlock* _bloqueActual;

	// buffer de lectura-escritura
	char*	_buffer;
	string  _filename;
    bool    _isOpen;

	bool readBloque(int nroBloque);
	bool writeBloque();
	bool crearBloque(int Anterior = -1, int Siguiente = -1);
	bool readHeader();
	bool writeHeader();
};

#endif
