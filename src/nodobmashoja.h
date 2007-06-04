// nodobmashoja.h

#ifndef NODOBMASHOJA_H_INCLUDED
#define NODOBMASHOJA_H_INCLUDED

#include "nodobmas.h"
#include "nodobmasindice.h"

class NodoBMasHoja : public NodoBMas
{
public:

	NodoBMasHoja(int Numero = 0, int Padre = 0, int HnoIzquierdo = -1, int HnoDerecho = -1);

	void read (char* buffer);
	void write(char* buffer);

	int searchFile(const char* key);
	int searchFileAndVersion(const char* fileName, int version);

	NodoBMas* split(int Numero, char* arreglo, int bytesArreglo, int clavesArreglo, char** claveAlPadre);
	void promoteRoot(NodoBMas** nodo1, NodoBMas** nodo2, int id1, int id2,
					int clavesArreglo, char** arreglo, int bytesArreglo,
					char** claveARaiz);

    // getters
	int getHnoDerecho()   const { return _hnoDerecho; }
	int getHnoIzquierdo() const { return _hnoIzquierdo; }
    int getType() const { return NODOHOJA; }
   
    // setters
	void setHnoDerecho  (int HnoDerecho) { _hnoDerecho = HnoDerecho; }
	void setHnoIzquierdo(int HnoIzquierdo) { _hnoIzquierdo = HnoIzquierdo; }

	// lista las claves y referencias del nodo
	void list();

private:
	int _hnoDerecho;
	int _hnoIzquierdo;
};

#endif
