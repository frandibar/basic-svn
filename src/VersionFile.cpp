// VersionFile.cpp

#include "VersionFile.h"

using std::ios;
using std::list;

VersionFile::VersionFile()
{
	_buffer = new char[Bloque::TAMANIO_BLOQUE];
	_bloqueActual = 0;
	_cantBloques = 0;
}	

VersionFile::~VersionFile()
{
	delete _bloqueActual;
	delete _buffer;
}

bool VersionFile::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, Bloque::TAMANIO_BLOQUE);
        // leo la cantidad de nodos
        memcpy(&_cantBloques, nextByte, sizeof(int));
        nextByte += sizeof(int);

		return true;
    }
    return false;
}

bool VersionFile::writeHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_cantBloques, sizeof(int));
        nextByte += sizeof(int);
        // volcar al archivo
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, Bloque::TAMANIO_BLOQUE);
        return true;
    }
    return false;
}

bool VersionFile::readBloque(int nroBloque)
{
	if (_filestr.is_open()) {
		writeBloque(); // escribo el bloque actual;

		_filestr.seekg((nroBloque + 1) * Bloque::TAMANIO_BLOQUE,ios::beg);
		_filestr.seekp((nroBloque + 1) * Bloque::TAMANIO_BLOQUE,ios::beg);

		_filestr.read(_buffer,Bloque::TAMANIO_BLOQUE);
		_bloqueActual->read(_buffer);

		return true;
	}

	return false;
}

bool VersionFile::writeBloque()
{
	if (_filestr.is_open()) {
		
		if(_bloqueActual != 0){
			_bloqueActual->write(_buffer);	//escribo el bloque en el buffer

			_filestr.seekg((_bloqueActual->getNumero() + 1) * Bloque::TAMANIO_BLOQUE,ios::beg);
			_filestr.seekp((_bloqueActual->getNumero() + 1) * Bloque::TAMANIO_BLOQUE,ios::beg);

			_filestr.write(_buffer,Bloque::TAMANIO_BLOQUE);

			return true;
		}
		return false;
	}

	return false;
}

bool VersionFile::crearBloque(int Anterior, int Siguiente)
{

	if (_filestr.is_open()) {
		writeBloque(); // escribo el bloque actual;
		_bloqueActual = new Bloque(_cantBloques,Anterior,Siguiente);
		return true;
	}

	return false;
}

bool VersionFile::create(const char* fileName)
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

bool VersionFile::open(const char* fileName)
{
    _filestr.open(fileName, ios::in | ios::out | ios::binary);

	if(_filestr.is_open()){
		if (readHeader())
				return true;
		return false;
	}    
	return false;
}

bool VersionFile::close()
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

void VersionFile::insertVersion(int nroVersion, const char* User, time_t Fecha, long int Offset, char Tipo, int* nroBloqueNuevo){
	
	writeBloque(); // escribo el bloque actual
	delete _bloqueActual;
	_bloqueActual = new Bloque(_cantBloques); // creo el nuevo bloque apuntado por el actual
	*nroBloqueNuevo = _cantBloques;	// guardo la referencia al nro del bloque nuevo
	_cantBloques++;	// incremento la cantidad de bloques del archivo
    
	//creo la version nueva
	Version* version = new Version(nroVersion,nroVersion,Fecha,User,Offset,Tipo);
	_bloqueActual->insertVersion(version); // inserto la version
	delete version;
}

VersionFile::t_status VersionFile::insertVersion(int nroVersion,const char* User,long int Fecha,long int Offset,char Tipo,int bloque,int* nroBloqueNuevo)
{
	readBloque(bloque); // obtengo el bloque
	Version* ultimaVersion = _bloqueActual->getLastVersion();
	int ultimoOriginal = ultimaVersion->getOriginal();
	delete ultimaVersion;
	Version* nuevaVersion = new Version(nroVersion,ultimoOriginal,Fecha,User,Offset,Tipo);
	if (_bloqueActual->hayLugar(nuevaVersion)) {
		if (!_bloqueActual->searchVersion(nuevaVersion->getNroVersion())) {
			_bloqueActual->insertVersion(nuevaVersion);
			delete nuevaVersion;
			return VersionFile::OK;
		}
		
		delete nuevaVersion;
		return VersionFile::ERROR;
	}
	
	Bloque* bloqueNuevo = new Bloque(_cantBloques,_bloqueActual->getNumero());
	_cantBloques++;
	_bloqueActual->setSiguiente(bloqueNuevo->getNumero());
	writeBloque();                  // escribo el bloque actual modificado
	delete _bloqueActual;	        // elimino el bloque actual
	_bloqueActual = bloqueNuevo;	// seteo el bloque actual como el nuevo
	_bloqueActual->insertVersion(nuevaVersion);
	delete nuevaVersion;
	*nroBloqueNuevo = _bloqueActual->getNumero();
	return VersionFile::OVERFLOW;
}

bool VersionFile::searchVersion(Version** version,int nroVersion,int bloque)
{
	readBloque(bloque);
	return _bloqueActual->searchVersion(nroVersion,version);
}


bool VersionFile::getVersionFrom(int original, int final, int bloque, list<Version>& lstVersions)
{
	// levanto el bloque a partir del que voy a empezar a buscar la original y armar la lista de versiones.
	// podre moverme hacia bloques anteriores o posteriores.
	
    lstVersions.clear();
	readBloque(bloque);
	bool found = _bloqueActual->searchVersion(original);

    // buscar el bloque conteniendo la version original
	while (!found) {
		
		int next = -1;
		int first = _bloqueActual->getFirstVersionNumber();

		if (original < first)
			next = _bloqueActual->getAnterior();
		else
			next = _bloqueActual->getSiguiente();

		if (next < 0) 
            return false;
		else {
			readBloque(next);
			found = _bloqueActual->searchVersion(original);
		}
	}

    // en este punto, _bloqueActual contiene la version original
    
	Version* auxVersion;
	_bloqueActual->searchVersion(original, &auxVersion);
	lstVersions.push_front(*auxVersion);
    delete auxVersion;

    bool end = false;

    // copio las restantes versiones del 1er bloque
    while (_bloqueActual->hasNext() && !end) {

        _bloqueActual->searchVersion(original, &auxVersion);
        lstVersions.push_back(*auxVersion);
        if (auxVersion->getNroVersion() == final)
            end = true;

        delete auxVersion;
    }

    int next = -1;
    while (!end) {
        next = _bloqueActual->getSiguiente();
        readBloque(next);
        _bloqueActual->moveFirst();
        while (_bloqueActual->hasNext() && !end) {
            auxVersion = _bloqueActual->getNext();
            if (auxVersion->getNroVersion() == final)
                end = true;
            lstVersions.push_back(*auxVersion);
            delete auxVersion;
        }
    }

	return true;
}

int VersionFile::getLastOriginalVersionNumber(int bloque)
{
    readBloque(bloque);
    Version* aux = _bloqueActual->getLastVersion();
    int ret = aux->getOriginal();
    delete aux;
    return ret;
}


int VersionFile::getLastVersionNumber(int bloque)
{
    readBloque(bloque);
    Version* aux = _bloqueActual->getLastVersion();
    int ret = aux->getNroVersion();
    delete aux;
    return ret;
}

