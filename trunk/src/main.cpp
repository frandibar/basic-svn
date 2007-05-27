#include "nodobmas.h"
#include "nodobmashoja.h"
#include "tests.h"


void testHojasInsercionSplitSearch()
{
	NodoBMasHoja* hoja = new NodoBMasHoja();

	hoja->insert("Pia",0);

	hoja->insert("Paz",1);

	hoja->insert("Ana",2);

	int res = hoja->searchFile("Pia");

	res = hoja->searchFile("Paz");

	res = hoja->searchFile("Ana");

	char* arreglo;
	int bytesArreglo = 0;
	int clavesArreglo = 0;
	char* claveAlPadre;

	hoja->insert("Juan",3,&clavesArreglo,&arreglo,&bytesArreglo);

	if(bytesArreglo)
	{
		NodoBMas* hojaNueva = hoja->split(1,arreglo,bytesArreglo,clavesArreglo,&claveAlPadre);

		delete arreglo;

		res = hoja->searchFile("Ana");

		res = hoja->searchFile("Juan");

		res = hojaNueva->searchFile("Paz");

		res = hojaNueva->searchFile("Pia");

		delete hojaNueva;
	}

	delete hoja;

	return;
}
void main(char** argv,int argc)
{
	//testArbol();

	//testBusqueda();

	testBusquedaVersion();

	//testList();

	//testVersion();

	//testBloque();
}