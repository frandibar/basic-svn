// Bloque.h

#ifndef USERBLOCK_H_INCLUDED
#define USERBLOCK_H_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

#define TAMANIO_ARREGLO_BLOQUE_USUARIO 236

class UserBlock
{
public:
    static const int TAMANIO_BLOQUE_USUARIO;

    UserBlock(int Numero = -1,int Anterior = -1 ,int Siguiente = -1);
    ~UserBlock();

    bool insertRef(int ref);
    void write(char* buffer);
    void read(char* buffer);
   
    void setSiguiente(int Siguiente){   _siguiente = Siguiente; }
    void setAnterior(int Anterior)  {   _anterior = Anterior;   }

    bool hayLugar();

    void moveFirst();
    int  getNext();
    bool hasNext();
    bool moveTo(int refNumber);
    
    // getters
    int getSiguiente() const        { return _siguiente;     }
    int getAnterior() const         { return _anterior;      }
    int getNumero() const           { return _numero;        }
    int getCantidadReferencias()const { return _cantReferencias; }
    
private:
    int _siguiente;
    int _anterior;
    int _espacioLibre;
    int _cantReferencias;
    int _used;
    int _actualOffset;
    int _numero;

    char _referencias[TAMANIO_ARREGLO_BLOQUE_USUARIO];
};

#endif


