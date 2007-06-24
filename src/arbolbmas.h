// arbolbmas.h

#ifndef ARBOLBMAS_H_INCLUDED
#define ARBOLBMAS_H_INCLUDED

#include "filedirnode.h"
#include "filedirleafnode.h"
#include "filedirindexnode.h"

#include <fstream>
#include <string>

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
    int getFirstBlock(const char* key);
    bool insert(const char* key,int reference);
    void list();

protected:
    // el tamanio de la cabecera es el tamanio de un nodo
    bool readHeader();
    bool writeHeader();

    bool readRoot();
    bool writeRoot();
    
    bool readNode(int id, FileDirNode** node);
    bool writeNode(FileDirNode* nodo);

    // devuelve el nro de nodo hoja donde debemos insertar key
    int searchPlace(const char* key);

    // metodo auxiliar para poder insertar
    int searchPlaceRec(const char* key);
    
    // metodo recursivo para hacer crecer el arbol en caso de overflow
    void insertarEnPadre(int NroNodoPadre, int NroNodoHijo, char* claveAlPadre);

    // metodo que actualiza el puntero al padre de los nodos apuntados por padre
    bool actualizarPadre(FileDirIndexNode* padre);

    bool isEmpty() const { return _raiz == 0; }
   
private: 
    // member variables
    FileDirNode*    _raiz;         // nodo raiz del arbol
    FileDirNode*    _nodoActual;   // nodo actual del arbol
    int          _nNodos;       // cantidad de nodos del archivo
    char*        _buffer;
    std::fstream _filestr;      // file stream para poder ir leyendo de archivo los nodos
    string       _filename;     // filename storing the tree    
    bool         _isOpen;
};

#endif
