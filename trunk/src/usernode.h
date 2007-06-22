// usernode.h

#ifndef USERNODE_H_INCLUDED
#define USERNODE_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <string>

#define TAMARREGLO 100 // NODE_SIZE - bytes admin bloque (_id + _nivel + _padre + _nclaves + _espacioLibre)

class UserNode 
{
public:
    static const int NODE_SIZE;

    enum t_status   { ALREADY_EXISTS = 0, OK, OVERFLOW, ERROR };
    enum t_nodeType { LEAF, INDEX };

    // constructors
    UserNode(int id = 0, int nivel = 0, int padre = -1);
    virtual ~UserNode() {};

    virtual void read (char* buffer) = 0;  // lee el nodo desde un buffer
    virtual void write(char* buffer) = 0;  // escribe el nodo en un buffer
    
    // busca la clave dentro del nodo, devuelve: 
    // - una referencia al archivo de datos si la clave buscada esta en el nodo y este es un nodo hoja.
    // - -1 si la clave buscada no esta en el nodo y es un nodo hoja.
    // - la referencia al nodo siguiente donde buscar si el nodo es indice
    virtual int search(const char* key) = 0; // key = nombre del archivo a buscar

    t_status insert(const char* key, int ref, int* clavesArreglo, char** arreglo, int* bytesArreglo);
    bool insert(const char* key, int ref) {
        return insertPair(key, ref);
    }

    bool isFull() const { return _espacioLibre == 0; }

    // getters
    int getId()         const { return _id;      }
    int getPadre()      const { return _padre;   }
    int getCantClaves() const { return _nclaves; }
    int getOffset()     const { return _offset;  }
    int getNivel()      const { return _nivel;   }
    int getRef(int idNode);
   
    virtual t_nodeType getType() const = 0;

    // setters
    void setPadre(int a_Padre) { _padre = a_Padre; }

    // generador de un nodo del mismo _nivel para cuando tengo que hacer un split
    virtual UserNode* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo,
                            char** claveAlPadre) = 0;

    // me genera 2 nuevos nodos al tener que promover la raiz
    virtual void promoteRoot(UserNode** nodo1, UserNode** nodo2, int id1, int id2,
                            int clavesArreglo, char** arreglo, int bytesArreglo, char** claveARaiz)  = 0;

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

    // member variables
    int  _nivel;
    int  _id;
    int  _padre;         // referencia al padre del nodo
    int  _espacioLibre;
    int  _offset;        // el offset de donde voy a empezar a insertar, esta variable se calcula
    int  _nclaves;
    char _pares[TAMARREGLO]; // pares clave-referencia en el caso de los nodos hoja
                             // pares clave-hijo claves mayores en el caso de los interiores

};                       
    
#endif
