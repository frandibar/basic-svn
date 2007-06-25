#ifndef FIXLENNODE_H_INLCUDED
#define FIXLENNODE_H_INLCUDED

#include "Node.h"

#include <cstdlib>
#include <iostream>
#include <string>

#define FIXLEN_STREAM_SIZE 490
#define KEY_LENGTH 10

class FixLenNode : public Node
{
public:
   const static int FIXLEN_NODE_SIZE;   
   const static int MAXKEYS;

   FixLenNode(int id = 0, int nivel = 0, int padre = -1);
   virtual ~FixLenNode() {};

   virtual int search(const char* key) = 0;
    
   t_status insert(const char* key, int ref);

   bool isFull() const { return _nclaves == MAXKEYS; }

   // getters
   int getRef(int idNode);

   // me genera 2 nuevos nodos al tener que promover la raiz
   virtual void promoteRoot(FixLenNode** nuevo_nodo,int id)  = 0;

protected:
   bool insertPair(const char* key, int ref);
   bool insertPair(const char* key, int ref, int* i, char* auxKey, int* auxRef);
   void completePairs(int* i, int* j, char* auxKey, int* auxRef);

   void readInfoAdm (char** nextByte);
   void readDatos   (char** nextByte);

   void writeInfoAdm(char** nextByte);
   void writeDatos  (char** nextByte);

   // member variables
   char  _pares[FIXLEN_STREAM_SIZE];// pares clave-referencia en el caso de los nodos hoja
                                    // pares clave-hijo claves mayores en el caso de los interiores   
};

#endif
