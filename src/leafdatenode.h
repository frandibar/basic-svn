#ifndef LEAFDATENODE_H_INCLUDED
#define LEAFDATENODE_H_INCLUDED

#include "datenode.h"
#include "indexdatenode.h"

class LeafDateNode : public DateNode
{
public:

	LeafDateNode(int Numero = 0, int Padre = 0, int HnoIzquierdo = -1, int HnoDerecho = -1);

	void read (char* buffer);
	void write(char* buffer);

	int search(const char* key);

	void promoteRoot(DateNode** nuevo_nodo,int id);
	IndexDateNode* convertirAIndice(int HijoIzquierdo, int primerRef, const char* key);

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
