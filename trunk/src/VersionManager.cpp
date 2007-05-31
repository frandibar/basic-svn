// VersionManager.cpp

#include "VersionManager.h"
#include "helpers.h"

#include <fstream>

const string VersionManager::TX_INDEX_FILENAME   = "tx_index.ndx";
const string VersionManager::TX_VERSION_FILENAME = "tx_versions.ndx";
const string VersionManager::TX_DIFFS_FILENAME   = "tx_diffs.dat";

const string VersionManager::BIN_INDEX_FILENAME   = "bin_index.ndx";
const string VersionManager::BIN_VERSION_FILENAME = "bin_versions.ndx";
const string VersionManager::BIN_DIFFS_FILENAME   = "bin_diffs.dat";

const int VersionManager::VERSION_DIGITS = 5;

VersionManager::VersionManager(const string& a_Almacen, const string& a_Repository) 
                                : _almacen(a_Almacen), _repository(a_Repository)
{

}

bool VersionManager::open()
{
    string path = _almacen + "//" + _repository + "//";
    bool ret = (_textIndex      .open((path + TX_INDEX_FILENAME)   .c_str()) &&
	            _textVersions   .open((path + TX_VERSION_FILENAME) .c_str()) &&
                _textContainer  .open((path + TX_DIFFS_FILENAME)   .c_str()) &&
                _binaryIndex    .open((path + BIN_INDEX_FILENAME)  .c_str()) &&
                _binaryVersions .open((path + BIN_VERSION_FILENAME).c_str()) &&
                _binaryContainer.open((path + BIN_DIFFS_FILENAME)  .c_str())
                );

    _isOpen = ret;
    return ret;
}

void VersionManager::close()
{
  _textIndex      .close();
  _textVersions   .close();
  _textContainer  .close();
  _binaryIndex    .close();
  _binaryVersions .close();
  _binaryContainer.close();

  return;
}

void compare(const string& a_FilenameA, const string& a_FilenameB)
{
    string cmd = "diff -e " + a_FilenameA + " " + a_FilenameB + " > tmp";
    system(cmd.c_str());
    remove("tmp");
}

    
bool VersionManager::addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type)
{
    if (!_isOpen)
        return false;

    int bloque;
    int nroNuevoBloque;
    long int offset;
    string key;
    if (a_Type == 't') {
        // busco en el indice a ver si esta el archivo
        bloque = _textIndex.searchFile(a_Filename.c_str());

        if (bloque >= 0) { // el archivo esta en el indice
            //debo insertar el diff si el archivo ya existe o el original si no,
            //al insertar voy a obtener el valor del offset
				
			
            VersionFile::t_status status = _textVersions.insertVersion(repositoryVersion, a_User.c_str(), a_Date, offset, 							
				a_Type, bloque, &nroNuevoBloque);
            switch (status) {
                case VersionFile::OK :
                    return true;
                    break;
                case VersionFile::OVERFLOW :
                    //tengo que generar la clave a partir de a_File y repositoryVersion
                    return _textIndex.insert(key.c_str(), nroNuevoBloque);
                default:
                    return false;
            }
        }
        else{
           // debo insertar el archivo completo
		   std::ifstream is(a_Filename.c_str());
	   	if (!is) return false;	
	   	
			offset =  _textContainer.append(is);
	   	
			is.close();
	   	
			if (offset == -1) return false;
         _textVersions.insertVersion(repositoryVersion, a_User.c_str(), a_Date, offset, a_Type, &nroNuevoBloque);
         key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
         _textIndex.insert(key.c_str(), nroNuevoBloque);	   
        }   
}

    if (a_Type == 'b') {
        std::ifstream is(a_Filename.c_str());
        if (!is) return false;
        offset = _binaryContainer.append(is);
	is.close();
        if (offset == -1) return false;

        // busco en el indice a ver si esta el archivo
        bloque = _binaryIndex.searchFile(a_Filename.c_str());

        if (bloque >= 0) { // el archivo esta en el indice
            VersionFile::t_status status = _binaryVersions.insertVersion(repositoryVersion, a_User.c_str(), a_Date, offset, a_Type, bloque, &nroNuevoBloque);
            switch (status) {
                case VersionFile::OK :
                    return true;
                    break;
                case VersionFile::OVERFLOW :
                    // tengo que generar la clave a partir de a_File y repositoryVersion
                    key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
                    return _binaryIndex.insert(key.c_str(), nroNuevoBloque);
                default:
                    return false;
            }
        }
        else {
            // debo insertar el archivo completo.	
            _binaryVersions.insertVersion(repositoryVersion, a_User.c_str(), a_Date, offset, a_Type, &nroNuevoBloque);
            key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
            _binaryIndex.insert(key.c_str(), nroNuevoBloque);
        }
    }
    return false;
}

bool VersionManager::create()
{
    string path = _almacen + "//" + _repository + "//";
    bool ret = (_textIndex      .create((path + TX_INDEX_FILENAME)   .c_str()) &&
	            _textVersions   .create((path + TX_VERSION_FILENAME) .c_str()) &&
                _textContainer  .create((path + TX_DIFFS_FILENAME)   .c_str()) &&
                _binaryIndex    .create((path + BIN_INDEX_FILENAME)  .c_str()) &&
                _binaryVersions .create((path + BIN_VERSION_FILENAME).c_str()) &&
                _binaryContainer.create((path + BIN_DIFFS_FILENAME)  .c_str())
                );

    _isOpen = ret;
    return ret;
}
