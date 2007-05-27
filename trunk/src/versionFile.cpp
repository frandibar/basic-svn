#include "versionFile.h"

using std::ios;

VersionFile::VersionFile(){
	_buffer = new char[Bloque::TAMANIOBLOQUE];
	
	_bloqueActual = 0;
	
	_cantBloques = 0;
}	

VersionFile::~VersionFile(){
	delete _bloqueActual;
	delete _buffer;
}

bool VersionFile::readHeader(){
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, Bloque::TAMANIOBLOQUE);
        // leo la cantidad de nodos
        memcpy(&_cantBloques, nextByte, sizeof(int));
        nextByte += sizeof(int);

		return true;
    }
    return false;
}

bool VersionFile::writeHeader(){
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_cantBloques, sizeof(int));
        nextByte += sizeof(int);
        // volcar al archivo
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, Bloque::TAMANIOBLOQUE);
        return true;
    }
    return false;
}

bool VersionFile::readBloque(int nroBloque){
	if(_filestr.is_open()){
		writeBloque(); // escribo el bloque actual;

		_filestr.seekg((nroBloque + 1) * Bloque::TAMANIOBLOQUE,ios::beg);
		_filestr.seekp((nroBloque + 1) * Bloque::TAMANIOBLOQUE,ios::beg);

		_filestr.read(_buffer,Bloque::TAMANIOBLOQUE);

		_bloqueActual->read(_buffer);

		return true;
	}

	return false;
}

bool VersionFile::writeBloque(){
	if(_filestr.is_open()){
		
		if(_bloqueActual != 0){
			_bloqueActual->write(_buffer);	//escribo el bloque en el buffer

			_filestr.seekg((_bloqueActual->getNumero() + 1) * Bloque::TAMANIOBLOQUE,ios::beg);
			_filestr.seekp((_bloqueActual->getNumero() + 1) * Bloque::TAMANIOBLOQUE,ios::beg);

			_filestr.write(_buffer,Bloque::TAMANIOBLOQUE);

			return true;
		}

		return false;
	}

	return false;
}

bool VersionFile::crearBloque(int Anterior, int Siguiente){

	if(_filestr.is_open()){
		writeBloque(); // escribo el bloque actual;
		
		_bloqueActual = new Bloque(_cantBloques,Anterior,Siguiente);

		return true;
	}

	return false;
}

bool VersionFile::create(char* fileName){
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

bool VersionFile::open(char* fileName){
    _filestr.open(fileName, ios::in | ios::out | ios::binary);

	if(_filestr.is_open()){
		if (readHeader())
				return true;
		return false;
	}    
	return false;
}

bool VersionFile::close(){
	
	if(_filestr.is_open()){
		
		if(writeHeader()){

			if(writeBloque()){
				_filestr.close();
				return true;
			}

		}
		
		return false;
	}

	return false;
}

void VersionFile::insertVersion(Version* version,int* nroBloqueNuevo){
	
	writeBloque(); // escribo el bloque actual

	delete _bloqueActual;
		
	_bloqueActual = new Bloque(_cantBloques); // creo el nuevo bloque apuntado por el actual

	*nroBloqueNuevo = _cantBloques;	// guardo la referencia al nro del bloque nuevo

	_cantBloques++;	// incremento la cantidad de bloques del archivo

	_bloqueActual->insertVersion(version); // inserto la version

	return;
}

int VersionFile::insertVersion(Version* version,int bloque,int* nroBloqueNuevo){

	readBloque(bloque); // obtengo el bloque

	if(_bloqueActual->hayLugar(version)){

		if( !_bloqueActual->searchVersion(version->getNroVersion()) ){
			_bloqueActual->insertVersion(version);
			return 1;
		}

		return 0;
	}
	
	Bloque* bloqueNuevo = new Bloque(_cantBloques,_bloqueActual->getNumero());

	_cantBloques++;

	_bloqueActual->setSiguiente(bloqueNuevo->getNumero());

	writeBloque(); // escribo el bloque actual modificado

	delete _bloqueActual;	// elimino el bloque actual

	_bloqueActual = bloqueNuevo;	// seteo el bloque actual como el nuevo

	_bloqueActual->insertVersion(version);

	*nroBloqueNuevo = _bloqueActual->getNumero();

	return 2;
}

bool VersionFile::searchVersion(Version** version,int nroVersion,int bloque){

	readBloque(bloque);

	return _bloqueActual->searchVersion(nroVersion,version);
}
