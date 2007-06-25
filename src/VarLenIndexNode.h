
#ifndef VARLENINDEXNODE_H_INCLUDED
#define VARLENINDEXNODE_H_INCLUDED

#include "VarLenNode.h"

class VarLenIndexNode : public VarLenNode
{
public:
   static const int BYTESREDONDEO;

   // constructors
   VarLenIndexNode(int id = 0, int nivel = 0, int padre = -1, int HijoIzquierdo = 0, char* key = 0, int ref = 0);
	virtual ~VarLenIndexNode();

   void read(char* buffer);
   void write(char* buffer);

   int search(const char* key);

   VarLenNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre);

   void promoteRoot(VarLenNode** nodo1, VarLenNode** nodo2, int id1, int id2, int clavesArreglo, char** arreglo, int bytesArreglo,
   char** claveARaiz);

   // getters
   int getHijoIzquierdo() const { return _hijoIzquierdo; }
   t_nodeType getType() const { return INDEX; }

protected:
    int   _hijoIzquierdo;            // la referencia al nodo con claves menores
    char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco
};

#endif
