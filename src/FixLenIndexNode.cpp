
#include "FixLenIndexNode.h"

const int FixLenIndexNode::BYTESREDONDEO = 4;

FixLenIndexNode::FixLenIndexNode(int id, int nivel, int padre, int HijoIzquierdo,const char* key, int ref) : FixLenNode(id, nivel, padre)
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

void FixLenIndexNode::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    memcpy(&_hijoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    readDatos(&nextByte);

    if (BYTESREDONDEO > 0)
        memcpy(&_redondeo, nextByte, sizeof(char) * BYTESREDONDEO);
}

void FixLenIndexNode::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    memcpy(nextByte, &_hijoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    writeDatos(&nextByte);

    if(BYTESREDONDEO > 0)
        memcpy(nextByte, &_redondeo, sizeof(char) * BYTESREDONDEO);
}

int FixLenIndexNode::search(const char* key)
{
   int ret = _hijoIzquierdo;
   char auxKey[KEY_LENGTH + 1];
   int auxRef;
   int i = 0;
	int offset = 0;
   while (i < _nclaves) {
      memcpy(auxKey, _pares+ offset,sizeof(char)*KEY_LENGTH);
		auxKey[KEY_LENGTH] = 0;
		offset += sizeof(char)*KEY_LENGTH;

      memcpy(&auxRef, _pares+ offset,sizeof(int));
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

void FixLenIndexNode::promoteRoot(FixLenNode** nuevo_nodo, int id)
{   
   int i;
   int auxRef;
   char auxKey[KEY_LENGTH];
   *nuevo_nodo = new FixLenIndexNode(id,_nivel,_id,_hijoIzquierdo);
	int offset = 0;

   for(i = 0; i < MAXKEYS; ++i) {
      memcpy(auxKey, _pares + offset, sizeof(char)*KEY_LENGTH);
		offset += sizeof(char)*KEY_LENGTH;

      memcpy(&auxRef, _pares + offset, sizeof(int));
		offset += sizeof(int);
      (*nuevo_nodo)->insert(auxKey,auxRef);
   }

	_nclaves = 0;
	return;
}
