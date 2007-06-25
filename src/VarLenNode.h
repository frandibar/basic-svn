#ifndef VARLENNODE_H_INCLUDED
#define VARLENNODE_H_INCLUDED

#include "Node.h"

#include <cstdlib>
#include <iostream>
#include <string>

#define VARLEN_STREAM_SIZE 2020

class VarLenNode : public Node
{
public:
   const static int VARLEN_NODE_SIZE;   

   VarLenNode(int id = 0, int nivel = 0, int padre = -1);
   virtual ~VarLenNode() {};
   
   t_status insert(const char* key, int ref, int* clavesArreglo, char** arreglo, int* bytesArreglo);
   bool insert(const char* key, int ref) {   
      return insertPair(key, ref);
   }
   
   bool isFull() const { return _espacioLibre == 0; }
   
   //getters
   int getOffset()   const { return _offset; }
   int getRef(int idNode);

   // generador de un nodo del mismo _nivel para cuando tengo que hacer un split
   virtual VarLenNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre) = 0;
   
   // me genera 2 nuevos nodos al tener que promover la raiz
   virtual void promoteRoot(VarLenNode** nodo1, VarLenNode** nodo2, int id1, int id2, int clavesArreglo, char** arreglo, int bytesArreglo, char** claveARaiz)  = 0;         
   
protected:
    bool insertPair(const char* key, int ref);
    bool insertPair(const char* key, int ref, int* offset, int* clavesArregloAux, char** arregloAux);
    bool insertPair(const char* key, int ref, int* clavesArreglo, char** arregloAux, int* bystesArreglo);

    void completePairs(int* offset, int tamanioArreglo, char* arregloAux);
    void completePairs(int* tamanioArreglo, char** arregloAux, int* bytesArreglo);            

    void readInfoAdm (char** nextByte);
    void writeInfoAdm(char** nextByte);

    void readDatos   (char** nextByte);
    void writeDatos  (char** nextByte);

   //member variables
   int _espacioLibre;
   int _offset;
   char _pares[VARLEN_STREAM_SIZE];   
};

#endif
