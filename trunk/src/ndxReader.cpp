
#include "nodobmas.h"

#include <iostream>
#include <fstream>
#include <string>

#define SIZE  2048
#define SIZE2 2016

using namespace std;

int main(int argc, char** argv)
{
    // usage: ./ndxReader filename
    if (argc != 2)
        return -1;

    ifstream is(argv[1]);
    if (!is.is_open()) return false;

    char buffer[SIZE];
    is.read(buffer, SIZE);
    int nNodos;
    memcpy(&nNodos, buffer, sizeof(int));
    cout << "#nodos  : (" << sizeof(int) << "b 0)\t" << nNodos << endl;
    cout << "-------------------------------------" << endl;

    for (int i = 0; i < nNodos && !is.eof(); ++i) {

        int pad = SIZE * (i + 1);

        is.read(buffer, SIZE);
        char* offset = buffer;

        int nivel;
        memcpy(&nivel, offset, sizeof(int));
        offset += sizeof(int);
        cout << "nivel   : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << nivel << endl;

        int id;
        memcpy(&id, offset, sizeof(int));
        offset += sizeof(int);
        cout << "id      : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << id << endl;

        int padre;
        memcpy(&padre, offset, sizeof(int));
        offset += sizeof(int);
        cout << "padre   : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << padre << endl;

        int espacioLibre;
        memcpy(&espacioLibre, offset, sizeof(int));
        offset += sizeof(int);
        cout << "e libre : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << espacioLibre << endl;

        int nclaves;
        memcpy(&nclaves, offset, sizeof(int));
        offset += sizeof(int);
        cout << "#claves : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << nclaves << endl;

        int keylength;
        memcpy(&keylength, offset, sizeof(int));
        offset += sizeof(int);
        cout << "keylen  : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << keylength << endl;

        char data[SIZE2];
        memcpy(data, offset, SIZE2 * sizeof(char));
        offset += SIZE2 * sizeof(char);
        cout << "data    : (" << SIZE2 * sizeof(char) << "b " << offset - buffer + pad << ")\t" << data << endl;

        if (nivel == 0) {   // nodo hoja
            int izq;
            memcpy(&izq, offset, sizeof(int));
            offset += sizeof(int);
            cout << "hno izq : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << izq << endl;

            int der;
            memcpy(&der, offset, sizeof(int));
            offset += sizeof(int);
            cout << "hno der : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << der << endl;
        } 
        else {  // nodo indice
            int izq;
            memcpy(&izq, offset, sizeof(int));
            offset += sizeof(int);
            cout << "hijo izq: (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << izq << endl;
        }

        cout << "-------------------------------------" << endl;
    }

    return 0;
}
