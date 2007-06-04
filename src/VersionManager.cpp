// VersionManager.cpp

#include "VersionManager.h"
#include "debug.h"
#include "helpers.h"

#include <fstream>
#include <list>

using std::list;

const string VersionManager::FILE_INDEX_FILENAME    = "file_index.ndx";
const string VersionManager::FILE_VERSION_FILENAME  = "file_versions.ndx";

const string VersionManager::DIR_INDEX_FILENAME     = "dir_index.ndx";
const string VersionManager::DIR_VERSION_FILENAME   = "dir_versions.ndx";

const string VersionManager::TXT_DIFFS_FILENAME     = "txt_diffs.dat";
const string VersionManager::BIN_DIFFS_FILENAME     = "bin_diffs.dat";

const string VersionManager::DIR_CONTAINER_FILENAME = "bin_diffs.dat";

const int VersionManager::VERSION_DIGITS = 5;

VersionManager::VersionManager(const string& a_Almacen, const string& a_Repository) 
                                : _isOpen(false), _almacen(a_Almacen), _repository(a_Repository)
{
}

bool VersionManager::destroy()
{
    debug("destroying VersionManager\n");
    bool ret = close();
    ret = ret && _fileIndex      .destroy() &&
                 _fileVersions   .destroy() &&
                 _textContainer  .destroy() &&
                 _binaryContainer.destroy() &&
                 _dirIndex       .destroy() &&
                 _dirVersions    .destroy();

    debug("VersionManager destroy " + string(ret ? "successfull" : "failed") + "\n");
    return ret;
}

bool VersionManager::open()
{
    if (_isOpen)
        return true;

    debug("opening VersionManager\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_fileIndex      .open((path + FILE_INDEX_FILENAME)  .c_str()) &&
               _fileVersions   .open((path + FILE_VERSION_FILENAME).c_str()) &&
               _textContainer  .open((path + TXT_DIFFS_FILENAME)   .c_str()) &&
               _binaryContainer.open((path + BIN_DIFFS_FILENAME)   .c_str()) &&
               _dirIndex       .open((path + DIR_INDEX_FILENAME)   .c_str()) &&
               _dirVersions    .open((path + DIR_VERSION_FILENAME) .c_str())
              );

    debug("VersionManager open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool VersionManager::close()
{
    if (!_isOpen)
        return true;

    debug("closing VersionManager\n");
    bool ret = _fileIndex      .close() &&
               _fileVersions   .close() &&
               _textContainer  .close() &&
               _binaryContainer.close() &&
               _dirIndex       .close() &&
               _dirVersions    .close();
    debug("VersionManager close " + string((ret) ? "successfull" : "failed") + "\n");

    return ret;
}

bool VersionManager::buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename)
// generates a file name a_Filename with the versions    
{
    std::ofstream osfinal(a_Filename.c_str());
    if (!osfinal.is_open())
        return false;

    _textContainer.get(lstVersions.begin()->getOffset(), osfinal);
    osfinal.close();

    if (lstVersions.size() > 1) {
        std::list<FileVersion>::const_iterator it;
        it = lstVersions.begin();
        ++it;
        for ( ; it != lstVersions.end(); ++it) {
            // save diff into a temporary file
            string tmpFilename = randomFilename("tmp_");
            std::ofstream osdiff(tmpFilename.c_str());
            _textContainer.get(it->getOffset(), osdiff);
            osdiff.close();

            // apply diff to last version
            string cmd = "ed -s " + a_Filename + " < " + tmpFilename;
            if (system(cmd.c_str()) != 0)
                return false;
            cmd = "rm -f " + tmpFilename;
            system(cmd.c_str());
        }
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
    // busco en el indice a ver si esta el archivo
    bloque = _fileIndex.searchFile(a_Filename.c_str());

    if (bloque >= 0) { // el archivo esta en el indice
        FileVersion* ultimaVersion;
        _fileVersions.getLastVersion(&ultimaVersion, bloque);
        if (ultimaVersion->getTipo() != a_Type) {
            delete ultimaVersion;
            return false;
        }
        delete ultimaVersion;

        if (a_Type == 't') {
            // debo insertar el diff si el archivo ya existe o el original sino,
            // al insertar voy a obtener el valor del offset
            int original = _fileVersions.getLastOriginalVersionNumber(bloque);
            int last     = _fileVersions.getLastVersionNumber(bloque);
			
            list<FileVersion> lstVersions;
            if (!_fileVersions.getVersionFrom(original, last, bloque, lstVersions)) {
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
                if (system(cmd.c_str()) == -1)
                    return false;
                cmd = "echo w >> " + tmpDiffFilename;
                if (system(cmd.c_str()) == -1)
                    return false;

                std::ifstream is(tmpDiffFilename.c_str());
                offset = _textContainer.append(is);
                is.close();

                // remove temporary files
                remove(tmpVersionFilename.c_str());
                remove(tmpDiffFilename.c_str());
            }

            // TODO: falta ver si la ultima version es la de borrado

            FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset,
                                                           a_Type, FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
            switch (status) {
                case FileVersionsFile::OK :
                    return true;
                    break;

                case FileVersionsFile::OVERFLOW :
                    // tengo que generar la clave a partir de a_File y repositoryVersion
                    return _fileIndex.insert(key.c_str(), nroNuevoBloque);

                default:
                    return false;
            }
        }
        else if (a_Type == 'b') {
            std::ifstream is(a_Filename.c_str());
            if (!is) 
                return false;
            offset = _binaryContainer.append(is);
            is.close();
            if (offset == -1) 
                return false;

            if (bloque >= 0) { // el archivo esta en el indice
                FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
                switch (status) {
                    case FileVersionsFile::OK :
                        return true;
                        break;
                    case FileVersionsFile::OVERFLOW :
                        // tengo que generar la clave a partir de a_File y repositoryVersion
                        key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
                        return _fileIndex.insert(key.c_str(), nroNuevoBloque);
                    default:
                        return false;
                }
            }
        }
    }
    else {
       // debo insertar el archivo completo
       std::ifstream is(a_Filename.c_str());
       if (!is.is_open()) 
           return false;	
    
       if (a_Type == 't')
           offset = _textContainer.append(is);
       else
           offset = _binaryContainer.append(is);

       is.close();
       if (offset == -1) 
           return false;

       if (!_fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, FileVersion::MODIFICACION, &nroNuevoBloque))
           return false;
       key = a_Filename + zeroPad(repositoryVersion, VERSION_DIGITS);
       return (_fileIndex.insert(key.c_str(), nroNuevoBloque));
    }   

    return false; // never gets here
}

bool VersionManager::create()
{
    if (_isOpen)
        return true;

    debug("creating VersionManager for Repositorio '" + _repository + "' in Almacen '" + _almacen + "'\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_fileIndex      .create((path + FILE_INDEX_FILENAME)  .c_str()) &&
	           _fileVersions   .create((path + FILE_VERSION_FILENAME).c_str()) &&
               _textContainer  .create((path + TXT_DIFFS_FILENAME)   .c_str()) &&
               _binaryContainer.create((path + BIN_DIFFS_FILENAME)   .c_str()) &&
               _dirIndex       .create((path + DIR_INDEX_FILENAME)   .c_str()) &&
               _dirVersions    .create((path + DIR_VERSION_FILENAME) .c_str())
              );

    debug("VersionManager creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool VersionManager::getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version)
{
    if (!_isOpen)
        return false;

    FileVersion* versionBuscada;
    int bloque;

    int version = -1;
    if (a_Version != "") {
        version = fromString<int>(a_Version);
        bloque = _fileIndex.searchFileAndVersion(a_Filename.c_str(), version);
        if (!_fileVersions.searchVersion(&versionBuscada, version, bloque)) {
            bloque = _fileIndex.searchFile(a_Filename.c_str());
            if(!_fileVersions.getLastVersion(&versionBuscada, bloque))
                return false;
            else if(versionBuscada->getNroVersion() > version) {
                delete versionBuscada;
                return false;
            }
        }        
    }
    else {
        bloque = _fileIndex.searchFile(a_Filename.c_str());
        if (!_fileVersions.getLastVersion(&versionBuscada, bloque))
           return false; 
    }

    char ftype = versionBuscada->getTipo();
    if (ftype == 't') {
        int original = versionBuscada->getOriginal();
        // busco en el indice a ver si esta el archivo
        list<FileVersion> versionsList;
        int final = versionBuscada->getNroVersion();
        delete versionBuscada;
        if (_fileVersions.getVersionFrom(original, final, bloque, versionsList)) {
            return (buildVersion(versionsList, a_TargetDir + "//" + a_Filename));
        }
        else
            return false;
    }
    else if (ftype == 'b') {
        // TODO
        return false;
    }
    return false; // never gets here
}

