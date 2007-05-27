#ifndef NODOBMASINDICE_H_INCLUDED
#define NODOBMASINDICE_H_INCLUDED

#include "nodobmas.h"

class NodoBMasIndice : public NodoBMas
{
public:
    static const int BYTESREDONDEO;

    // constructors
    NodoBMasIndice(int Numero = 0, int Nivel = 0, int Padre = -1, int HijoIzquierdo = 0, char* key = 0, int ref = 0);
	//~NodoBMasIndice();

    void read(char* buffer);
    void write(char* buffer);

    int searchFile(const char* key);
	int searchFileAndVersion(const char* fileName,int version);

    NodoBMas* split(int Numero, char* arreglo,int bytesArreglo,int clavesArreglo,
							char** claveAlPadre);
    void promoteRoot(NodoBMas** nodo1, NodoBMas** nodo2, int id1, int id2,
					int clavesArreglo,char** arreglo,int bytesArreglo,char** claveARaiz);

    // getters
    int getHijoIzquierdo() const { return _hijoIzquierdo; }
    int getType() const { return NODOINDICE; }

protected:
    int   _hijoIzquierdo;            // la referencia al nodo con claves menores
    char* _redondeo; // bytes de redondeo para completar el tamanio del nodo en disco

};

#endif

