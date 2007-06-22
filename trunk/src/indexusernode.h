// indexusernode.h

#ifndef INDEXUSERNODE_H_INCLUDED
#define INDEXUSERNODE_H_INCLUDED

#include "usernode.h"

class IndexUserNode : public UserNode
{
public:
    static const int BYTESREDONDEO;

    // constructors
    IndexUserNode(int Numero = 0, int Nivel = 0, int Padre = -1, int HijoIzquierdo = 0, char* key = 0, int ref = 0);
	//~NodoBMasIndice();

    void read(char* buffer);
    void write(char* buffer);

    int search(const char* key);

    UserNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre);
    void promoteRoot(UserNode** nodo1, UserNode** nodo2, int id1, int id2,
					int clavesArreglo, char** arreglo, int bytesArreglo, char** claveARaiz);

    // getters
    int getHijoIzquierdo() const { return _hijoIzquierdo; }
    t_nodeType getType() const { return INDEX; }

protected:
    int   _hijoIzquierdo;            // la referencia al nodo con claves menores
    char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco

};

#endif
