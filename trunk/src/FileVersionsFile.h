// FileVersionsFile.h

#ifndef VERSIONFILE_H_INCLUDED
#define VERSIONFILE_H_INCLUDED

#include "FileVersion.h"
#include "FileBlock.h"

#include <fstream>
#include <list>
#include <string>

using std::string;

class FileVersionsFile
{
public:
    enum t_status { ERROR = 0, OK, OVERFLOW };

	FileVersionsFile();
	~FileVersionsFile();

	bool create(const string& a_Filename); // crea el archivo
	bool destroy();

	bool open(const string& a_Filename);
	bool close();
	
	// trata de insertar las version en el bloque recibido con referencia:
	// - si lo inserta en el bloque -> devuelve OK
	// - si lo inserta en otro bloque porque el bloque cuyo nro se recibe 
	//	 como referencia se desborda -> devuelve OVERFLOW
	// - si no lo inserta porque esa version ya estaba en el bloque -> devuelve ERROR
	t_status insertVersion(int nroVersion, const char* User, tm Fecha, long int Offset, char Tipo, FileVersion::t_versionType VersionType, int bloque, int* nroBloqueNuevo);

	// crea un nuevo bloque para insertar la version, es la 1� version de un archivo nuevo
	// en la variable nroBloqueNuevo se devuelve el nro del bloque que se creo para poder
	// ingresarlo en el indice
	void insertVersion(int nroVersion, const char* User, tm Fecha, long int Offset, char Tipo, FileVersion::t_versionType VersionType, int* nroBloqueNuevo);
	
	bool searchVersion(FileVersion** version, int nroVersion,int bloque);
	bool getVersionFrom(int original, int final, int bloque, std::list<FileVersion>& lstVersions);
	int getLastOriginalVersionNumber(int bloque);
	int getLastVersionNumber(int bloque);
	void getLastVersion(FileVersion** version,int bloque);	//devuelve la ultima version del bloque

protected:
	bool readBloque(int nroBloque);
	bool writeBloque();
	bool crearBloque(int Anterior = -1, int Siguiente = -1);
	bool readHeader();
	bool writeHeader();

private:
	std::fstream _filestr;       // file descriptor
	int          _cantBloques;
	FileBlock*   _bloqueActual;
	char*        _buffer;        // buffer de lectura-escritura
    string       _filename;
    bool         _isOpen;
};	

#endif

