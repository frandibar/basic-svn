#ifndef INDEXDATENODE_H_INCLUDED
#define INDEXDATENODE_H_INCLUDED

#include "datenode.h"

class IndexDateNode : public DateNode
{
public:
    static const int BYTESREDONDEO;

    // constructors
    IndexDateNode(int Numero = 0, int Nivel = 0, int Padre = -1, int HijoIzquierdo = 0, const char* key = 0, int ref = 0);

    void read(char* buffer);
    void write(char* buffer);

    int search(const char* key);

    void promoteRoot(DateNode** nuevo_nodo, int id);

    // getters
    int getHijoIzquierdo() const { return _hijoIzquierdo; }
    t_nodeType getType() const { return INDEX; }

protected:
    int   _hijoIzquierdo;            // la referencia al nodo con claves menores
    char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco

};

#endif

