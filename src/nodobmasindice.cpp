#include "nodobmasindice.h"

const int NodoBMasIndice::BYTESREDONDEO = 4;

NodoBMasIndice::NodoBMasIndice(int Numero, int Nivel, int Padre, int HijoIzquierdo, char* key, int ref)
                               : NodoBMas(Numero, Nivel, Padre)
{   
    _hijoIzquierdo = HijoIzquierdo;
    
    if (BYTESREDONDEO > 0) {
		_redondeo = new char[BYTESREDONDEO + 1];
        strcpy(_redondeo, "****");
        _redondeo[BYTESREDONDEO] = 0;
    }

    if (ref != 0) {
        insertPair(key, ref);
        _nclaves = 1;
    }
    else _nclaves = 0;
}

/*NodoBMasIndice::~NodoBMasIndice(){
	delete _redondeo;
}*/

void NodoBMasIndice::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    memcpy(&_hijoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    readDatos(&nextByte);

    if (BYTESREDONDEO > 0)
        memcpy(&_redondeo, nextByte, sizeof(char) * BYTESREDONDEO);
}

void NodoBMasIndice::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    memcpy(nextByte, &_hijoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    writeDatos(&nextByte);

    if(BYTESREDONDEO > 0)
        memcpy(nextByte, &_redondeo, sizeof(char) * BYTESREDONDEO);
}

int NodoBMasIndice::searchFile(const char* key)
{
    int ret = _hijoIzquierdo;

    char* auxKey;
	char* auxFileName;
    int auxRef;
	int tamanioClave;
	int offset = 0;

	if(_nclaves){
		int i = 0;
		while (i < _nclaves) {
      
			memcpy(&tamanioClave,_pares + offset,sizeof(int));
			offset += sizeof(int);

			auxKey = new char[(tamanioClave + 1) * sizeof(char)];
			memcpy(auxKey,
					_pares + offset,sizeof(char)*tamanioClave);
			auxKey[tamanioClave] = 0;
			offset += tamanioClave * sizeof(char);

			auxFileName = new char[(tamanioClave - 4) * sizeof(char)];
			memcpy(auxFileName,auxKey,(tamanioClave - 5));
			auxFileName[tamanioClave - 5] = 0;

			memcpy(&auxRef,
					_pares + offset 
					,sizeof(int));
			offset += sizeof(int);

			if(strcmp(key, auxFileName) < 0){
				 delete auxKey;
				 delete auxFileName;
				 return ret;
			}
			
			else{				
				ret = auxRef;
				i++;
				delete auxKey;
				delete auxFileName;
			}
		}

		return ret;
	}

	return ret;
}

int NodoBMasIndice::searchFileAndVersion(const char* fileName,int version)
{
    int ret = _hijoIzquierdo;

    char* auxKey;
	char* auxFileName;
	char* versionStr;
    int auxRef;
	int tamanioClave;
	int offset = 0;

	if(_nclaves){
		int i = 0;
		while (i < _nclaves) {
      
			memcpy(&tamanioClave,_pares + offset,sizeof(int));
			offset += sizeof(int);

			auxKey = new char[(tamanioClave + 1) * sizeof(char)];
			memcpy(auxKey,
					_pares + offset,sizeof(char)*tamanioClave);
			auxKey[tamanioClave] = 0;
			offset += tamanioClave * sizeof(char);

			auxFileName = new char[(tamanioClave - 4) * sizeof(char)];
			memcpy(auxFileName,auxKey,(tamanioClave - 5));
			auxFileName[tamanioClave - 5] = 0;

			memcpy(&auxRef,
					_pares + offset 
					,sizeof(int));
			offset += sizeof(int);

			if(strcmp(fileName, auxFileName) < 0){
				 delete auxKey;
				 delete auxFileName;
				 return ret;
			}
			
			else if(strcmp(fileName,auxFileName) > 0){				
				ret = auxRef;
				i++;
				delete auxKey;
				delete auxFileName;
			}

			else{
				versionStr = new char[6];

				memcpy(versionStr,auxKey + (tamanioClave - 5),5*sizeof(char));
				versionStr[5] = 0;

				int auxVersion = atoi(versionStr);

				delete versionStr;
				delete auxKey;
				delete auxFileName;

				if(version >= auxVersion){
					ret = auxRef;
					i++;
				}

				else
					return ret;			
			}
		}

		return ret;
	}

	return ret;
}

NodoBMas* NodoBMasIndice::split(int Numero, char* arreglo,int bytesArreglo,int clavesArreglo,
							char** claveAlPadre)
{
	NodoBMasIndice* ret;
    int auxRef;
    char* auxKey;
	int longClave;
	int offsetArreglo = 0;

	int clavesCopiadas = 0;

	// si tengo 1 sola clave en el arreglo trato de poder sacar alguna del nodo actual
	// para poder insertarla en el nuevo nodo
	if(( clavesArreglo == 1)&&(_espacioLibre < TAMANIOARREGLO /2)){

		int clavesLeidas = 0;

		while(offsetArreglo < TAMANIOARREGLO /2){
			// tamanio de la clave
			memcpy(&longClave,_pares + offsetArreglo,sizeof(int));
			offsetArreglo += sizeof(int);

			// avanzo la cantidad de caracteres de la clave
			offsetArreglo += sizeof(char) * longClave; 

			// avanzo la cantidad de bytes que ocupa la referencia
			offsetArreglo += sizeof(int);

			clavesLeidas++;
		}

		// nuevo offset en el arreglo del nodo actual
		int nuevoOffset = offsetArreglo;
		
		// copio a partir de ahora todas las claves que restan al nuevo nodo
		for(int i = clavesLeidas; i < _nclaves;i++){
			memcpy(&longClave,_pares + offsetArreglo,sizeof(int));
			offsetArreglo += sizeof(int);

			auxKey = new char[(longClave +1) * sizeof(char)];
			memcpy(auxKey,_pares + offsetArreglo,sizeof(char)*longClave);
			auxKey[longClave] = 0;
			offsetArreglo += sizeof(char)*longClave;

			memcpy(&auxRef,_pares + offsetArreglo, sizeof(int));
			offsetArreglo += sizeof(int);

			if(!clavesCopiadas)
			{
				*claveAlPadre = new char[(longClave + 1) * sizeof(char)];
				strcpy(*claveAlPadre,auxKey);

				ret = new NodoBMasIndice(Numero,_nivel,_padre,auxRef);

			}

			else{
				ret->insert(auxKey,auxRef);
			}

			delete(auxKey);

			clavesCopiadas++;
		}

		_offset = nuevoOffset;
		_nclaves = clavesLeidas;
		_espacioLibre = TAMANIOARREGLO - _offset;
	}

	offsetArreglo = 0;
	// sino la 1º clave del nuevo nodo va a ser la que viene desde el arreglo
    int copiadasDelArreglo = 0;

	if(!clavesCopiadas){
		// tomo la clave menor para subir al _padre
		memcpy(&longClave,
			arreglo + offsetArreglo,
			sizeof(int));
		offsetArreglo += sizeof(int);

		*claveAlPadre = new char[(longClave + 1)*sizeof(char)];
		memcpy(*claveAlPadre,arreglo + offsetArreglo,longClave * sizeof(char));
		(*claveAlPadre)[longClave] = 0;
		offsetArreglo += longClave * sizeof(char);

		// tomo la referencia para poder setear el hijo izquierdo
		memcpy(&auxRef,arreglo + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		ret = new NodoBMasIndice(Numero,_nivel,_padre,auxRef);

		copiadasDelArreglo++;
	}

    // copio las claves restantes al nuevo nodo
    for (int j = copiadasDelArreglo; j < clavesArreglo; j++){
        memcpy(&longClave,arreglo + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		auxKey = new char[(longClave + 1)*sizeof(char)];			
		memcpy(auxKey,
               arreglo + offsetArreglo,
               sizeof(char)*longClave);
		auxKey[longClave] = 0;
		offsetArreglo += longClave * sizeof(char);

        memcpy(&auxRef,
               arreglo + offsetArreglo,
               sizeof(int));
		offsetArreglo += sizeof(int);

        ret->insert(auxKey,auxRef);
		delete auxKey;
    }

    return ret;
}

void NodoBMasIndice::promoteRoot(NodoBMas** nodo1,NodoBMas** nodo2,int id1,int id2,
								int clavesArreglo,char** arreglo,int bytesArreglo,
								char** claveARaiz)
{   
    int i;
    int auxRef;
    char* auxKey; 
	int tamanioClave;
	int offsetArreglo = 0;

    *nodo1 = new NodoBMasIndice(id1,_nivel,_id,_hijoIzquierdo);

	// copio todas las claves de la raiz al nodo1
    for(i = 0; i < _nclaves; ++i) {
        
		memcpy(&tamanioClave,_pares + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);
		
		auxKey = new char[(tamanioClave +1) * sizeof(char)];
		memcpy(auxKey,
                _pares + offsetArreglo,
                sizeof(char)*tamanioClave);
		auxKey[tamanioClave] = 0;
		offsetArreglo += tamanioClave * sizeof(char);

        memcpy(&auxRef,
                _pares + offsetArreglo,
                sizeof(int));
		offsetArreglo += sizeof(int);

        (*nodo1)->insert(auxKey,auxRef);

		delete(auxKey);
    }

	// completo hasta tener la mitad del nodo1 "ocupada"
	offsetArreglo = 0;
	int clavesCopiadas = 0;

	while((*nodo1)->getOffset() < TAMANIOARREGLO / 2)
	{
		memcpy(&tamanioClave,(*arreglo) + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		auxKey = new char[(tamanioClave +1)*sizeof(char)];
		memcpy(auxKey,(*arreglo) + offsetArreglo,sizeof(char)*tamanioClave);
		auxKey[tamanioClave] = 0;
		offsetArreglo += tamanioClave * sizeof(char);

		memcpy(&auxRef,(*arreglo) + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		(*nodo1)->insert(auxKey,auxRef);

		delete(auxKey);

		clavesCopiadas++;
	}

    // tomo la clave que va a ir a la raiz nueva y 
	// la referencia que va a ir al hijo izquierdo del nodo 2
	memcpy(&tamanioClave,(*arreglo) + offsetArreglo,sizeof(int));
	offsetArreglo += sizeof(int);

	*claveARaiz = new char[(tamanioClave + 1) * sizeof(char)];
	memcpy(*claveARaiz,(*arreglo) + offsetArreglo,sizeof(char)*tamanioClave);
	(*claveARaiz)[tamanioClave] = 0;
	offsetArreglo += sizeof(char)*tamanioClave;

	memcpy(&auxRef,(*arreglo) + offsetArreglo,sizeof(int));
	offsetArreglo += sizeof(int);

	clavesCopiadas++;

	// creo el nodo 2 con el hijo izquierdo apuntando a auxRef
    *nodo2 = new NodoBMasIndice(id2,_nivel,_id,auxRef);


	//copio las claves y referencias restantes del arreglo
    for (i = clavesCopiadas; i < clavesArreglo; ++i) {
        memcpy(&tamanioClave,(*arreglo) + offsetArreglo,sizeof(int));
		offsetArreglo += sizeof(int);

		auxKey = new char[(tamanioClave + 1)*sizeof(char)];
		memcpy(auxKey,
                (*arreglo) + offsetArreglo,
                sizeof(char)*tamanioClave);
		auxKey[tamanioClave] = 0;
		offsetArreglo += sizeof(char)*tamanioClave;

        memcpy(&auxRef,
                (*arreglo) + offsetArreglo,
                sizeof(int));
		offsetArreglo += sizeof(int);

        (*nodo2)->insert(auxKey,auxRef);

		delete (auxKey);
		clavesCopiadas++;
    }
}

