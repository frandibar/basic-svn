// nodobmas.cpp

#include "nodobmas.h"
#include <iostream>

const int NodoBMas::NODE_SIZE = 2048;  // arbitrario

// constructor
NodoBMas::NodoBMas(int id, int nivel, int padre)
{
    _nivel        = nivel;
    _id           = id;
    _padre        = padre;
    _espacioLibre = TAMANIOARREGLO;
    _offset       = 0;
    _nclaves      = 0;
}

bool NodoBMas::insertPair(const char* key, int ref, int* offset, int* tamanioArregloAux, char** arregloAux)
{
    bool end = false;

	int tamanioClave;
	int offsetInsercion;
	int auxRef;
	char* auxKey;
	int tamanioClaveAInsertar = strlen(key);
	*tamanioArregloAux = _nclaves;

    while ((*offset < _offset) && !end) {
		// guardo el offser donde prodria llegar a insertar
		offsetInsercion = *offset;
        // obtengo la clave y la referencia de la posicion i en la tira de bytes

		// tomo el tamanio de la clave
		memcpy(&tamanioClave,_pares + (*offset),sizeof(int));
		*offset += sizeof(int);

		// leo la clave
		auxKey = new char[(tamanioClave + 1) * sizeof(char)];
        
		memcpy(auxKey, _pares + (*offset), sizeof(char)*tamanioClave);
		auxKey[tamanioClave] = 0;
		*offset += tamanioClave * sizeof(char);
		
		// leo la referencia
        memcpy(&auxRef,
                _pares + (*offset),
                sizeof(int));
		*offset += sizeof(int);

        int cmp = strcmp(key,auxKey);
        if (cmp < 0) {
            // genero un arreglo auxiliar para poder contemplar los corrimientos en el arreglo
            // de claves
            *arregloAux = new char[(TAMANIOARREGLO - offsetInsercion) * sizeof(char)];
			memcpy(*arregloAux,_pares + offsetInsercion,TAMANIOARREGLO - offsetInsercion);

			// inserto tamanio de la clave, clave y referencia
			// tamanio clave
			memcpy(_pares + offsetInsercion,
					&tamanioClaveAInsertar,
					sizeof(int));
			offsetInsercion += sizeof(int);

			// clave
            memcpy(_pares + offsetInsercion, key, sizeof(char) * tamanioClaveAInsertar);
			offsetInsercion += tamanioClaveAInsertar * sizeof(char);

			// referencia
            memcpy(_pares + offsetInsercion, &ref, sizeof(int));
			offsetInsercion += sizeof(int);

			*offset = offsetInsercion;
            end = true;
        }

        else if (cmp == 0){
            std::cout << "la clave que se intenta insertar ya existe" << std::endl;
            return false;
        }

		if(!end)
			(*tamanioArregloAux)--;

		delete auxKey;

    }

    if (!end) { // si llego aca es porque tengo que insertar al final
        // inserto tamanio de la clave, clave y referencia
        // tamanio clave
        memcpy(_pares + (*offset), &tamanioClaveAInsertar, sizeof(int));
        *offset += sizeof(int);

        // clave
        memcpy(_pares + *offset, key, sizeof(char)*tamanioClaveAInsertar);
        *offset += tamanioClaveAInsertar * sizeof(char);

        // referencia
        memcpy(_pares + *offset, &ref, sizeof(int));
        *offset += sizeof(int);

        _offset = *offset;
    }

    return true;
}

void NodoBMas::completePairs(int* offset, int tamanioArreglo, char* arregloAux)
{
	char* nextByte = arregloAux;

	char* auxKey;
	int auxRef;
	int tamanioClave;
    for (int i = 0; i < tamanioArreglo; ++i) {

		// leo del arregloAuxiliar todos los valores que voy a copiar
		memcpy(&tamanioClave,nextByte,sizeof(int));
		nextByte += sizeof(int);
		
		auxKey = new char[tamanioClave * sizeof(char)];
		memcpy(auxKey,nextByte,tamanioClave * sizeof(char));
		nextByte += tamanioClave * sizeof(char);
		memcpy(&auxRef,nextByte,sizeof(int));
		nextByte += sizeof(int);

		//copio dentro del arreglo del nodo la informacion
		memcpy(_pares + (*offset),&tamanioClave,sizeof(int));
		(*offset) += sizeof(int);
		memcpy(_pares + (*offset),auxKey,tamanioClave * sizeof(char));
		(*offset) += tamanioClave * sizeof(char);
		memcpy(_pares + (*offset),&auxRef,sizeof(int));
		(*offset) += sizeof(int);
		
		delete(auxKey);
    }
}

bool NodoBMas::insertPair(const char* key, int ref)
{
	char* arregloAux = 0;
    int offset = 0,
		tamanioArregloAux = 0;

    if (insertPair(key, ref, &offset,&tamanioArregloAux,&arregloAux)) {
        if (tamanioArregloAux != 0) {
            completePairs(&offset,tamanioArregloAux,arregloAux);
			delete arregloAux;
		}
		_nclaves++;
		_espacioLibre -= (strlen(key) * sizeof(char) + 2 * sizeof(int));
		_offset = TAMANIOARREGLO - _espacioLibre;
        return true;
    }
    return false;
}

void NodoBMas::readInfoAdm(char** nextByte)
{   
    memcpy(&_nivel,*nextByte,sizeof(int));
    *nextByte += sizeof(int);

    memcpy(&_id,*nextByte,sizeof(int));
    *nextByte += sizeof(int);

    memcpy(&_padre,*nextByte,sizeof(int));
    *nextByte += sizeof(int);

	memcpy(&_espacioLibre,*nextByte,sizeof(int));
	*nextByte += sizeof(int);

	_offset = TAMANIOARREGLO - _espacioLibre;
}

void NodoBMas::readDatos(char** nextByte)
{
    memcpy(&_nclaves,*nextByte,sizeof(_nclaves));
    *nextByte += sizeof(_nclaves);
    memcpy(_pares,*nextByte,sizeof(char) * TAMANIOARREGLO);
    *nextByte += sizeof(char) * TAMANIOARREGLO;
}

void NodoBMas::writeInfoAdm(char** nextByte)
{
    memcpy(*nextByte,&_nivel,sizeof(int));
    *nextByte += sizeof(int);   
    memcpy(*nextByte,&_id,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(*nextByte,&_padre,sizeof(int));
    *nextByte += sizeof(int);
	memcpy(*nextByte,&_espacioLibre,sizeof(int));
	*nextByte += sizeof(int);
}

void NodoBMas::writeDatos(char** nextByte)
{
    memcpy(*nextByte,&_nclaves,sizeof(_nclaves));
    *nextByte += sizeof(_nclaves);
    memcpy(*nextByte,_pares,sizeof(char) * TAMANIOARREGLO);
    *nextByte += sizeof(char) * TAMANIOARREGLO;
}

bool NodoBMas::insertPair(const char* key, int ref, int *clavesArreglo, char** arregloAux, int* bytesArreglo)
{
	int offset = 0;
	int cantidadClaves = 0;

    bool end = false;

	int tamanioClave;
	int offsetInsercion;
	char* auxKey;
	int auxRef;
	int tamanioClaveAInsertar = strlen(key);
	*clavesArreglo = _nclaves + 1;

    while ((offset < _offset) && !end) {
		// guardo el offser donde prodria llegar a insertar
		offsetInsercion = offset;
        // obtengo la clave y la referencia de la posicion i en la tira de bytes

		// tomo el tamanio de la clave
		memcpy(&tamanioClave,_pares + offset,sizeof(int));
		offset += sizeof(int);

		// leo la clave
		auxKey = new char[(tamanioClave + 1) * sizeof(char)];
        
		memcpy(auxKey,
                _pares + offset,
                sizeof(char)*tamanioClave);
		auxKey[tamanioClave] = 0;
		offset += tamanioClave * sizeof(char);
		
		// leo la referencia
        memcpy(&auxRef,
                _pares + offset,
                sizeof(int));
		offset += sizeof(int);

        int cmp = strcmp(key,auxKey);
        if (cmp < 0) {
            // genero un arreglo auxiliar para poder contemplar los corrimientos en el arreglo
            // de claves
			*bytesArreglo = TAMANIOARREGLO - offsetInsercion + tamanioClaveAInsertar * sizeof(char) + 2 * sizeof(int);
            *arregloAux = new char[(*bytesArreglo) * sizeof(char)];
			int offsetArreglo = 0;
			
			// copio la clave que quiero insertar en el arreglo junto con los datos administrativos
			// inserto tamaño de la clave, clave y referencia
			// tamaño clave
			memcpy(*arregloAux + offsetArreglo,
					&tamanioClaveAInsertar,
					sizeof(int));
			offsetArreglo += sizeof(int);

			// clave
            memcpy(*arregloAux + offsetArreglo, 
                    key , 
                    sizeof(char)*tamanioClaveAInsertar);
			offsetArreglo += tamanioClaveAInsertar * sizeof(char);

			// referencia
            memcpy(*arregloAux + offsetArreglo,
                    &ref,
                    sizeof(int));
			offsetArreglo += sizeof(int);
			
			// copio el resto del arreglo que le sigue a la clave nueva
			memcpy(*arregloAux + offsetArreglo,_pares + offsetInsercion,TAMANIOARREGLO - offsetInsercion);

            end = true;
        }

        else if (cmp == 0) {
            std::cout << "la clave que se intenta insertar ya existe" << std::endl;
            return false;
        }
			
		if (!end) {
			(*clavesArreglo)--;
			cantidadClaves++;
		}

		delete auxKey;

    }

	if (!end) {
        *bytesArreglo = tamanioClaveAInsertar * sizeof(char) + 2 * sizeof(int);
        *arregloAux = new char[(*bytesArreglo) * sizeof(char)];
        int offsetArreglo = 0;

        // copio la clave que quiero insertar en el arreglo junto con los datos administrativos
        // inserto tamaño de la clave, clave y referencia
        // tamaño clave
        memcpy(*arregloAux + offsetArreglo,
                &tamanioClaveAInsertar,
                sizeof(int));
        offsetArreglo += sizeof(int);

        // clave
        memcpy(*arregloAux + offsetArreglo, key, sizeof(char)*tamanioClaveAInsertar);
        offsetArreglo += tamanioClaveAInsertar * sizeof(char);

        // referencia
        memcpy(*arregloAux + offsetArreglo, &ref, sizeof(int));
        offsetArreglo += sizeof(int);
	}

	if (end)
		_offset = offsetInsercion;

	_nclaves = cantidadClaves;
	_espacioLibre = TAMANIOARREGLO - _offset;

	return true;
}

void NodoBMas::completePairs(int* tamanioArreglo, char **arregloAux, int* bytesArreglo)
{
	char* arregloAux2;
	char* auxKey;
	int tamanioClaveAux;
	int auxRef;

	int offsetArreglo = 0;

	while (TAMANIOARREGLO / 2 <= _espacioLibre) {

		// levanto tamanio de la clave, clave y referencia
		memcpy(&tamanioClaveAux,*arregloAux + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		auxKey = new char[tamanioClaveAux * sizeof(char)];
		memcpy(auxKey,*arregloAux + offsetArreglo,tamanioClaveAux * sizeof(char));
		offsetArreglo += tamanioClaveAux * sizeof(char);

		memcpy(&auxRef,*arregloAux + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		(*tamanioArreglo)--;
		
		// ahora copio en el arreglo del nodo estos datos
		memcpy(_pares + _offset,&tamanioClaveAux,sizeof(int));
		_offset += sizeof(int);

		memcpy(_pares + _offset,auxKey,tamanioClaveAux * sizeof(char));
		_offset += tamanioClaveAux * sizeof(char);

		memcpy(_pares + _offset,&auxRef,sizeof(int));
		_offset += sizeof(int);

		_nclaves++;
		_espacioLibre = TAMANIOARREGLO - _offset;

		delete auxKey;
	}

	int bytesACopiar = *bytesArreglo - offsetArreglo;
	arregloAux2 = new char[bytesACopiar * sizeof(char)];
	memcpy(arregloAux2,*arregloAux + offsetArreglo,bytesACopiar);
	delete(*arregloAux);
	*arregloAux = arregloAux2;
	*bytesArreglo = bytesACopiar;
}

NodoBMas::t_status NodoBMas::insert(const char* key, int ref, int* clavesArreglo, char** arregloAux, int* bytesArreglo)
{
    int tamanioClaveYRef = strlen(key) * sizeof(char) + sizeof(int) * 2;
	
	if (tamanioClaveYRef <= _espacioLibre)
        return (insertPair(key,ref) ? OK : ERROR);

	else if (insertPair(key, ref,clavesArreglo,arregloAux,bytesArreglo)) {
        completePairs(clavesArreglo, arregloAux,bytesArreglo);            
        return OVERFLOW;  // se inserto normalmente pero con overflow
    }
    else
        return ALREADY_EXISTS; // no se pudo insertar porque la clave ya estaba
}


int NodoBMas::getRef(int idNode)
{
    int ret = 0;
	int tamanioClave = 0;
	int offsetArreglo = 0;

	for (int i = 0; i < idNode; ++i) {
		memcpy(&tamanioClave, _pares + offsetArreglo, sizeof(int));
		offsetArreglo += sizeof(int) + tamanioClave * sizeof(char);
		memcpy(&ret, _pares + offsetArreglo, sizeof(int));
		offsetArreglo += sizeof(int);
	}
    return ret;
}
