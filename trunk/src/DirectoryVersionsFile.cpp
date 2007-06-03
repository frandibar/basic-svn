// DirectoryVersionsFile.cpp

#include "DirectoryVersionsFile.h"

using std::list;
using std::ios;

DirectoryVersionsFile::DirectoryVersionsFile()
{
	_buffer = new char[DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS];
	_bloqueActual = 0;
	_cantBloques = 0;
}	

DirectoryVersionsFile::~DirectoryVersionsFile()
{
	delete _bloqueActual;
	delete _buffer;
}

bool DirectoryVersionsFile::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS);
        // leo la cantidad de bloques
        memcpy(&_cantBloques, nextByte, sizeof(int));
        nextByte += sizeof(int);

		return true;
    }
    return false;
}

bool DirectoryVersionsFile::writeHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_cantBloques, sizeof(int));
        nextByte += sizeof(int);
        // volcar al archivo
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS);
        return true;
    }
    return false;
}

bool DirectoryVersionsFile::readBloque(int nroBloque)
{
	if (_filestr.is_open()) {
		if(_bloqueActual != 0){
			writeBloque(); // escribo el bloque actual;
			delete _bloqueActual;
		}

		_bloqueActual = new DirectoryBlock();

		_filestr.seekg((nroBloque + 1) * DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS,ios::beg);
		_filestr.seekp((nroBloque + 1) * DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS,ios::beg);

		_filestr.read(_buffer,DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS);
		_bloqueActual->read(_buffer);

		return true;
	}

	return false;
}

bool DirectoryVersionsFile::writeBloque()
{
	if (_filestr.is_open()) {
		
		if(_bloqueActual != 0){
			_bloqueActual->write(_buffer);	//escribo el bloque en el buffer

			_filestr.seekg((_bloqueActual->getNumero() + 1) * DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS,ios::beg);
			_filestr.seekp((_bloqueActual->getNumero() + 1) * DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS,ios::beg);

			_filestr.write(_buffer,DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS);

			return true;
		}
		return false;
	}

	return false;
}

bool DirectoryVersionsFile::crearBloque(int Anterior, int Siguiente)
{

	if (_filestr.is_open()) {
		writeBloque(); // escribo el bloque actual;
		_bloqueActual = new DirectoryBlock(_cantBloques,Anterior,Siguiente);
		return true;
	}

	return false;
}

bool DirectoryVersionsFile::create(const char* fileName)
{
    _filestr.open(fileName, ios::out | ios::in | ios::binary);

	if (!_filestr) {
		_filestr.open(fileName, ios::out | ios::binary);
		
		_filestr.close();

		_filestr.open(fileName, ios::in | ios::out | ios::binary);
		if (!_filestr)
			return false;
	}

    _cantBloques = 0;

    return writeHeader();
}

bool DirectoryVersionsFile::open(const char* fileName)
{
    _filestr.open(fileName, ios::in | ios::out | ios::binary);

	if(_filestr.is_open()){
		if (readHeader())
				return true;
		return false;
	}    
	return false;
}

bool DirectoryVersionsFile::close()
{
	if (_filestr.is_open()) {
		if (writeHeader()) {
			if (writeBloque()) {
				_filestr.close();
				return true;
			}
		}
		return false;
	}
	return false;
}

void DirectoryVersionsFile::insertVersion(DirectoryVersion* newVersion, int* nroBloqueNuevo){
	
	writeBloque(); // escribo el bloque actual
	delete _bloqueActual;
	_bloqueActual = new DirectoryBlock(_cantBloques); // creo el nuevo bloque apuntado por el actual
	*nroBloqueNuevo = _cantBloques;	// guardo la referencia al nro del bloque nuevo
	_cantBloques++;	// incremento la cantidad de bloques del archivo
    
	//inserto la nueva version
	_bloqueActual->insertVersion(newVersion);
	
	return;
}

DirectoryVersionsFile::t_status DirectoryVersionsFile::insertVersion(DirectoryVersion* newVersion,int bloque,int* nroBloqueNuevo)
{
	readBloque(bloque); // obtengo el bloque
	if (_bloqueActual->hayLugar(newVersion)) {
		if (!_bloqueActual->searchVersion(newVersion->getNroVersion())) {
			_bloqueActual->insertVersion(newVersion);
			return DirectoryVersionsFile::OK;
		}
		
		return DirectoryVersionsFile::ERROR;
	}
	
	DirectoryBlock* bloqueNuevo = new DirectoryBlock(_cantBloques,_bloqueActual->getNumero());
	_cantBloques++;
	_bloqueActual->setSiguiente(bloqueNuevo->getNumero());
	writeBloque();                  // escribo el bloque actual modificado
	delete _bloqueActual;	        // elimino el bloque actual
	_bloqueActual = bloqueNuevo;	// seteo el bloque actual como el nuevo
	_bloqueActual->insertVersion(newVersion);
	*nroBloqueNuevo = _bloqueActual->getNumero();
	return DirectoryVersionsFile::OVERFLOW;
}

bool DirectoryVersionsFile::searchVersion(DirectoryVersion** version,int nroVersion,int bloque)
{
	readBloque(bloque);
	return _bloqueActual->searchVersion(nroVersion,version);
}


bool DirectoryVersionsFile::getVersion(int versionNumber,int bloque,DirectoryVersion** version)
{
	// levanto el bloque a partir del que voy a empezar a buscar la original y armar la lista de versiones.
	// podre moverme hacia bloques anteriores o posteriores.
	
	readBloque(bloque);
	bool found = _bloqueActual->searchVersion(versionNumber);

    // buscar el bloque conteniendo la version original
	while (!found) {
		
		int next = -1;
		int first = _bloqueActual->getFirstVersionNumber();

		if (versionNumber < first)
			next = _bloqueActual->getAnterior();
		else
			next = _bloqueActual->getSiguiente();

		if (next < 0) 
            return false;
		else {
			readBloque(next);
			found = _bloqueActual->searchVersion(versionNumber);
		}
	}

    // en este punto, _bloqueActual contiene la version original    
	DirectoryVersion* auxVersion;
	_bloqueActual->searchVersion(versionNumber, &auxVersion);

	return true;
}

int DirectoryVersionsFile::getLastVersionNumber(int bloque)
{
    readBloque(bloque);
    DirectoryVersion* aux = _bloqueActual->getLastVersion();
    int ret = aux->getNroVersion();
    delete aux;
    return ret;
}

