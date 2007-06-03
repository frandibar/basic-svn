// Version.cpp

#include "FileVersion.h"

FileVersion::FileVersion()
{
    _nroVersion		= -1;
    _original		= -1;
    _tipo			= 0;
    _offset			= -1;
    _user			= 0;
	_versionType	= MODIFICACION;
}

FileVersion::FileVersion(int NroVersion, int Original, tm Fecha, const char* User, long int Offset, char Tipo,t_versionType VersionType)
{
    _nroVersion = NroVersion;
    _original = Original;
    _fecha = Fecha;
    _offset = Offset;
    _tipo = Tipo;
	_versionType = VersionType;

    int tamanio = strlen(User);

    _user = new char[(tamanio + 1) * sizeof(char)];

    memcpy(_user, User, tamanio * sizeof(char));
    _user[tamanio] = 0;
}

FileVersion::~FileVersion()
{
	if(_user);
		delete _user;
}

void FileVersion::write(char* buffer)
{
    int tamanioUsuario;
    //copio el nro de version
    memcpy(buffer,&_nroVersion,sizeof(int));
    buffer += sizeof(int);

    //copio el nº de version original
    memcpy(buffer,&_original,sizeof(int));
    buffer += sizeof(int);

    //copio la fecha de la version
    memcpy(buffer,&_fecha,sizeof(tm));
    buffer += sizeof(tm);

    //copio el offset dentro del archivo de originales y diffs
    memcpy(buffer,&_offset,sizeof(long int));
    buffer += sizeof(long int);

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

	//tipo de version
	memcpy(buffer,&_versionType,sizeof(t_versionType));
	buffer += sizeof(t_versionType);

    return;
}

void FileVersion::read(char** buffer)
{
    int tamanioUsuario;
    // leo el nro de version
    memcpy(&_nroVersion,*buffer,sizeof(int));
    *buffer += sizeof(int);

    //leo el nº de version original
    memcpy(&_original,*buffer,sizeof(int));
    *buffer += sizeof(int);

    //leo la fecha de la version
    memcpy(&_fecha, *buffer, sizeof(tm));
    *buffer += sizeof(tm);

    //leo el offset de la version dentro del archivo de originales y diffs
    memcpy(&_offset,*buffer,sizeof(int));
    *buffer+= sizeof(int);

    //leo el usuario, previamente leo la longitud de este campo
    memcpy(&tamanioUsuario,*buffer,sizeof(int));
    *buffer += sizeof(int);

    if(_user != 0){
        delete _user;
        _user = 0;
    }
    _user = new char[(tamanioUsuario + 1)*sizeof(char)];    // creo el campo
    memcpy(_user,*buffer,tamanioUsuario * sizeof(char)); // copio los caracteres
    _user[tamanioUsuario] = 0;  // coloco la marca de fin
    *buffer += tamanioUsuario * sizeof(char);

    //leo el tipo de archivo
    memcpy(&_tipo,*buffer,sizeof(char));
    *buffer += sizeof(char);

	//leo el tipo de version
	memcpy(&_versionType,*buffer,sizeof(t_versionType));
	*buffer += sizeof(t_versionType);

    return;
}

int FileVersion::tamanioEnDisco(){
    int tamanio = sizeof(int);					//nro de version 
		tamanio+= sizeof(int);					//nro del original                  
        tamanio+= sizeof(tm);					//fecha
		tamanio+= sizeof(long int);				//offset				  
		tamanio+= sizeof(int);					//longitud del atributo nombre usuario				  
        tamanio+= strlen(_user) * sizeof(char);	//usuario                   
        tamanio+= sizeof(char);					//tipo
		tamanio+= sizeof(t_versionType);		//tipo de version

    return tamanio;
}

FileVersion& FileVersion::operator=(const FileVersion &version)
{
    _nroVersion		= version._nroVersion;
    _original		= version._original;
    _fecha			= version._fecha;
    _offset			= version._offset;
    _tipo			= version._tipo;
	_versionType	= version._versionType;

	int tamanio = strlen(version._user);
    delete _user;
	_user = new char[(tamanio + 1) * sizeof(char)];
	strcpy(_user, version._user);
	return *this;
}

int FileVersion::operator==(const FileVersion &version) const
{
   return ((this->_nroVersion != version._nroVersion) ? 0 : 1);
}


int FileVersion::operator<(const FileVersion &version) const
{
   return ((this->_nroVersion < version._nroVersion) ? 1 : 0);
}





 
