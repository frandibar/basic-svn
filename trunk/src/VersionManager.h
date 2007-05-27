// VersionManager.h

#include "arbolbmas.h"
#include <string>

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

    VersionManager(const string& a_Path);
    VersionManager() {};

    bool open(const string& a_Path, const string& a_Name);
    bool insert(const string& a_Filename);

    //string filename = a_Path + string("//") + a_Name + string(".ndx");

private:
    // member variables
    string    _path;
    ArbolBMas _index;

};


