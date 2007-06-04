// VersionManager.h

#ifndef VERSION_MANAGER_H_INCLUDED
#define VERSION_MANAGER_H_INCLUDED

#include "arbolbmas.h"
#include "Container.h"
#include "FileVersion.h"
#include "FileVersionsFile.h"
#include "DirectoryVersionsFile.h"

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

    static const string DIR_INDEX_FILENAME;
    static const string DIR_VERSION_FILENAME;

    static const int VERSION_DIGITS;

    // constructor
    VersionManager(const string& a_Almacen, const string& a_Repository);

    bool create();
    bool destroy();  
    
    bool open();
    bool close();  

    bool isOpen() const { return _isOpen; }
    bool addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type);    
    bool getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version, t_filetype a_Filetype);

protected:
    bool buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename);

private:
    // member variables
    bool      _isOpen;
    string    _almacen;
    string    _repository;

    ArbolBMas             _textIndex;     // index for text files
    FileVersionsFile      _textVersions;  // version file for text files
    Container             _textContainer; // aca va el archivo de diffs para los de texto
    
    ArbolBMas             _binaryIndex;       // index for binary files
    FileVersionsFile      _binaryVersions;    // version file for binary files
    Container             _binaryContainer;   // aca va el archivo de diffs para los binarios

    ArbolBMas             _dirIndex;       // index for directories
    DirectoryVersionsFile _dirVersions;    // version file for directories
};

#endif

