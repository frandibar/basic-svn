
#ifndef NODOBMASINDICE_H_INCLUDED
#define NODOBMASINDICE_H_INCLUDED

#include "FileDirNode.h"

class FileDirIndexNode : public FileDirNode
{
public:
   static const int BYTESREDONDEO;

   // constructors
   FileDirIndexNode(int id = 0, int nivel = 0, int padre = -1, int HijoIzquierdo = 0, char* key = 0, int ref = 0); 
	~FileDirIndexNode();

   void read(char* buffer);
   void write(char* buffer);

   int search(const char* key);
	int searchFileAndVersion(const char* fileName,int version);
   int getFirstOf(const char* key);

   FileDirNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre);
   void promoteRoot(VarLenNode** nodo1, VarLenNode** nodo2, int id1, int id2, int clavesArreglo, char** arreglo, int bytesArreglo, char**  claveARaiz);

   // getters
   int getHijoIzquierdo() const { return _hijoIzquierdo; }
   t_nodeType getType() const { return INDEX; }

protected:
   int   _hijoIzquierdo;            // la referencia al nodo con claves menores
   char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco
};

#endif
