
#include "filedirleafnode.h"

using std::cout;
using std::endl;

    FileDirLeafNode::FileDirLeafNode(int id, int padre, int HnoIzquierdo, int HnoDerecho)
: FileDirNode(id, 0, padre)
{
    _hnoIzquierdo = HnoIzquierdo;
    _hnoDerecho   = HnoDerecho;
}

void FileDirLeafNode::read(char* buffer)
{
    char* nextByte = buffer;
    readInfoAdm(&nextByte);
    readDatos(&nextByte);
    memcpy(&_hnoIzquierdo, nextByte, sizeof(int));
    nextByte += sizeof(int);
    memcpy(&_hnoDerecho, nextByte, sizeof(int));
}

void FileDirLeafNode::write(char* buffer)
{
    char* nextByte = buffer;
    writeInfoAdm(&nextByte);
    writeDatos(&nextByte);
    memcpy(nextByte, &_hnoIzquierdo, sizeof(int));
    nextByte += sizeof(int);
    memcpy(nextByte, &_hnoDerecho, sizeof(int));
}

int FileDirLeafNode::search(const char* key)
{
    char* auxKey;
    char* auxFileName;
    int longClave;
    int auxRef;
    int i = 0;
    int offsetLectura = 0;
    int ret = -1;

    while (i < _nclaves) {     
        memcpy(&longClave,_pares + offsetLectura,sizeof(int));
        offsetLectura += sizeof(int);

        auxKey = new char[(longClave + 1) * sizeof(char)];
        memcpy(auxKey, _pares + offsetLectura, sizeof(char) * longClave);
        offsetLectura += sizeof(char) * longClave;
        auxKey[longClave] = 0; // coloco la marca de fin

        // genero el nombre del archivo
        auxFileName = new char[(longClave - 4) * sizeof(char)];
        memcpy(auxFileName,auxKey,(longClave - 5));
        auxFileName[longClave - 5] = 0;

        memcpy(&auxRef, _pares + offsetLectura, sizeof(int));
        offsetLectura += sizeof(int);

        if (strcmp(key, auxFileName) == 0){
            i++;
            ret = auxRef;
            delete auxKey;
            delete auxFileName;
        }
        else if(strcmp(key,auxFileName) > 0)
        {
            i++;
            delete auxKey;
            delete auxFileName;
        }        
        else{
            delete auxFileName;
            delete(auxKey);
            return ret;
        }
    }
    return ret;
}

int FileDirLeafNode::searchFileAndVersion(const char* fileName,int version)
{
    char* auxKey;
    char* auxFileName;
    char* versionStr;
    int auxVersion;
    int longClave;
    int auxRef;
    int i = 0;
    int offsetLectura = 0;
    int ret = -1;

    while (i < _nclaves) {
        memcpy(&longClave,_pares + offsetLectura,sizeof(int));
        offsetLectura += sizeof(int);

        auxKey = new char[(longClave + 1) * sizeof(char)];
        memcpy(auxKey, _pares + offsetLectura, sizeof(char) * longClave);
        offsetLectura += sizeof(char) * longClave;
        auxKey[longClave] = 0; // coloco la marca de fin

        // genero el nombre del archivo
        auxFileName = new char[(longClave - 4) * sizeof(char)];
        memcpy(auxFileName,auxKey,(longClave - 5));
        auxFileName[longClave - 5] = 0;

        //genero la clave string
        versionStr = new char[6];
        memcpy(versionStr, auxKey + (longClave - 5),5*sizeof(char));
        versionStr[5] = 0;

        auxVersion = atoi(versionStr);
        delete(versionStr);

        memcpy(&auxRef, _pares + offsetLectura, sizeof(int));
        offsetLectura += sizeof(int);

        if (strcmp(fileName, auxFileName) == 0){
            i++;
            delete auxKey;
            delete auxFileName;

            if(version >= auxVersion)
                ret = auxRef;			
            else
                return ret;
        }
        else if(strcmp(fileName,auxFileName) > 0)
        {
            i++;
            delete auxKey;
            delete auxFileName;
        }       
        else{
            delete auxFileName;
            delete(auxKey);
            return ret;
        }
    }
    return ret;
}


FileDirNode* FileDirLeafNode::split(int Numero, char* arreglo,int bytesArreglo,int clavesArreglo, char** claveAlPadre)
{
    int longClave = 0;
    int offsetArreglo = 0;
    FileDirLeafNode* ret = new FileDirLeafNode(Numero, _padre, _id, _hnoDerecho);    
    int auxRef;
    char* auxKey;
    int clavesCopiadas = 0;

    // si tengo 1 sola clave en el arreglo trato de poder sacar alguna del nodo actual
    // para poder insertarla en el nuevo nodo
    if(( clavesArreglo == 1)&&(_espacioLibre < VARLEN_STREAM_SIZE /2)){
        int clavesLeidas = 0;

        while(offsetArreglo < VARLEN_STREAM_SIZE /2){
            // tamanio de la clave
            memcpy(&longClave,_pares + offsetArreglo,sizeof(int));
            offsetArreglo += sizeof(int);
            // avanzo la cantidad de caracteres de la clave
            offsetArreglo += sizeof(char) * longClave; 
            // avanzo la cantidad de bytes que ocupa la referencia
            offsetArreglo += sizeof(int);

            clavesLeidas++;
        }

        // nuevo offset en el arreglo del nodo actual
        int nuevoOffset = offsetArreglo;	
        // copio a partir de ahora todas las claves que restan al nuevo nodo
        for(int i = clavesLeidas; i < _nclaves;i++){
            memcpy(&longClave,_pares + offsetArreglo,sizeof(int));
            offsetArreglo += sizeof(int);

            auxKey = new char[(longClave +1) * sizeof(char)];
            memcpy(auxKey,_pares + offsetArreglo,sizeof(char)*longClave);
            auxKey[longClave] = 0;
            offsetArreglo += sizeof(char)*longClave;

            memcpy(&auxRef,_pares + offsetArreglo, sizeof(int));
            offsetArreglo += sizeof(int);

            if(!clavesCopiadas)
            {
                *claveAlPadre = new char[(longClave + 1) * sizeof(char)];
                strcpy(*claveAlPadre,auxKey);
            }

            ret->insert(auxKey,auxRef);

            delete (auxKey);

            clavesCopiadas++;
        }

        _offset = nuevoOffset;
        _nclaves = clavesLeidas;
        _espacioLibre = VARLEN_STREAM_SIZE - _offset;
    }

    offsetArreglo = 0;
    // sino la 1º clave del nuevo nodo va a ser la que viene desde el arreglo
    if(!clavesCopiadas)
    {
        // tomo la clave menor para subir al _padre
        memcpy(&longClave,arreglo + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);

        *claveAlPadre = new char[(longClave +1) * sizeof(char)];
        memcpy((*claveAlPadre), arreglo + offsetArreglo, sizeof(char) * longClave);
        (*claveAlPadre)[longClave] = 0; // coloco la marca de fin de cadena
    }

    //vuelvo a posicionarme en el principio del arreglo para pasar todas las claves
    offsetArreglo = 0;

    // copio las _nclaves de la mitad en adelante en el nuevo nodo
    for(int j = 0; j < clavesArreglo; ++j) {
        // leo la longitud
        memcpy(&longClave,arreglo + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);		

        // inicializo la clave y copio su contenido
        auxKey = new char[(longClave + 1) * sizeof(char)];
        memcpy(auxKey, arreglo + offsetArreglo, sizeof(char) * longClave);
        offsetArreglo += longClave * sizeof(char);
        auxKey[longClave] = 0; // le coloco la marca de fin de cadena

        memcpy(&auxRef, arreglo + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);

        ret->insert(auxKey, auxRef);

        delete(auxKey);
    }

    // seteo el nuevo hno derecho para el nodo actual
    _hnoDerecho = ret->getId();

    return ret;
}

void FileDirLeafNode::promoteRoot(VarLenNode** nodo1,VarLenNode** nodo2, int id1, int id2,int clavesArreglo,char** arreglo, 
        int bytesArreglo, char** claveARaiz)
{
    // creo los dos nodos que van a ser los hijos de la raiz
    *nodo1 = new FileDirLeafNode(id1, _id, -1, id2);

    *nodo2 = new FileDirLeafNode(id2,_id,id1,-1);

    int offsetArreglo = 0;
    int tamanioClave;
    int auxRef;
    char* auxKey;
    int clavesLeidas = 0;

    // copio la mitad de las claves que quedaron en la hoja al nuevo nodo derecho "nodo1"
    while(offsetArreglo < VARLEN_STREAM_SIZE / 2){
        //leo el tamaño de la 1º clave
        memcpy(&tamanioClave,_pares + offsetArreglo,sizeof(int));
        offsetArreglo += sizeof(int);

        auxKey = new char[(tamanioClave + 1) * sizeof(char)];
        memcpy(auxKey, _pares + offsetArreglo, sizeof(char) * tamanioClave);
        auxKey[tamanioClave] = 0;
        offsetArreglo += sizeof(char) * tamanioClave;

        memcpy(&auxRef, _pares + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);

        (*nodo1)->insert(auxKey, auxRef);

        delete(auxKey);

        clavesLeidas ++;
    }

    if(clavesLeidas < _nclaves){
        memcpy(&tamanioClave,_pares + offsetArreglo,sizeof(int));
        offsetArreglo += sizeof(int);

        *claveARaiz = new char[(tamanioClave + 1) * sizeof(char)];
        memcpy(*claveARaiz, _pares + offsetArreglo, sizeof(char) * tamanioClave);
        (*claveARaiz)[tamanioClave] = 0;
        offsetArreglo += sizeof(char) * tamanioClave;

        memcpy(&auxRef, _pares + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);

        (*nodo2)->insert(*claveARaiz, auxRef);

        clavesLeidas ++;

        for(int i = clavesLeidas;i < _nclaves;i++){			
            memcpy(&tamanioClave,_pares + offsetArreglo,sizeof(int));
            offsetArreglo += sizeof(int);

            auxKey = new char[(tamanioClave + 1) * sizeof(char)];

            memcpy(auxKey, _pares + offsetArreglo, sizeof(char) * tamanioClave);
            auxKey[tamanioClave] = 0;
            offsetArreglo += sizeof(char) * tamanioClave;

            memcpy(&auxRef, _pares + offsetArreglo, sizeof(int));
            offsetArreglo += sizeof(int);

            (*nodo2)->insert(auxKey, auxRef);

            delete(auxKey);				
        }
    }
    else{
        offsetArreglo = 0;

        memcpy(&tamanioClave,(*arreglo) + offsetArreglo,sizeof(int));
        offsetArreglo += sizeof(int);

        *claveARaiz= new char[(tamanioClave + 1) * sizeof(char)];
        memcpy(*claveARaiz,(*arreglo) + offsetArreglo,sizeof(char)*tamanioClave);
        (*claveARaiz)[tamanioClave] = 0;
    }

    offsetArreglo = 0;

    int clavesCopiadasDesdeArreglo = 0;

    while(clavesCopiadasDesdeArreglo < clavesArreglo){
        memcpy(&tamanioClave,(*arreglo) + offsetArreglo,sizeof(int));
        offsetArreglo += sizeof(int);

        auxKey = new char[(tamanioClave + 1) * sizeof(char)];
        memcpy(auxKey, (*arreglo) + offsetArreglo, sizeof(char) * tamanioClave);
        auxKey[tamanioClave] = 0;
        offsetArreglo += sizeof(char) * tamanioClave;

        memcpy(&auxRef, (*arreglo) + offsetArreglo, sizeof(int));
        offsetArreglo += sizeof(int);

        (*nodo2)->insert(auxKey, auxRef);

        clavesCopiadasDesdeArreglo++;

        delete(auxKey);
    }

    return;
}

void FileDirLeafNode::list(){
    char* auxKey;
    int auxRef;
    int longClave;	
    int offset = 0;

    for(int i = 0;i < _nclaves;++i){
        memcpy(&longClave,_pares + offset,sizeof(int));
        offset += sizeof(int);

        auxKey = new char[(longClave + 1) * sizeof(char)];
        memcpy(auxKey,_pares + offset, sizeof(char) * longClave);
        auxKey[longClave] = 0;
        offset += sizeof(char) * longClave;

        memcpy(&auxRef,_pares + offset,sizeof(int));
        offset += sizeof(int);

        cout<<"clave: "<<auxKey<<" referencia: "<<auxRef<<endl;

        delete(auxKey);
    }
}

int FileDirLeafNode::getFirstOf(const char* key)
{
    char* auxKey;
    char* auxFileName;
    int longClave;
    int auxRef;
    int i = 0;
    int offsetLectura = 0;
    int ret = -1;

    while (i < _nclaves) {
        memcpy(&longClave,_pares + offsetLectura,sizeof(int));
        offsetLectura += sizeof(int);

        auxKey = new char[(longClave + 1) * sizeof(char)];
        memcpy(auxKey, _pares + offsetLectura, sizeof(char) * longClave);
        offsetLectura += sizeof(char) * longClave;
        auxKey[longClave] = 0; // coloco la marca de fin

        // genero el nombre del archivo
        auxFileName = new char[(longClave - 4) * sizeof(char)];
        memcpy(auxFileName,auxKey,(longClave - 5));
        auxFileName[longClave - 5] = 0;

        memcpy(&auxRef, _pares + offsetLectura, sizeof(int));
        offsetLectura += sizeof(int);

        if (strcmp(key, auxFileName) >=  0){
            i++;
            ret = auxRef;
            delete auxKey;
            delete auxFileName;
        }
        else{
            delete auxFileName;
            delete(auxKey);
            return ret;
        }
    }
    return ret;
}

