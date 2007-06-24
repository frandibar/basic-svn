//filedirleafnode.h

#ifndef FILEDIRLEAFNODE_H_INCLUDED
#define FILEDIRLEAFNODE_H_INCLUDED

#include "filedirnode.h"
#include "filedirindexnode.h"

class FileDirLeafNode : public FileDirNode
{
public:

	FileDirLeafNode(int id = 0, int padre = 0, int HnoIzquierdo = -1, int HnoDerecho = -1);

	void read (char* buffer);
	void write(char* buffer);

	int search(const char* key);
	int searchFileAndVersion(const char* fileName, int version);
   int getFirstOf(const char* key);

	FileDirNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre);
	void promoteRoot(VarLenNode** nodo1, VarLenNode** nodo2, int id1, int id2,int clavesArreglo, char** arreglo, int bytesArreglo, 
   char** claveARaiz);

    // getters
	int getHnoDerecho()   const { return _hnoDerecho; }
	int getHnoIzquierdo() const { return _hnoIzquierdo; }
   t_nodeType getType() const { return LEAF; }
   
    // setters
	void setHnoDerecho  (int HnoDerecho) { _hnoDerecho = HnoDerecho; }
	void setHnoIzquierdo(int HnoIzquierdo) { _hnoIzquierdo = HnoIzquierdo; }

	// lista las claves y referencias del nodo
	void list();

private:
	int _hnoDerecho;
	int _hnoIzquierdo;
};

#endif
