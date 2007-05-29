#include "version.h"

Version::Version()
{
    _nroVersion = -1;
    _original = -1;
    _fecha = -1;
    _tipo = 0;
    _offset = -1;

    _user = 0;
}

Version::Version(int NroVersion, int Original, time_t Fecha, const char* User, long int Offset, char Tipo){
    _nroVersion = NroVersion;
    _original = Original;
    _fecha = Fecha;
    _offset = Offset;
    _tipo = Tipo;
    
    int tamanio = strlen(User);

    _user = new char[(tamanio + 1) * sizeof(char)];

    memcpy(_user,User,tamanio*sizeof(char));
    _user[tamanio] = 0;
}

Version::~Version()
{
    delete _user;
}

void Version::write(char* buffer)
{
    int tamanioUsuario;
    //copio el nro de version
    memcpy(buffer,&_nroVersion,sizeof(int));
    buffer += sizeof(int);

    //copio el nº de version original
    memcpy(buffer,&_original,sizeof(int));
    buffer += sizeof(int);

    //copio la fecha de la version
    memcpy(buffer,&_fecha,sizeof(time_t));
    buffer += sizeof(time_t);

    //copio el offset dentro del archivo de originales y diffs
    memcpy(buffer,&_offset,sizeof(int));
    buffer += sizeof(int);

    //copio el tamaño del campo usuario
    tamanioUsuario = strlen(_user);
    memcpy(buffer,&tamanioUsuario,sizeof(int));
    buffer += sizeof(int);

    //copio el campo usuario
    memcpy(buffer,_user,tamanioUsuario * sizeof(char));
    buffer += tamanioUsuario * sizeof(char);

    //copio el tipo de archivo
    memcpy(buffer,&_tipo,sizeof(char));
    buffer += sizeof(char);

    return;
}

void Version::read(char* buffer)
{
    int tamanioUsuario;
    // leo el nro de version
    memcpy(&_nroVersion,buffer,sizeof(int));
    buffer += sizeof(int);

    //leo el nº de version original
    memcpy(&_original,buffer,sizeof(int));
    buffer += sizeof(int);

    //leo la fecha de la version
    memcpy(&_fecha, buffer, sizeof(time_t));
    buffer += sizeof(time_t);

    //leo el offset de la version dentro del archivo de originales y diffs
    memcpy(&_offset,buffer,sizeof(int));
    buffer+= sizeof(int);

    //leo el usuario, previamente leo la longitud de este campo
    memcpy(&tamanioUsuario,buffer,sizeof(int));
    buffer += sizeof(int);

    
    if(_user != 0){
        delete _user;
        _user = 0;
    }
    _user = new char[(tamanioUsuario + 1)*sizeof(char)];    // creo el campo
    memcpy(_user,buffer,tamanioUsuario * sizeof(char)); // copio los caracteres
    _user[tamanioUsuario] = 0;  // coloco la marca de fin
    buffer += tamanioUsuario * sizeof(char);

    //leo el tipo de archivo
    memcpy(&_tipo,buffer,sizeof(char));
    buffer += sizeof(char);

    return;
}

int Version::tamanioEnDisco(){
    int tamanio =   sizeof(int) +                   //nro de version 
                    sizeof(int) +                   //nro del original
                    sizeof(long int) +              //fecha
                    sizeof(int) +                   //longitud del atributo nombre usuario
                    strlen(_user) * sizeof(char) +  //usuario
                    sizeof(char);                   //tipo

    return tamanio;
}






 
