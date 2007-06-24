//fixlennode.cpp

#include "fixlennode.h"
#include <iostream>

// CONSTS
const int FixLenNode::MAXKEYS           = 35;
const int FixLenNode::FIXLEN_NODE_SIZE  = 512;

// constructor
FixLenNode::FixLenNode(int id, int nivel, int padre) : Node(id, nivel, padre)
{}

bool FixLenNode::insertPair(const char* key, int ref, int* i, char* auxKey, int* auxRef)
{
    bool end = false;

    while (((*i) <_nclaves) && (!end) ) {
        // obtengo la clave y la referencia de la posicion i en la tira de bytes
        memcpy(auxKey, _pares+(( KEY_LENGTH+sizeof(int) )* (*i)), sizeof(char)*KEY_LENGTH);
        memcpy(auxRef, _pares+(( KEY_LENGTH+sizeof(int) )* (*i) + KEY_LENGTH * sizeof(char)), sizeof(int));

        int cmp = strcmp(key, auxKey);
        if (cmp < 0) {
            //inserto clave y referencia
            memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )* (*i)), key, sizeof(char)*KEY_LENGTH);
            memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )* (*i) + KEY_LENGTH * sizeof(char) ), &ref, sizeof(int));
            end = true;
        }
        else if (cmp > 0)
            (*i)++;
        else {
            std::cout << "la clave que se intenta insertar ya existe" << std::endl;
            return false;
        }
    }
    if ((*i) >= _nclaves) {
        memcpy(auxKey, key, KEY_LENGTH * sizeof(char));
        memcpy(auxRef, &ref, sizeof(int));
    }
    return true;
}

void FixLenNode::completePairs(int* i,int* j,char auxKey[KEY_LENGTH],int* auxRef)
{
   (*i)++;
   for ((*j) = (*i); (*j) < _nclaves; ++(*j)) {
      char auxKey2[KEY_LENGTH];
      int auxRef2;

      memcpy(auxKey2, _pares+( (KEY_LENGTH * sizeof(char) + sizeof(int))* (*j)), sizeof(char) * KEY_LENGTH);
      memcpy(&auxRef2, _pares+( (KEY_LENGTH * sizeof(char) + sizeof(int))* (*j) + KEY_LENGTH * sizeof(char)), sizeof(int));

      memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )* (*j)), auxKey, sizeof(char)*KEY_LENGTH);
      memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )* (*j) + KEY_LENGTH * sizeof(char) ), auxRef, sizeof(int));

      strcpy(auxKey, auxKey2);
      *auxRef = auxRef2;
   }

   return;
}

bool FixLenNode::insertPair(const char* key, int ref)
{
   char auxKey[KEY_LENGTH];
   int auxRef, i = 0, j;

   if (insertPair(key, ref, &i, auxKey, &auxRef)) {
      if (i < _nclaves) {
            
         completePairs(&i, &j, auxKey, &auxRef);

         memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )*j), auxKey, sizeof(char)*KEY_LENGTH);

         memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )*j+ KEY_LENGTH * sizeof(char) ), &auxRef, sizeof(int));                    
         _nclaves++;
      }
                
      else if ( i == _nclaves) {
         memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )*_nclaves), key, sizeof(char)*KEY_LENGTH);
         memcpy(_pares+( (KEY_LENGTH*sizeof(char) + sizeof(int) )*_nclaves + KEY_LENGTH * sizeof(char) ), &ref, sizeof(int));
         _nclaves++;
      }
        return true;
    }
  return false;
}

void FixLenNode::readInfoAdm(char** nextByte)
{   
    memcpy(&_nivel,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(&_id,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(&_padre,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
}

void FixLenNode::readDatos(char** nextByte)
{
    memcpy(&_nclaves,*nextByte,sizeof(short int));
    *nextByte += sizeof(short int);
    memcpy(_pares,*nextByte,sizeof(char) * FIXLEN_STREAM_SIZE);
    *nextByte += sizeof(char) * FIXLEN_STREAM_SIZE;
}

void FixLenNode::writeInfoAdm(char** nextByte)
{
    memcpy(*nextByte,&_nivel,sizeof(int));
    *nextByte += sizeof(int);   
    memcpy(*nextByte,&_id,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(*nextByte,&_padre,sizeof(int));
    *nextByte += sizeof(int);
}

void FixLenNode::writeDatos(char** nextByte)
{
    memcpy(*nextByte,&_nclaves,sizeof(short int));
    *nextByte += sizeof(short int);
    memcpy(*nextByte,_pares,sizeof(char) * FIXLEN_STREAM_SIZE);
    *nextByte += sizeof(char) * FIXLEN_STREAM_SIZE;
}

FixLenNode::t_status FixLenNode::insert(const char* key, int ref)
{
   if (_nclaves < MAXKEYS)
   {
      if(insertPair(key,ref))
         return OK;
      
      return ERROR;
   }
   else 
      return OVERFLOW;
}

int FixLenNode::getRef(int idNode)
{
    int ret;
    memcpy(&ret, _pares+( (KEY_LENGTH * sizeof(char) + sizeof(int))* idNode + KEY_LENGTH * sizeof(char)), sizeof(int));
    return ret;
}
