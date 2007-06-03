#ifndef ARBOLBMAS_H_INCLUDED
#define ARBOLBMAS_H_INCLUDED

#include <fstream>
#include <string>

#include "nodobmas.h"
#include "nodobmashoja.h"
#include "nodobmasindice.h"

using std::string;

class ArbolBMas
{
public:
    ArbolBMas();    // constructor
    ~ArbolBMas();   // destructor
    
    bool create(const string& a_Filename);
    bool destroy();

    bool open(const string& a_Filename);
    bool close();

    int searchFile(const char* key);
    int searchFileAndVersion(const char* fileName,int version);
    bool insert(const char* key,int reference);
    void list();

protected:
    // el tamanio de la cabecera es el tamanio de un nodo
    bool readHeader();
    bool writeHeader();
    bool readRoot();
    bool writeRoot();
    bool readNode(int id, NodoBMas** node);
    bool writeNode(NodoBMas* nodo);

    // devuelve el nro de nodo hoja donde debemos insertar key
    int searchPlace(const char* key);

    // metodo auxiliar para poder insertar
    int searchPlaceRec(const char* key);
    
    // metodo recursivo para hacer crecer el arbol en caso de overflow
    void insertarEnPadre(int NroNodoPadre, int NroNodoHijo, char* claveAlPadre);

    // metodo que actualiza el puntero al padre de los nodos apuntados por padre
    bool actualizarPadre(NodoBMasIndice* padre);
   
private: 
    // member variables
    NodoBMas*    _raiz;         // nodo raiz del arbol
    NodoBMas*    _nodoActual;   // nodo actual del arbol
    int          _nNodos;       // cantidad de nodos del archivo
    char*        _buffer;
    std::fstream _filestr;      // file stream para poder ir leyendo de archivo los nodos
    string       _filename;     // filename storing the tree    
};

#endif
