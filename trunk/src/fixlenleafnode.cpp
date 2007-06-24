//fixlenleafnode.cpp

#include "fixlenleafnode.h"

FixLenLeafNode::FixLenLeafNode(int id, int padre, int HnoIzquierdo, int HnoDerecho) : FixLenNode(id, 0, padre)
{
    _hnoIzquierdo = HnoIzquierdo;
    _hnoDerecho   = HnoDerecho;
}

void FixLenLeafNode::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    readDatos(&nextByte);
    memcpy(&_hnoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    memcpy(&_hnoDerecho, nextByte, sizeof(int));
}

void FixLenLeafNode::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    writeDatos(&nextByte);
    memcpy(nextByte, &_hnoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    memcpy(nextByte, &_hnoDerecho, sizeof(int));
}

int FixLenLeafNode::search(const char* key)
{
   char auxKey[KEY_LENGTH + 1];
   int auxRef;
   int i = 0;

	int offset = 0;
   while (i < _nclaves) {
      memcpy(auxKey, _pares + offset, sizeof(char) * KEY_LENGTH);
		auxKey[KEY_LENGTH] = 0;
		offset += sizeof(char) * KEY_LENGTH;

      memcpy(&auxRef, _pares + offset, sizeof(int));
		offset += sizeof(int);

      if (strcmp(key, auxKey) == 0)
            return auxRef;    
      else
          i++;
  }

  return -1;
}

FixLenIndexNode* FixLenLeafNode::convertirAIndice(int HijoIzquierdo, int primerRef,const char* key)
{
    FixLenIndexNode* ret = new FixLenIndexNode(_id, _nivel + 1, _padre, HijoIzquierdo);

    ret->insert(key, primerRef);
    return ret;
}

void FixLenLeafNode::promoteRoot(FixLenNode** nuevo_nodo, int id)
{
   int i;
   int auxRef;
   char auxKey[KEY_LENGTH];

   *nuevo_nodo= new FixLenLeafNode(id, _id, -1,-1);

	int offset = 0;

   for (i = 0; i < MAXKEYS; ++i) {
      memcpy(auxKey, _pares + offset, sizeof(char) * KEY_LENGTH);
		offset += sizeof(char) * KEY_LENGTH;

      memcpy(&auxRef, _pares + offset, sizeof(int));
		offset += sizeof(int);

      (*nuevo_nodo)->insert(auxKey, auxRef);
   }
   return;
}

