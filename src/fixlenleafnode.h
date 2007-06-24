//fixlenleafnode.h

#ifndef FIXLENLEAFNODE_H_INCLUDED
#define FIXLENLEAFNODE_H_INCLUDED

#include "fixlennode.h"
#include "fixlenindexnode.h"

class FixLenLeafNode : public FixLenNode
{
public:

	FixLenLeafNode(int id = 0, int padre = 0, int HnoIzquierdo = -1, int HnoDerecho = -1);

	void read (char* buffer);
	void write(char* buffer);

	int search(const char* key);

	void promoteRoot(FixLenNode** nuevo_nodo,int id);
	FixLenIndexNode* convertirAIndice(int HijoIzquierdo, int primerRef, const char* key);

   // getters
	int getHnoDerecho()   const { return _hnoDerecho; }
	int getHnoIzquierdo() const { return _hnoIzquierdo; }
   t_nodeType getType() const { return LEAF; }
   
   // setters
	void setHnoDerecho  (int HnoDerecho) { _hnoDerecho = HnoDerecho; }
	void setHnoIzquierdo(int HnoIzquierdo) { _hnoIzquierdo = HnoIzquierdo; }

protected:
	int _hnoDerecho;
	int _hnoIzquierdo;
};

#endif
