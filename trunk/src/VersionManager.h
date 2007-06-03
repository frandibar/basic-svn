// VersionManager.h

#ifndef VERSION_MANAGER_H_INCLUDED
#define VERSION_MANAGER_H_INCLUDED

#include "arbolbmas.h"
#include "FileVersion.h"
#include "FileVersionsFile.h"
#include "Container.h"

#include <ctime>
#include <string>
#include <list>

using std::string;


class VersionManager
{
public:
    // static members
    static const string TXT_INDEX_FILENAME;
    static const string TXT_VERSION_FILENAME;
    static const string TXT_DIFFS_FILENAME;

    static const string BIN_INDEX_FILENAME;
    static const string BIN_VERSION_FILENAME;
    static const string BIN_DIFFS_FILENAME;

    static const int VERSION_DIGITS;

    // constructor
    VersionManager(const string& a_Almacen, const string& a_Repository);

    bool open();
    bool addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type);    
    bool create();
    void close();  
    bool isOpen() const { return _isOpen; }

protected:
    bool buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename);

private:
    // member variables
    bool      _isOpen;
    string    _almacen;
    string    _repository;

    ArbolBMas			_textIndex;     // index for text files
    FileVersionsFile	_textVersions;  // version file for text files
    Container			_textContainer; // aca va el archivo de diffs para los de texto
    
    ArbolBMas			_binaryIndex;       // index for binary files
    FileVersionsFile	_binaryVersions;    // version file for binary files
    Container			_binaryContainer;   // aca va el archivo de diffs para los binarios
};

#endif

