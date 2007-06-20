#include "datenode.h"
#include <iostream>

// CONSTS
const int DateNode::MAXCLAVES      = 35;
const int DateNode::NODE_SIZE    = 512;

// constructor
DateNode::DateNode(int Numero, int Nivel, int Padre)
{
    _id = Numero;
    _nivel = Nivel;
    _padre = Padre;
    _nclaves = 0;
}

int DateNode::insertPair(const char* key, int ref, int* i, char* auxKey, int* auxRef)
{
    bool end = false;

    while (((*i) <_nclaves) && (!end) ) {
        // obtengo la clave y la referencia de la posicion i en la tira de bytes
        memcpy(auxKey,
                _pares+(( LONGCLAVE+sizeof(int) )* (*i)),
                sizeof(char)*LONGCLAVE);

        memcpy(auxRef,
                _pares+(( LONGCLAVE+sizeof(int) )* (*i) + LONGCLAVE * sizeof(char)),
                sizeof(int));

        int cmp = strcmp(key, auxKey);
        if (cmp < 0) {
            //inserto clave y referencia
            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )* (*i)), 
                    key , 
                    sizeof(char)*LONGCLAVE);

            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )* (*i) + LONGCLAVE * sizeof(char) ),
                    &ref,
                    sizeof(int));

            end = true;
        }

        else if (cmp > 0)
            (*i)++;
        else {
            std::cout << "la clave que se intenta insertar ya existe" << std::endl;
            return 0;
        }
    }

    if ((*i) >= _nclaves) {
        memcpy(auxKey, key, LONGCLAVE * sizeof(char));
        memcpy(auxRef, &ref, sizeof(int));
    }

    return 1;
}

void DateNode::completePairs(int* i,int* j,char auxKey[LONGCLAVE],int* auxRef)
{
    (*i)++;
    for ((*j) = (*i); (*j) < _nclaves; ++(*j)) {
        char auxKey2[LONGCLAVE];
        int auxRef2;

        memcpy(auxKey2,
                _pares+( (LONGCLAVE * sizeof(char) + sizeof(int))* (*j)),
                sizeof(char) * LONGCLAVE);

        memcpy(&auxRef2,
                _pares+( (LONGCLAVE * sizeof(char) + sizeof(int))* (*j) + LONGCLAVE * sizeof(char)),
                sizeof(int));


        memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )* (*j)), 
                auxKey , 
                sizeof(char)*LONGCLAVE);

        memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )* (*j) + LONGCLAVE * sizeof(char) ),
                auxRef,
                sizeof(int));

        strcpy(auxKey, auxKey2);
        *auxRef = auxRef2;
    }

    return;
}

int DateNode::insertPair(const char* key, int ref)
{
    char auxKey[LONGCLAVE];
    int auxRef,
        i = 0,
        j;

    if (insertPair(key, ref, &i, auxKey, &auxRef)) {
        if (i < _nclaves) {
            
            completePairs(&i, &j, auxKey, &auxRef);

            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )*j), 
                    auxKey , 
                    sizeof(char)*LONGCLAVE);

            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )*j+ LONGCLAVE * sizeof(char) ),
                    &auxRef,
                    sizeof(int));
                    
            _nclaves++;
        }
                
        else if ( i == _nclaves) {
            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )*_nclaves), 
                    key, 
                    sizeof(char)*LONGCLAVE);

            memcpy(_pares+( (LONGCLAVE*sizeof(char) + sizeof(int) )*_nclaves + LONGCLAVE * sizeof(char) ),
                    &ref,
                    sizeof(int));
                    
            _nclaves++;
        }

        return 1;
    }
    return 0;
}

void DateNode::readInfoAdm(char** nextByte)
{   
    memcpy(&_nivel,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(&_id,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(&_padre,*nextByte,sizeof(int));
    *nextByte += sizeof(int);
}

void DateNode::readDatos(char** nextByte)
{
    memcpy(&_nclaves,*nextByte,sizeof(short int));
    *nextByte += sizeof(short int);
    memcpy(_pares,*nextByte,sizeof(char) * STREAMSIZE);
    *nextByte += sizeof(char) * STREAMSIZE;
}

void DateNode::writeInfoAdm(char** nextByte)
{
    memcpy(*nextByte,&_nivel,sizeof(int));
    *nextByte += sizeof(int);   
    memcpy(*nextByte,&_id,sizeof(int));
    *nextByte += sizeof(int);
    memcpy(*nextByte,&_padre,sizeof(int));
    *nextByte += sizeof(int);
}

void DateNode::writeDatos(char** nextByte)
{
    memcpy(*nextByte,&_nclaves,sizeof(short int));
    *nextByte += sizeof(short int);
    memcpy(*nextByte,_pares,sizeof(char) * STREAMSIZE);
    *nextByte += sizeof(char) * STREAMSIZE;
}

int DateNode::insert(const char* key, int ref)
{
    if (_nclaves < MAXCLAVES)
        return insertPair(key,ref);
    else 
        return 2; //overflow -> devuelvo 2
}

int DateNode::getRef(int idNode)
{
    int ret;

    memcpy(&ret,
        _pares+( (LONGCLAVE * sizeof(char) + sizeof(int))* idNode + LONGCLAVE * sizeof(char)),
        sizeof(int));

    return ret;
}
