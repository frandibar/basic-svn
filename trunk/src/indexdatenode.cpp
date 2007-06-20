#include "indexdatenode.h"

const int IndexDateNode::BYTESREDONDEO = 4;

IndexDateNode::IndexDateNode(int Numero, int Nivel, int Padre, int HijoIzquierdo,const char* key, int ref)
                               : DateNode(Numero, Nivel, Padre)
{
    _redondeo = new char[BYTESREDONDEO];
    _hijoIzquierdo = HijoIzquierdo;
    
    if (BYTESREDONDEO > 0) {
        memcpy(_redondeo, "****",BYTESREDONDEO * sizeof(char));
        _redondeo[BYTESREDONDEO] = 0;
    }

    if (ref != 0) {
        insertPair(key, ref);
        _nclaves = 1;
    }
    else _nclaves = 0;
}

void IndexDateNode::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    memcpy(&_hijoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    readDatos(&nextByte);

    if (BYTESREDONDEO > 0)
        memcpy(&_redondeo, nextByte, sizeof(char) * BYTESREDONDEO);
}

void IndexDateNode::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    memcpy(nextByte, &_hijoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    writeDatos(&nextByte);

    if(BYTESREDONDEO > 0)
        memcpy(nextByte, &_redondeo, sizeof(char) * BYTESREDONDEO);
}

int IndexDateNode::search(const char* key)
{
    int ret = _hijoIzquierdo;

    char auxKey[LONGCLAVE + 1];
    int auxRef;

    int i = 0;
	int offset = 0;
    while (i < _nclaves) {
        memcpy(auxKey,
                _pares+ offset,sizeof(char)*LONGCLAVE);
		auxKey[LONGCLAVE] = 0;
		offset += sizeof(char)*LONGCLAVE;

        memcpy(&auxRef,
                _pares+ offset,sizeof(int));
		offset += sizeof(int);

        if(strcmp(key, auxKey) < 0)
            return ret;
        else {
            ret = auxRef;
            i++;
        }
    }
    return ret;
}

void IndexDateNode::promoteRoot(DateNode** nuevo_nodo, int id)
{   
    int i;
    int auxRef;
    char auxKey[LONGCLAVE];

    *nuevo_nodo = new IndexDateNode(id,_nivel,_id,_hijoIzquierdo);

	int offset = 0;

    for(i = 0; i < MAXCLAVES; ++i) {
        memcpy(auxKey,
                _pares + offset,
                sizeof(char)*LONGCLAVE);
		offset += sizeof(char)*LONGCLAVE;

        memcpy(&auxRef,
                _pares + offset,
                sizeof(int));
		offset += sizeof(int);

        (*nuevo_nodo)->insert(auxKey,auxRef);
    }

	_nclaves = 0;

	return;
}

