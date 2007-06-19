// Bloque.h

#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include "FileVersion.h"

#include <cstdlib>
#include <string>
#include <iostream>

#define TAMANIO_ARREGLO_BLOQUE_ARCHIVOS 1004

class FileBlock
{
public:
    static const int TAMANIO_BLOQUE_ARCHIVOS;

    FileBlock(int Numero = -1,int Anterior = -1 ,int Siguiente = -1);
    ~FileBlock();

    bool insertVersion(FileVersion* version);
    bool searchVersion(int nro,FileVersion** version);
    bool searchVersion(int nro);
    void write(char* buffer);
    void read(char* buffer);
    FileVersion* getLastVersion();

    int getFirstVersionNumber();

    void setSiguiente(int Siguiente){   _siguiente = Siguiente; }
    void setAnterior(int Anterior)  {   _anterior = Anterior;   }

    bool hayLugar(FileVersion* version);

    void moveFirst();
    FileVersion* getNext();
    bool hasNext();
    
    // getters
    int getSiguiente() const        { return _siguiente;     }
    int getAnterior() const         { return _anterior;      }
    int getNumero() const           { return _numero;        }
    int getCantidadVersiones()const { return _cantVersiones; }
    
    bool getHistory(std::ifstream& is);

private:
    int _siguiente;
    int _anterior;
    int _espacioLibre;
    int _cantVersiones;
    int _used;
    int _actualOffset;
    int _numero;

    char _versiones[TAMANIO_ARREGLO_BLOQUE_ARCHIVOS];
};

#endif


