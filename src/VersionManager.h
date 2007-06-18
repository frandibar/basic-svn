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

    bool addFile(int repositoryVersion,const string& repositoryName, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type);
    bool addDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date);
    bool add(int repositoryVersion,const string& repositoryName, const string& a_Target, const string& a_User, time_t a_Date, t_filetype        a_Type);
    bool addRec(const string& a_Target, int componenteALeer, const string& pathActual,const string& repositoryName, int repositoryVersion,       int cantComponentesPath, const      string& a_Username, time_t a_Date, t_filetype a_Type);

    bool getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version,const string& repositoryName);
    bool getDirectory(const string& a_TargetDir, const string& pathToFile, const string& a_Path, const string& a_DirName, const string& a_Version,const string& repositoryName);
    bool get(const string& a_Version, const string& a_Target,const string& repositoryName, const string& a_TargetDestiny);
    bool getDiffByDate(std::ifstream& is, const string& a_Date);
    bool getHistory(std::ifstream& is, const string& a_Filename);

    bool getDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Target, const string& repositoryName);
    bool getFileDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Filename);
    bool getDirectoryDiff(const string& a_DirName, const string& a_VersionA, const string& a_VersionB, int tabs);

    bool removeFileOrDirectory(int repositoryVersion, const string& repositoryName, const string& pathActual, const string& a_User, time_t a_Date);
    bool removeFile(int repositoryVersion, const string& repositoryName, const string& a_Filename, const string& a_User, time_t a_Date);
    bool removeDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date);

protected:
    bool buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename);
    bool buildTextVersion(int bloque, FileVersion* versionBuscada, const string& a_Filename);
    bool getFileVersionAndBlock(int* bloque, FileVersion** versionBuscada, const string& a_Filename, const string& a_Version);
    bool getDirVersion(DirectoryVersion** versionBuscada, const string& a_Dirname, const string& a_Version);
	bool indexAFile(int repositoryVersion, const string& key, const string& a_User, tm* date, int offset, char a_Type, FileVersion::t_versionType a_VersionType, int bloque);
    void showAddedDirectory(DirectoryVersion* dirVersion, const string& path, int tabs);

	bool indexADirectory(int repositoryVersion, const string& key, DirectoryVersion* nuevaVersion, int bloque);

private:
    // member variables
    bool      _isOpen;
    string    _almacen;
    string    _repository;

    ArbolBMas        _fileIndex;
    FileVersionsFile _fileVersions;

    ArbolBMas             _dirIndex;
    DirectoryVersionsFile _dirVersions;

    Container        _textContainer;
    Container        _binaryContainer;
};

#endif

