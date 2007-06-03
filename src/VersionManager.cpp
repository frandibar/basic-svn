// VersionManager.cpp

#include "VersionManager.h"
#include "debug.h"
#include "helpers.h"

#include <fstream>
#include <list>

using std::list;

const string VersionManager::TXT_INDEX_FILENAME   = "txt_index.ndx";
const string VersionManager::TXT_VERSION_FILENAME = "txt_versions.ndx";
const string VersionManager::TXT_DIFFS_FILENAME   = "txt_diffs.dat";

const string VersionManager::BIN_INDEX_FILENAME   = "bin_index.ndx";
const string VersionManager::BIN_VERSION_FILENAME = "bin_versions.ndx";
const string VersionManager::BIN_DIFFS_FILENAME   = "bin_diffs.dat";

const int VersionManager::VERSION_DIGITS = 5;

VersionManager::VersionManager(const string& a_Almacen, const string& a_Repository) 
                                : _almacen(a_Almacen), _repository(a_Repository)
{
}

bool VersionManager::destroy()
{
    debug("destroying VersionManager\n");
    bool ret = close();
    //ret = ret && _textIndex      .destroy() &&
                 //_textVersions   .destroy() &&
                 //_textContainer  .destroy() &&
                 //_binaryIndex    .destroy() &&
                 //_binaryVersions .destroy() &&
                 //_binaryContainer.destroy();

    ret = ret && _textIndex      .destroy();
    ret = ret && _textVersions   .destroy();
    ret = ret && _textContainer  .destroy();
    ret = ret && _binaryIndex    .destroy();
    ret = ret && _binaryVersions .destroy();
    ret = ret && _binaryContainer.destroy();

    debug("VersionManager destroy " + string(ret ? "successfull" : "failed") + "\n");
    return ret;
}

bool VersionManager::open()
{
    if (_isOpen)
        return true;

    debug("opening VersionManager\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_textIndex      .open((path + TXT_INDEX_FILENAME)  .c_str()) &&
               _textVersions   .open((path + TXT_VERSION_FILENAME).c_str()) &&
               _textContainer  .open((path + TXT_DIFFS_FILENAME)  .c_str()) &&
               _binaryIndex    .open((path + BIN_INDEX_FILENAME)  .c_str()) &&
               _binaryVersions .open((path + BIN_VERSION_FILENAME).c_str()) &&
               _binaryContainer.open((path + BIN_DIFFS_FILENAME)  .c_str())
              );

    debug("VersionManager open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool VersionManager::close()
{
    if (!_isOpen)
        return true;

    debug("closing VersionManager\n");
    bool ret = _textIndex      .close() &&
               _textVersions   .close() &&
               _textContainer  .close() &&
               _binaryIndex    .close() &&
               _binaryVersions .close() &&
               _binaryContainer.close();
    debug("VersionManager close " + string((ret) ? "successfull" : "failed") + "\n");

    return ret;
}

bool VersionManager::buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename)
{
    std::ofstream os(a_Filename.c_str());
    if (!os.is_open())
        return false;

    std::list<FileVersion>::const_iterator it;
    for (it = lstVersions.begin(); it != lstVersions.end(); ++it) {
        _textContainer.get(it->getOffset(), os);
    }
    return true;
}

    
bool VersionManager::addFile(int repositoryVersion, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type)
{
    if (!_isOpen)
        return false;

    int bloque;
    int nroNuevoBloque;
    long int offset;
    string key;

    tm* date = localtime(&a_Date);

    if (a_Type == 't') {
        // busco en el indice a ver si esta el archivo
        bloque = _textIndex.searchFile(a_Filename.c_str());

        if (bloque >= 0) { // el archivo esta en el indice
            // debo insertar el diff si el archivo ya existe o el original sino,
            // al insertar voy a obtener el valor del offset
            int original = _textVersions.getLastOriginalVersionNumber(bloque);
            int last     = _textVersions.getLastVersionNumber(bloque);
			
            list<FileVersion> lstVersions;
            if (!_textVersions.getVersionFrom(original, last, bloque, lstVersions)) {
                // debo insertar el archivo completo
                std::ifstream is(a_Filename.c_str());
                if (!is) 
                    return false;	

                offset = _textContainer.append(is);
                is.close();

                if (offset == -1) 
                    return false;
            }
            else {
                string tmpVersionFilename = randomFilename("tmp_");
                buildVersion(lstVersions, tmpVersionFilename);
                // fsVersion contains the file up to version
                // now we need to generate a diff between the file being commited and the last version

                string tmpDiffFilename = randomFilename("tmp_");
                string cmd = "diff -e " + tmpVersionFilename + " " + a_Filename + " > " + tmpDiffFilename;
                system(cmd.c_str());

                std::ifstream is(tmpDiffFilename.c_str());
                offset = _textContainer.append(is);
                is.close();

                // remove temporary files
                remove(tmpVersionFilename.c_str());
                remove(tmpDiffFilename.c_str());
            }

            // TODO: falta ver si la ultima version es la de borrado

            FileVersionsFile::t_status status = _textVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset,
                                                           a_Type, FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
            switch (status) {
                case FileVersionsFile::OK :
                    return true;
                    break;

                case FileVersionsFile::OVERFLOW :
                    // tengo que generar la clave a partir de a_File y repositoryVersion
                    return _textIndex.insert(key.c_str(), nroNuevoBloque);

                default:
                    return false;
            }
        }
        else {
           // debo insertar el archivo completo
		   std::ifstream is(a_Filename.c_str());
           if (!is) 
               return false;	
	   	
           offset = _textContainer.append(is);
           is.close();

           if (offset == -1) 
               return false;

           _textVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, FileVersion::MODIFICACION, &nroNuevoBloque);
           key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
           _textIndex.insert(key.c_str(), nroNuevoBloque);	   
        }   
    }

    if (a_Type == 'b') {
        std::ifstream is(a_Filename.c_str());
        if (!is) 
            return false;
        offset = _binaryContainer.append(is);
        is.close();
        if (offset == -1) 
            return false;

        // busco en el indice a ver si esta el archivo
        bloque = _binaryIndex.searchFile(a_Filename.c_str());

        if (bloque >= 0) { // el archivo esta en el indice
            FileVersionsFile::t_status status = _binaryVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
            switch (status) {
                case FileVersionsFile::OK :
                    return true;
                    break;
                case FileVersionsFile::OVERFLOW :
                    // tengo que generar la clave a partir de a_File y repositoryVersion
                    key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
                    return _binaryIndex.insert(key.c_str(), nroNuevoBloque);
                default:
                    return false;
            }
        }
        else {
            // debo insertar el archivo completo.	
            _binaryVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type,FileVersion::MODIFICACION, &nroNuevoBloque);
            key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
            _binaryIndex.insert(key.c_str(), nroNuevoBloque);
        }
    }
    return false;
}

bool VersionManager::create()
{
    if (_isOpen)
        return true;

    debug("creating VersionManager for Repositorio '" + _repository + "' in Almacen '" + _almacen + "'\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_textIndex      .create((path + TXT_INDEX_FILENAME)  .c_str()) &&
	           _textVersions   .create((path + TXT_VERSION_FILENAME).c_str()) &&
               _textContainer  .create((path + TXT_DIFFS_FILENAME)  .c_str()) &&
               _binaryIndex    .create((path + BIN_INDEX_FILENAME)  .c_str()) &&
               _binaryVersions .create((path + BIN_VERSION_FILENAME).c_str()) &&
               _binaryContainer.create((path + BIN_DIFFS_FILENAME)  .c_str())
              );

    debug("VersionManager creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}
