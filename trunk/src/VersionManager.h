// VersionManager.h

#include "arbolbmas.h"
#include "version.h"
#include "versionFile.h"
#include "version.h"
#include "Container.h"

#include <string>
#include <ctime>

using std::string;


class VersionManager
{
public:
    static const string TX_INDEX_FILENAME;
    static const string TX_VERSION_FILENAME;
    static const string TX_DIFFS_FILENAME;

    static const string BIN_INDEX_FILENAME;
    static const string BIN_VERSION_FILENAME;
    static const string BIN_DIFFS_FILENAME;

    static const int VERSION_DIGITS;

    VersionManager() {};
    VersionManager(const string& a_Almacen, const string& a_Repository);

    bool open();
    bool addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type);    
    bool create();
    void close();  
    bool isOpen() const { return _isOpen; }

private:
    // member variables
    bool      _isOpen;
    string    _almacen;
    string    _repository;

    ArbolBMas   _textIndex;     // index for text files
    VersionFile _textVersions;  // version file for text files
    Container   _textContainer; // aca va el archivo de diffs para los de texto
    
    ArbolBMas   _binaryIndex;       // index for binary files
    VersionFile _binaryVersions;    // version file for binary files
    Container   _binaryContainer;   // aca va el archivo de diffs para los binarios
};


