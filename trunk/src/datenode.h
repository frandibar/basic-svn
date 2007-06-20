// datenode.h

#ifndef DATENODE_H_INCLUDED
#define DATENODE_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <string>

#define STREAMSIZE 490   // NODE_SIZE - bytes admin bloque (_id + _nivel + _padre + _nclaves + _espacioLibre)
#define LONGCLAVE	10
class DateNode 
{
public:
    static const int NODE_SIZE;
	 static const int MAXCLAVES;

    enum t_status   { ALREADY_EXISTS = 0, OK, OVERFLOW, ERROR };
    enum t_nodeType { LEAF, INDEX };

    // constructors
    DateNode(int id = 0, int nivel = 0, int padre = -1);
    virtual ~DateNode() {};

    virtual void read (char* buffer) = 0;  // lee el nodo desde un buffer
    virtual void write(char* buffer) = 0;  // escribe el nodo en un buffer
    
    // busca la clave dentro del nodo, devuelve: 
    // - una referencia al archivo de datos si la clave buscada esta en el nodo y este es un 
    //   nodo hoja.
    // - -1 si la clave buscada no esta en el nodo y es un nodo hoja.
    // - la referencia al nodo siguiente donde buscar si el nodo es indice
    virtual int search(const char* key) = 0;
    
                                                                        
    // metodo virtual para insertar una clave en un nodo
    int insert(const char* key, int ref);

    int isFull() const { return _nclaves == MAXCLAVES; }

    // getters
    int getId()         const { return _id;      }
    int getPadre()      const { return _padre;   }
    int getCantClaves() const { return _nclaves; }
    int getRef(int idNode);
	int getNivel()		const { return _nivel;	 }

    // devuelve 1 si el nodo es interior y 2 si es hoja
    virtual t_nodeType getType() const = 0;

    // setters
    void setPadre(int Padre) { _padre = Padre; }

    // me genera 2 nuevos nodos al tener que promover la raiz
    virtual void promoteRoot(DateNode** nuevo_nodo,int id)  = 0;

protected:
    int insertPair(const char* key, int ref);
    int insertPair(const char* key, int ref, int* i, char* auxKey, int* auxRef);
    void completePairs(int* i, int* j, char* auxKey, int* auxRef);

    void readInfoAdm (char** nextByte);
    void readDatos   (char** nextByte);

    void writeInfoAdm(char** nextByte);
    void writeDatos  (char** nextByte);

    // member variables
    int       _id;      // id del nodo
    int       _nivel;   // nivel del nodo
    int       _padre;   // referencia al padre del nodo
    short int _nclaves; // cantidad de claves que contiene el nodo
    char	     _pares[STREAMSIZE];   // pares clave-referencia en el caso de los nodos hoja
                        // pares clave-hijo claves mayores en el caso de los interiores

};                       
    
#endif
