//fixlenindexnode.h
#ifndef FIXLENINDEXNODE_H_INCLUDED
#define FIXLENINDEXNODE_H_INCLUDED

#include "fixlennode.h"

class FixLenIndexNode : public FixLenNode
{
public:
    static const int BYTESREDONDEO;

    // constructors
    FixLenIndexNode(int id = 0, int nivel = 0, int padre = -1, int HijoIzquierdo = 0, const char* key = 0, int ref = 0);

    void read(char* buffer);
    void write(char* buffer);

    int search(const char* key);

    void promoteRoot(FixLenNode** nuevo_nodo, int id);

    // getters
    int getHijoIzquierdo() const { return _hijoIzquierdo; }
    t_nodeType getType() const { return INDEX; }

protected:
    int   _hijoIzquierdo;            // la referencia al nodo con claves menores
    char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco

};

#endif

