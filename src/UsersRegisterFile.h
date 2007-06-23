// UsersRegisterFile.h

#ifndef USERSREGISTERFILE_H_INCLUDED
#define USERSREGISTERFILE_H_INCLUDED

#include "UserBlock.h"

#include <fstream>
#include <list>
#include <string>

using std::string;
using std::list;

class UsersRegisterFile
{
public:
    enum t_status { ERROR = 0, OK, OVERFLOW };

	UsersRegisterFile();
	~UsersRegisterFile();

	bool create(const string& a_Filename); // crea el archivo
	bool destroy();

	bool open(const string& a_Filename);
	bool close();
	
	// trata de insertar las version en el bloque recibido con referencia:
	// - si lo inserta en el bloque -> devuelve OK
	// - si lo inserta en otro bloque porque el bloque cuyo nro se recibe 
	//	 como referencia se desborda -> devuelve OVERFLOW
	// - si no lo inserta porque esa version ya estaba en el bloque -> devuelve ERROR
	t_status insertRef(int reference, int bloque, int* nroBloqueNuevo);

	// crea un nuevo bloque para insertar la version, es la 1era version de un archivo nuevo
	// en la variable nroBloqueNuevo se devuelve el nro del bloque que se creo para poder
	// ingresarlo en el indice
	bool insertRef(int reference, int* nroBloqueNuevo);
	
   list<int> getReferences(int bloque, int cant);
   list<int> getAllReferences(int bloque);

protected:
	bool readBloque(int nroBloque);
	bool writeBloque();
	bool crearBloque(int Anterior = -1, int Siguiente = -1);
	bool readHeader();
	bool writeHeader();

private:
	std::fstream _filestr;       // file descriptor
	int          _cantBloques;
	UserBlock*   _bloqueActual;
	char*        _buffer;        // buffer de lectura-escritura
   string       _filename;
   bool         _isOpen;
};	

#endif

