#ifndef NODOBMASINDICE
#define NODOBMASINDICE

#define BYTESREDONDEO 2

#include "nodobmas.h"

class NodoBMasIndice:public NodoBMas
{
protected:
	int hijoIzquierdo; // la referencia al nodo con claves menores
	char redondeo[BYTESREDONDEO]; // bytes de redondeo para completar el tamaño del nodo en disco

public:
	NodoBMasIndice(int Numero, int Nivel, int Padre,int HijoIzquierdo,
				char key[LONGCLAVE],int ref);

	NodoBMasIndice(int Numero,int Nivel, int Padre,int HijoIzquierdo);

	NodoBMasIndice();

	~NodoBMasIndice();

	void read(char* buffer);
	void write(char* buffer);

	int search(char key[LONGCLAVE]);

	int type();

	NodoBMas* split(int Numero,char claveMenor[LONGCLAVE]);

	void promoteRoot(NodoBMas** nodo1, NodoBMas** nodo2,int id1,int id2);

	int getHijoIzquierdo();
};

#endif