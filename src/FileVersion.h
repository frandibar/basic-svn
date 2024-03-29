// version.h

#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>
#include <ctime>

class FileVersion
{
public:

	enum t_versionType { MODIFICACION = 0, BORRADO};

    FileVersion();
    FileVersion(int NroVersion, int Original, tm Fecha, const char* User, long int Offset, char TipoArchivo,t_versionType VersionType);
    ~FileVersion();

    //empaquetadores y desempaquetadores
    void read (char** buffer);
    void write(char* buffer);

    //getters - no hay setters porque no hay modificacion de la info de las versiones
    int				getNroVersion()  const { return _nroVersion; }
    int				getOriginal()    const { return _original;   }
    tm				getFecha()       const { return _fecha;      }
    char*			getUser()        const { return _user;       }
    long int		getOffset()      const { return _offset;     }
    char			getTipo()        const 	  { return _tipo;       }
	t_versionType	getVersionType() const { return _versionType;}

    int tamanioEnDisco();

	FileVersion &operator=(const FileVersion &version);
    int operator==(const FileVersion &version) const;
    int operator<(const FileVersion &version) const;	 

protected:
    
    int				_nroVersion;	// numero de la version        
    int				_original;		// numero de la version que es el ultimo original de este archivo
    tm				_fecha;			// fecha en que se establecio la version
    char*			_user;			// el usuario que establecio la version
    long int		_offset;		// offset donde se encuentra la version en el archivo que contiene originales y diffs
    char			_tipo;			// tipo de archivo del que se trata la version "b" = binario, "t" = texto
	t_versionType	_versionType;	// tipo de version modificacion o borrado
};

#endif

