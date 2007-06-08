// VersionManager.h

#ifndef VERSION_MANAGER_H_INCLUDED
#define VERSION_MANAGER_H_INCLUDED

#include "arbolbmas.h"
#include "Container.h"
#include "FileVersion.h"
#include "DirectoryVersion.h"
#include "FileVersionsFile.h"
#include "DirectoryVersionsFile.h"
#include "helpers.h"

#include <ctime>
#include <string>
#include <list>
#include <sys/stat.h>

using std::string;


class VersionManager
{
public:
    // static members
    static const string FILE_INDEX_FILENAME;
    static const string FILE_VERSION_FILENAME;

    static const string DIR_INDEX_FILENAME;
    static const string DIR_VERSION_FILENAME;

    static const string TXT_DIFFS_FILENAME;
    static const string BIN_DIFFS_FILENAME;

    static const string DIR_CONTAINER_FILENAME;

    static const int VERSION_DIGITS;

    // constructor
    VersionManager(const string& a_Almacen, const string& a_Repository);

    bool create();
    bool destroy();  
    
    bool open();
    bool close();  

    bool isOpen() const { return _isOpen; }
    bool addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type);
    bool add(int repositoryVersion, const string& a_Target, const string& a_User, time_t a_Date, t_filetype a_Type, const string& repositoryName);
    bool addRec(const string& a_Target, int componenteALeer, const string& pathActual, int repositoryVersion, int cantComponentesPath, const string& a_Username, time_t a_Date, t_filetype a_Type, const string& repositoryName);
    bool getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version);
    bool getDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Filename);

protected:
    bool buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename);
    bool getVersionAndBlock(int* bloque, FileVersion** versionBuscada, const string& a_Filename, const string& a_Version);
    bool buildTextVersion(int bloque, FileVersion* versionBuscada, const string& a_Filename);

private:
    // member variables
    bool      _isOpen;
    string    _almacen;
    string    _repository;

    ArbolBMas        _fileIndex;
    FileVersionsFile _fileVersions;

    ArbolBMas        		_dirIndex;
    DirectoryVersionsFile 	_dirVersions;

    Container        _textContainer;
    Container        _binaryContainer;
    //DirContainer     _dirContainer;
};

#endif

