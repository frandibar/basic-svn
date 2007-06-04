#ifndef NODOBMAS_H_INCLUDED
#define NODOBMAS_H_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

#define TAMANIOARREGLO 2022   // TAMANIONODO - bytes admin bloque (_id + _nivel + _padre + _nclaves + _espacioLibre)

class NodoBMas 
{
public:
//    static const int TAMANIOARREGLO;
    static const int TAMANIONODO;
    static const int NODOHOJA;
    static const int NODOINDICE;

    // constructors
    NodoBMas(int Numero = 0, int Nivel = 0, int Padre = -1);
    virtual ~NodoBMas() {};

    virtual void read (char* buffer) = 0;  // lee el nodo desde un buffer
    virtual void write(char* buffer) = 0;  // escribe el nodo en un buffer
    
    // busca la clave dentro del nodo, devuelve: 
    // - una referencia al archivo de datos si la clave buscada esta en el nodo y este es un 
    //   nodo hoja.
    // - -1 si la clave buscada no esta en el nodo y es un nodo hoja.
    // - la referencia al nodo siguiente donde buscar si el nodo es indice
    virtual int searchFile(const char* key) = 0; // key = nombre del archivo a buscar
    virtual int searchFileAndVersion(const char* fileName,int version) = 0;
    
                                                                        
    // metodo virtual para insertar una clave en un nodo
    int insert(const char* key, int ref, int* clavesArreglo, char** arreglo,int* bytesArreglo);
    int insert(const char* key, int ref);

    int isFull() const { return _espacioLibre == 0; }

    // getters
    int getId()         const { return _id;      }
    int getPadre()      const { return _padre;   }
    int getCantClaves() const { return _nclaves; }
    int getOffset()     const { return _offset;  }
    int getNivel()      const { return _nivel;   }
    int getRef(int idNode);
    // devuelve 1 si el nodo es interior y 2 si es hoja
    virtual int getType() const = 0;

    // setters
    void setPadre(int Padre) { _padre = Padre; }

    // generador de un nodo del mismo _nivel para cuando tengo que hacer un split
    virtual NodoBMas* split(int Numero, char* arreglo,int bytesArreglo,int clavesArreglo,
                            char** claveAlPadre) = 0;

    // me genera 2 nuevos nodos al tener que promover la raiz
    virtual void promoteRoot(NodoBMas** nodo1, NodoBMas** nodo2,int id1,int id2,
                            int clavesArreglo, char** arreglo, int bytesArreglo,
                            char** claveARaiz)  = 0;

protected:
    int insertPair(const char* key, int ref);
    int insertPair( const char* key, int ref, int* offset,
                    int* clavesArregloAux,char** arregloAux);
    int insertPair(const char* key,int ref,int* clavesArreglo,char** arregloAux,int* bystesArreglo);
    void completePairs(int* offset,int tamanioArreglo,char* arregloAux);
    void completePairs(int* tamanioArreglo,char** arregloAux,int* bytesArreglo);            

    void readInfoAdm (char** nextByte);
    void readDatos   (char** nextByte);
    void writeInfoAdm(char** nextByte);
    void writeDatos  (char** nextByte);

    // member variables
    int       _id;               // id del nodo
    int       _nivel;            // nivel del nodo
    int       _padre;            // referencia al padre del nodo
    short int _nclaves;          // cantidad de claves que contiene el nodo
    int       _espacioLibre;     // cantidad de espacio libre dentro del nodo
    int       _offset;           // el offset de donde voy a empezar a insertar, esta variable se calcula
    char _pares[TAMANIOARREGLO]; // pares clave-referencia en el caso de los nodos hoja
                                 // pares clave-hijo claves mayores en el caso de los interiores

};                       
    
#endif
