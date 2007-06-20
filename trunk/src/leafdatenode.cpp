#include "leafdatenode.h"

LeafDateNode::LeafDateNode(int Numero, int Padre, int HnoIzquierdo, int HnoDerecho)
    : DateNode(Numero, 0, Padre)
{
    _hnoIzquierdo = HnoIzquierdo;
    _hnoDerecho   = HnoDerecho;
}

void LeafDateNode::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    readDatos(&nextByte);
    memcpy(&_hnoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    memcpy(&_hnoDerecho, nextByte, sizeof(int));
}

void LeafDateNode::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    writeDatos(&nextByte);
    memcpy(nextByte, &_hnoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    memcpy(nextByte, &_hnoDerecho, sizeof(int));
}

int LeafDateNode::search(const char* key)
{
    char auxKey[LONGCLAVE + 1];
    int auxRef;
    int i = 0;

	int offset = 0;
    while (i < _nclaves) {
        memcpy(auxKey, 
               _pares + offset, 
               sizeof(char) * LONGCLAVE);
		auxKey[LONGCLAVE] = 0;
		offset += sizeof(char) * LONGCLAVE;

        memcpy(&auxRef, 
               _pares + offset, 
               sizeof(int));
		offset += sizeof(int);

        if (strcmp(key, auxKey) == 0) {
            return auxRef;
        }
        else
            i++;
    }

    return -1;
}

IndexDateNode* LeafDateNode::convertirAIndice(int HijoIzquierdo, int primerRef,const char* key)
{
    IndexDateNode* ret = new IndexDateNode(_id, _nivel + 1, _padre, HijoIzquierdo);

    ret->insert(key, primerRef);
    return ret;
}

void LeafDateNode::promoteRoot(DateNode** nuevo_nodo, int id)
{
    int i;
    int auxRef;
    char auxKey[LONGCLAVE];

    *nuevo_nodo= new LeafDateNode(id, _id, -1,-1);

	int offset = 0;

    for (i = 0; i < MAXCLAVES; ++i) {
        memcpy(auxKey,
               _pares + offset,
               sizeof(char) * LONGCLAVE);
		offset += sizeof(char) * LONGCLAVE;

        memcpy(&auxRef,
               _pares + offset,
               sizeof(int));
		offset += sizeof(int);

        (*nuevo_nodo)->insert(auxKey, auxRef);
    }

	return;
}

