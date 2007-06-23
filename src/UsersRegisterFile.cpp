// UsersRegisterFile.cpp

#include "UsersRegisterFile.h"

using std::ios;
using std::list;

// constructor
UsersRegisterFile::UsersRegisterFile() : _cantBloques(0), _bloqueActual(0), _isOpen(false)
{
	_buffer = new char[UserBlock::TAMANIO_BLOQUE_USUARIO];
}	

UsersRegisterFile::~UsersRegisterFile()
{
	delete _bloqueActual;
	delete _buffer;
}

bool UsersRegisterFile::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, UserBlock::TAMANIO_BLOQUE_USUARIO);
        // leo la cantidad de nodos
        memcpy(&_cantBloques, nextByte, sizeof(int));
        nextByte += sizeof(int);

		return true;
    }
    return false;
}

bool UsersRegisterFile::writeHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_cantBloques, sizeof(int));
        nextByte += sizeof(int);
        // volcar al archivo
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, UserBlock::TAMANIO_BLOQUE_USUARIO);
        return true;
    }
    return false;
}

bool UsersRegisterFile::readBloque(int nroBloque)
{
	if (_filestr.is_open()) {
		if(_bloqueActual != 0){
			writeBloque(); // escribo el bloque actual;
			delete _bloqueActual;
		}

		_bloqueActual = new UserBlock();

		_filestr.seekg((nroBloque + 1) * UserBlock::TAMANIO_BLOQUE_USUARIO,ios::beg);
		_filestr.seekp((nroBloque + 1) * UserBlock::TAMANIO_BLOQUE_USUARIO,ios::beg);

		_filestr.read(_buffer,UserBlock::TAMANIO_BLOQUE_USUARIO);
		_bloqueActual->read(_buffer);

		return true;
	}

	return false;
}

bool UsersRegisterFile::writeBloque()
{
	if (_filestr.is_open()) {
		if (_bloqueActual != 0) {
			_bloqueActual->write(_buffer);	//escribo el bloque en el buffer

			_filestr.seekg((_bloqueActual->getNumero() + 1) * UserBlock::TAMANIO_BLOQUE_USUARIO,ios::beg);
			_filestr.seekp((_bloqueActual->getNumero() + 1) * UserBlock::TAMANIO_BLOQUE_USUARIO,ios::beg);

			_filestr.write(_buffer,UserBlock::TAMANIO_BLOQUE_USUARIO);
			return true;
		}
	}
	return false;  // this does not mean an error
}

bool UsersRegisterFile::crearBloque(int Anterior, int Siguiente)
{
	if (_filestr.is_open()) {
		writeBloque(); // escribo el bloque actual;
		_bloqueActual = new UserBlock(_cantBloques,Anterior,Siguiente);
		return true;
	}

	return false;
}

bool UsersRegisterFile::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    _filestr.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_filestr.is_open()) {
		_filestr.clear();
		_filestr.open(a_Filename.c_str(), ios::out | ios::binary);
		_filestr.close();
		_filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
		_isOpen = _filestr.is_open();
	}

    _cantBloques = 0;
    _isOpen = _isOpen && writeHeader();
    _filename = a_Filename;
    return _isOpen;
}

bool UsersRegisterFile::destroy()
{
    if (_isOpen) 
        return false;

    int ret = remove(_filename.c_str());
    return ret == 0;
}

bool UsersRegisterFile::open(const string& a_Filename)
{
    if (_isOpen) 
        return true;

    _filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);

	_isOpen = _filestr.is_open() && readHeader();
    _filename = a_Filename;
    return _isOpen;
}

bool UsersRegisterFile::close()
{
    if (!_isOpen)
        return true;

	_isOpen = _filestr.is_open() && writeHeader() && writeBloque();
    if (_isOpen) {
        _filestr.close();
        _isOpen = _filestr.is_open();
	}
    return !_isOpen;
}

bool UsersRegisterFile::insertRef(int reference, int* nroBloqueNuevo)
{
	writeBloque();  // ignore return value

	delete _bloqueActual;
	_bloqueActual = new UserBlock(_cantBloques); // creo el nuevo bloque apuntado por el actual
    if (!_bloqueActual)
        return false;

	*nroBloqueNuevo = _cantBloques;	// guardo la referencia al nro del bloque nuevo
	_cantBloques++;	// incremento la cantidad de bloques del archivo
    
	//creo la version nueva
	if (!_bloqueActual->insertRef(reference))
        return false;
   return true;
}

UsersRegisterFile::t_status UsersRegisterFile::insertRef(int reference, int bloque, int* nroBloqueNuevo)
{
	readBloque(bloque); // obtengo el bloque

	if (_bloqueActual->hayLugar()) {
		   if(_bloqueActual->insertRef(reference))
		      return UsersRegisterFile::OK;
		
		   return UsersRegisterFile::ERROR;
	}
	
	UserBlock* bloqueNuevo = new UserBlock(_cantBloques,_bloqueActual->getNumero());
	_cantBloques++;
	_bloqueActual->setSiguiente(bloqueNuevo->getNumero());
	writeBloque();                  // escribo el bloque actual modificado
	delete _bloqueActual;	        // elimino el bloque actual
	_bloqueActual = bloqueNuevo;	// seteo el bloque actual como el nuevo
	_bloqueActual->insertRef(reference);
	*nroBloqueNuevo = _bloqueActual->getNumero();
	return UsersRegisterFile::OVERFLOW;
}

list<int> UsersRegisterFile::getReferences(int bloque, int cant)
{
   list<int> ret;

   readBloque(bloque);
   
   int ref_a_insertar;

   if(_bloqueActual->getCantidadReferencias() >= cant)
   {
      _bloqueActual->moveTo(_bloqueActual->getCantidadReferencias() - cant);
      
      while(_bloqueActual->hasNext())
      {
         ref_a_insertar = _bloqueActual->getNext();
         ret.push_back(ref_a_insertar);
      }
      
      return ret;   
   }

   int restantes = cant - _bloqueActual->getCantidadReferencias();
   int indice = restantes;

   int proximo_a_leer = _bloqueActual->getAnterior();

   while( (restantes > 0) && (proximo_a_leer >= 0) )
   {
      readBloque(proximo_a_leer);
      if(restantes <= _bloqueActual->getCantidadReferencias())
        indice = _bloqueActual->getCantidadReferencias() - restantes;

      restantes -= _bloqueActual->getCantidadReferencias();
      
      if(restantes > 0)
         proximo_a_leer = _bloqueActual->getAnterior();
   }

   do{   
      _bloqueActual->moveTo(indice);
   
      while(_bloqueActual->hasNext())
      {
         ref_a_insertar = _bloqueActual->getNext();
         ret.push_back(ref_a_insertar);
      }
      
      proximo_a_leer = _bloqueActual->getSiguiente();
      if(proximo_a_leer >= 0)
         readBloque(proximo_a_leer);
      
      indice = 1;
   }while(proximo_a_leer >= 0);

   return ret;
}

list<int> UsersRegisterFile::getAllReferences(int bloque)
{
    list<int> ret;

    int ref_a_insertar;

    readBloque(bloque);

    while(_bloqueActual->getAnterior() >= 0)
        readBloque(_bloqueActual->getAnterior());

    bool fin = false;

    do {
        _bloqueActual->moveFirst();

        while(_bloqueActual->hasNext())
        {
            int ref_a_insertar = _bloqueActual->getNext();
            ret.push_back(ref_a_insertar);
        }

        if(_bloqueActual->getSiguiente() < 0)
            fin = true;

        else
            readBloque(_bloqueActual->getSiguiente());
    } while(!fin);

    return ret;
}
