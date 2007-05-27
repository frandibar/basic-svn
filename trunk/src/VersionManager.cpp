// VersionManager.cpp

#include "VersionManager.h"


const string TX_INDEX_FILENAME   = "tx_index.ndx";
const string TX_VERSION_FILENAME = "tx_versions.ndx";
const string TX_DIFFS_FILENAME   = "tx_diffs.dat";

const string BIN_INDEX_FILENAME   = "bin_index.ndx";
const string BIN_VERSION_FILENAME = "bin_versions.ndx";
const string BIN_DIFFS_FILENAME   = "bin_diffs.dat";

VersionManager::VersionManager(const string& a_Path) : _path(a_Path)
{

}


bool VersionManager::open(const string& a_Path, const string& a_Name)
{
    return false;
}

bool VersionManager::insert(const string& a_Filename)
{
    return false;
}
