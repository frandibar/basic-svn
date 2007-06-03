// FileVersionsFile.cpp

#include "FileVersionsFile.h"
#include "debug.h"

using std::ios;
using std::list;

// constructor
FileVersionsFile::FileVersionsFile() : _cantBloques(0), _bloqueActual(0), _isOpen(false)
{
	_buffer = new char[FileBlock::TAMANIO_BLOQUE_ARCHIVOS];
}	

FileVersionsFile::~FileVersionsFile()
{
	delete _bloqueActual;
	delete _buffer;
}

bool FileVersionsFile::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, FileBlock::TAMANIO_BLOQUE_ARCHIVOS);
        // leo la cantidad de nodos
        memcpy(&_cantBloques, nextByte, sizeof(int));
        nextByte += sizeof(int);

		return true;
    }
    return false;
}

bool FileVersionsFile::writeHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_cantBloques, sizeof(int));
        nextByte += sizeof(int);
        // volcar al archivo
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, FileBlock::TAMANIO_BLOQUE_ARCHIVOS);
        return true;
    }
    return false;
}

bool FileVersionsFile::readBloque(int nroBloque)
{
	if (_filestr.is_open()) {
		if(_bloqueActual != 0){
			writeBloque(); // escribo el bloque actual;
			delete _bloqueActual;
		}

		_bloqueActual = new FileBlock();

		_filestr.seekg((nroBloque + 1) * FileBlock::TAMANIO_BLOQUE_ARCHIVOS,ios::beg);
		_filestr.seekp((nroBloque + 1) * FileBlock::TAMANIO_BLOQUE_ARCHIVOS,ios::beg);

		_filestr.read(_buffer,FileBlock::TAMANIO_BLOQUE_ARCHIVOS);
		_bloqueActual->read(_buffer);

		return true;
	}

	return false;
}

bool FileVersionsFile::writeBloque()
{
	if (_filestr.is_open()) {
		
		if(_bloqueActual != 0){
			_bloqueActual->write(_buffer);	//escribo el bloque en el buffer

			_filestr.seekg((_bloqueActual->getNumero() + 1) * FileBlock::TAMANIO_BLOQUE_ARCHIVOS,ios::beg);
			_filestr.seekp((_bloqueActual->getNumero() + 1) * FileBlock::TAMANIO_BLOQUE_ARCHIVOS,ios::beg);

			_filestr.write(_buffer,FileBlock::TAMANIO_BLOQUE_ARCHIVOS);

			return true;
		}
		return false;
	}

	return false;
}

bool FileVersionsFile::crearBloque(int Anterior, int Siguiente)
{

	if (_filestr.is_open()) {
		writeBloque(); // escribo el bloque actual;
		_bloqueActual = new FileBlock(_cantBloques,Anterior,Siguiente);
		return true;
	}

	return false;
}

bool FileVersionsFile::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("creating FileVersionsFile in '" + a_Filename + "'\n");
    _filestr.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_filestr.is_open()) {
		_filestr.open(a_Filename.c_str(), ios::out | ios::binary);
		_filestr.close();
		_filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
		_isOpen = _filestr.is_open();
	}

    _cantBloques = 0;
    _isOpen = _isOpen && writeHeader();
    _filename = a_Filename;
    debug("FileVersionsFile creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool FileVersionsFile::destroy()
{
    if (_isOpen) 
        return false;

    debug("destroying FileVersionsFile '" + _filename + "'\n");
    int ret = remove(_filename.c_str());
    debug("FileVersionsFile destroy " + string((ret == 0) ? "successfull" : "failed") + "\n");
    return ret == 0;
}

bool FileVersionsFile::open(const string& a_Filename)
{
    if (_isOpen) 
        return true;

    debug("opening FileVersionsFile '" + a_Filename + "'\n");
    _filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);

	_isOpen = _filestr.is_open() && readHeader();
    _filename = a_Filename;
    debug("FileVersionsFile open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool FileVersionsFile::close()
{
    if (!_isOpen)
        return true;

    debug("closing FileVersionsFile '" + _filename + "'\n");
	_isOpen = _filestr.is_open() && writeHeader() && writeBloque();
    if (_isOpen) {
        _filestr.close();
        _isOpen = _filestr.is_open();
	}
    debug("FileVersionsFile close " + string(!_isOpen ? "successfull" : "failed") + "\n");
    return !_isOpen;
}

void FileVersionsFile::insertVersion(int nroVersion, const char* User, tm Fecha, long int Offset, char Tipo, FileVersion::t_versionType VersionType, int* nroBloqueNuevo)
{
	writeBloque(); // escribo el bloque actual
	delete _bloqueActual;
	_bloqueActual = new FileBlock(_cantBloques); // creo el nuevo bloque apuntado por el actual
	*nroBloqueNuevo = _cantBloques;	// guardo la referencia al nro del bloque nuevo
	_cantBloques++;	// incremento la cantidad de bloques del archivo
    
	//creo la version nueva
	FileVersion* version = new FileVersion(nroVersion, nroVersion, Fecha, User, Offset, Tipo, VersionType);
	_bloqueActual->insertVersion(version); // inserto la version
	delete version;
}

FileVersionsFile::t_status FileVersionsFile::insertVersion(int nroVersion, const char* User, tm Fecha, long int Offset, char Tipo, FileVersion::t_versionType VersionType, int bloque, int* nroBloqueNuevo)
{
	readBloque(bloque); // obtengo el bloque
	FileVersion* ultimaVersion = _bloqueActual->getLastVersion();
	int ultimoOriginal = ultimaVersion->getOriginal();
	delete ultimaVersion;
	FileVersion* nuevaVersion = new FileVersion(nroVersion, ultimoOriginal, Fecha, User, Offset, Tipo, VersionType);
	if (_bloqueActual->hayLugar(nuevaVersion)) {
		if (!_bloqueActual->searchVersion(nuevaVersion->getNroVersion())) {
			_bloqueActual->insertVersion(nuevaVersion);
			delete nuevaVersion;
			return FileVersionsFile::OK;
		}
		
		delete nuevaVersion;
		return FileVersionsFile::ERROR;
	}
	
	FileBlock* bloqueNuevo = new FileBlock(_cantBloques,_bloqueActual->getNumero());
	_cantBloques++;
	_bloqueActual->setSiguiente(bloqueNuevo->getNumero());
	writeBloque();                  // escribo el bloque actual modificado
	delete _bloqueActual;	        // elimino el bloque actual
	_bloqueActual = bloqueNuevo;	// seteo el bloque actual como el nuevo
	_bloqueActual->insertVersion(nuevaVersion);
	delete nuevaVersion;
	*nroBloqueNuevo = _bloqueActual->getNumero();
	return FileVersionsFile::OVERFLOW;
}

bool FileVersionsFile::searchVersion(FileVersion** version,int nroVersion,int bloque)
{
	readBloque(bloque);
	return _bloqueActual->searchVersion(nroVersion,version);
}


bool FileVersionsFile::getVersionFrom(int original, int final, int bloque, list<FileVersion>& lstVersions)
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
	FileVersion* auxVersion;
	_bloqueActual->searchVersion(original, &auxVersion);
	
	lstVersions.push_back(*auxVersion);
    //delete auxVersion;

    bool end = (auxVersion->getNroVersion() == final);

    // copio las restantes versiones del 1er bloque
    while (_bloqueActual->hasNext() && !end) {
		
        auxVersion = _bloqueActual->getNext();

        lstVersions.push_back(*auxVersion);

        if (auxVersion->getNroVersion() <= final)
		{
			if(auxVersion->getNroVersion() == final)
				end = true;
		}

		else
			end = true;

        //delete auxVersion;
    }

    int next = _bloqueActual->getSiguiente();
    while ( (!end)&&(next >= 0)) {
		
		readBloque(next);
        	
		_bloqueActual->moveFirst();

        while (_bloqueActual->hasNext() && !end) {
            auxVersion = _bloqueActual->getNext();
            if (auxVersion->getNroVersion() >= final)
                end = true;

            if(auxVersion->getNroVersion() <= final)
				lstVersions.push_back(*auxVersion);
            //delete auxVersion;
        }
		
		next = _bloqueActual->getSiguiente();
    }

	return true;
}

int FileVersionsFile::getLastOriginalVersionNumber(int bloque)
{
    readBloque(bloque);
    FileVersion* aux = _bloqueActual->getLastVersion();
    int ret = aux->getOriginal();
    delete aux;
    return ret;
}


int FileVersionsFile::getLastVersionNumber(int bloque)
{
    readBloque(bloque);
    FileVersion* aux = _bloqueActual->getLastVersion();
    int ret = aux->getNroVersion();
    delete aux;
    return ret;
}

