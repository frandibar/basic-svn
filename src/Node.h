#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <string>

class Node
{
public:
    enum t_status   { ALREADY_EXISTS = 0, OK, OVERFLOW, ERROR };
    enum t_nodeType { LEAF, INDEX };

    // constructors
    Node(int id = 0, int nivel = 0, int padre = -1);
    virtual ~Node() {};

    virtual void read (char* buffer) = 0;  // lee el nodo desde un buffer
    virtual void write(char* buffer) = 0;  // escribe el nodo en un buffer
    
    // busca la clave dentro del nodo, devuelve: 
    // - una referencia al archivo de datos si la clave buscada esta en el nodo y este es un nodo hoja.
    // - -1 si la clave buscada no esta en el nodo y es un nodo hoja.
    // - la referencia al nodo siguiente donde buscar si el nodo es indice
    virtual int search(const char* key) = 0; // key = nombre del archivo a buscar

    virtual bool isFull() const = 0;

    // getters
    int getId()         const { return _id;      }
    int getPadre()      const { return _padre;   }
    int getCantClaves() const { return _nclaves; }
    int getNivel()      const { return _nivel;   }
    virtual int getRef(int idNode) = 0;
   
    virtual t_nodeType getType() const = 0;

    // setters
    void setPadre(int a_Padre) { _padre = a_Padre; }

protected:
    virtual bool insertPair(const char* key, int ref) = 0;

    virtual void readInfoAdm (char** nextByte) = 0;
    virtual void writeInfoAdm(char** nextByte) = 0;

    virtual void readDatos   (char** nextByte) = 0;
    virtual void writeDatos  (char** nextByte) = 0;

    // member variables
    int  _nivel;
    int  _id;
    int  _padre;         // referencia al padre del nodo
    int  _nclaves;
};                       
    
#endif
