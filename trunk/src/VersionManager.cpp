// VersionManager.cpp

#include "VersionManager.h"
#include "debug.h"
#include "helpers.h"
#include <fstream>
#include <list>
#include <iterator>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

using std::list;
using std::iterator;

using namespace std;

const string VersionManager::FILE_INDEX_FILENAME     = "file_index.ndx";
const string VersionManager::FILE_VERSION_FILENAME   = "file_versions.ndx";

const string VersionManager::DIR_INDEX_FILENAME      = "dir_index.ndx";
const string VersionManager::DIR_VERSION_FILENAME    = "dir_versions.ndx";

const string VersionManager::TXT_DIFFS_FILENAME      = "txt_diffs.dat";
const string VersionManager::BIN_DIFFS_FILENAME      = "bin_diffs.dat";

const string VersionManager::DIR_CONTAINER_FILENAME  = "bin_diffs.dat";

const string VersionManager::DATE_INDEX_FILENAME     = "date_index.ndx";
const string VersionManager::DATE_LOG_FILENAME       =  "date_log.txt";

const string VersionManager::USERS_INDEX_FILENAME    = "users_index.ndx";
const string VersionManager::USERS_REGISTER_FILENAME = "users_register.ndx";

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
                 _dirVersions    .destroy() &&
                 _dateIndex      .destroy() &&
                 _dateLog        .destroy() &&
                 _usersIndex     .destroy() &&
                 _usersReg       .destroy();

    debug("VersionManager destroy " + string(ret ? "successfull" : "failed") + "\n");
    return ret;
}

bool VersionManager::open()
{
    if (_isOpen)
        return true;

    debug("opening VersionManager\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_fileIndex      .open((path + FILE_INDEX_FILENAME)    .c_str()) &&
               _fileVersions   .open((path + FILE_VERSION_FILENAME)  .c_str()) &&
               _textContainer  .open((path + TXT_DIFFS_FILENAME)     .c_str()) &&
               _binaryContainer.open((path + BIN_DIFFS_FILENAME)     .c_str()) &&
               _dirIndex       .open((path + DIR_INDEX_FILENAME)     .c_str()) &&
               _dirVersions    .open((path + DIR_VERSION_FILENAME)   .c_str()) &&
               _dateIndex      .open((path + DATE_INDEX_FILENAME)    .c_str()) &&
               _dateLog        .open((path + DATE_LOG_FILENAME)      .c_str()) &&
               _usersIndex     .open((path + USERS_INDEX_FILENAME)   .c_str()) &&
               _usersReg       .open((path + USERS_REGISTER_FILENAME).c_str())
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
               _dirVersions    .close() &&
               _dateIndex      .close() &&
               _dateLog        .close() &&
               _usersIndex     .close() &&   
               _usersReg       .close();
    debug("VersionManager close " + string((ret) ? "successfull" : "failed") + "\n");

    return ret;
}

bool VersionManager::buildVersion(std::list<FileVersion>& lstVersions, const string& a_Filename)
// generates a file named a_Filename with the required version
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

    
bool VersionManager::addFile(int repositoryVersion, const string& repositoryName, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type)
{
    if (!_isOpen)
        return false;

    long int offset;
    string key = repositoryName;

    for(int i = 1; i <= countComponents(a_Filename); ++i)
        key = key + "//" + getComponent(a_Filename,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el archivo
    int bloque = _fileIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el archivo esta en el indice
        FileVersion* ultimaVersion;
        _fileVersions.getLastVersion(&ultimaVersion, bloque);
        if (ultimaVersion->getTipo() != a_Type) {
            delete ultimaVersion;
            return false;
        }
          
        // analizo si la ultima version fue o no de borrado, si es asi, debo copiar todo el archivo
        if (ultimaVersion->getVersionType() == FileVersion::BORRADO) {
            delete ultimaVersion;   //elimino la ultima version

            std::ifstream is(a_Filename.c_str());
            if (!is) 
                return false;
            offset = _textContainer.append(is);
            is.close();
            if (offset == -1) 
                return false;

            return indexAFile(repositoryVersion, key, a_User, date, offset, a_Type, FileVersion::MODIFICACION,bloque);
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

                bool empty = isEmptyFile(tmpDiffFilename);
                if (!empty) {
                    std::ifstream is(tmpDiffFilename.c_str());
                    offset = _textContainer.append(is);
                    is.close();
                }
                // remove temporary files
                remove(tmpVersionFilename.c_str());
                remove(tmpDiffFilename.c_str());

                if (empty)
                    return false;
            }
            return indexAFile(repositoryVersion, key, a_User, date, offset, a_Type, FileVersion::MODIFICACION, bloque);
        }
        else if (a_Type == 'b') {
            std::ifstream is(a_Filename.c_str());
            if (!is) 
                return false;

            // check if versions differ
            FileVersion* ultimaVersion;
            _fileVersions.getLastVersion(&ultimaVersion, bloque);
            if (ultimaVersion->getVersionType() != FileVersion::BORRADO) { 
                string tmpVersion = randomFilename("tmp_");
                ofstream os(tmpVersion.c_str());   
                _binaryContainer.get(offset, os);
                os.close();
                if (!areDifferentFiles(tmpVersion, a_Filename))
                    return false;
            } 
            offset = _binaryContainer.append(is);
            is.close();
            if (offset == -1) 
                return false;

            if (bloque >= 0) { // el archivo esta en el indice
                return indexAFile(repositoryVersion, key, a_User, date, offset, a_Type, FileVersion::MODIFICACION,bloque);
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

       int nroNuevoBloque;
       if (!_fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, FileVersion::MODIFICACION, &nroNuevoBloque))
           return false;
       key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
       return (_fileIndex.insert(key.c_str(), nroNuevoBloque));
    }   

    return false; // never gets here
}

bool VersionManager::addDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date)
{
    if (!_isOpen)
        return false;

    DIR* dir;
    struct dirent* myDirent;
    DirectoryVersion* nuevaVersion;
    
    string key = repositoryName;

    for(int i = 1; i <= countComponents(a_Directoryname); ++i)
        key = key + "//" + getComponent(a_Directoryname,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el directorio
    int bloque = _dirIndex.searchFile(key.c_str());

    if (bloque >= 0) { 
        // el directorio esta en el indice
        DirectoryVersion* ultimaVersion;
        int lastVersion  = _dirVersions.getLastVersionNumber(bloque); //obtengo el numero de la ultima version

        _dirVersions.getVersion(lastVersion, bloque, &ultimaVersion);   //obtengo la ultima version

        // creo la nueva version
        nuevaVersion = new DirectoryVersion(repositoryVersion,a_User.c_str(),*date,DirectoryVersion::MODIFICACION);

        // debo cotejar los cambios que hubo en el directorio (eliminacion y agregado de nuevos archivos/directorios)
        // abro el directorio que quiero "versionar"
        if ((dir = opendir(a_Directoryname.c_str())) == NULL) {
            delete nuevaVersion;
            delete ultimaVersion;

            cerr << "El directorio: " << a_Directoryname << " no existe" << endl;
            return false;
        }

        // obtengo una lista con todos los nombres de los archivos/directorios pertenecientes al directorio que quiero agregar
        list<string> fileIncludedLst;
        while ((myDirent = readdir(dir)) != NULL) {
            string filename = myDirent->d_name;
            if ((filename.compare(".") != 0) && (filename.compare("..") != 0))
                fileIncludedLst.push_back(filename);
        }

        closedir(dir);

        list<File>* filesLst = ultimaVersion->getFilesList(); // lista con los archivos que pertencian a la ultima version
        list<File>::iterator it_oldFiles;     // iterador sobre la lista de los archivos/directorios de la ultima version
        list<string>::iterator it_newFiles;   // iterador sobre la lista de los archivos/directorios de la nueva version

        bool result = true;

        if (ultimaVersion->getType() != DirectoryVersion::BORRADO) {     
            // ahora debo cotejar si hay algun borrado y luego generar la version del archivo

            list<File> filesErased; // lista con los nombres de los archivos/directorios que fueron borrados

            for (it_oldFiles = filesLst->begin(); it_oldFiles != filesLst->end(); ++it_oldFiles) {
                string fname = it_oldFiles->getName();
                bool included = false;

                for (it_newFiles = fileIncludedLst.begin();it_newFiles != fileIncludedLst.end(); ++it_newFiles)
                    if (fname.compare(*it_newFiles) == 0)
                        included = true;

                if (!included) {
                    File* file = new File(it_oldFiles->getName(), it_oldFiles->getVersion(), it_oldFiles->getType());
                    filesErased.push_back(*file);
                }              
                else
                    nuevaVersion->addFile(it_oldFiles->getName(), it_oldFiles->getVersion(), it_oldFiles->getType());
            }

            list<File>::iterator it_erasedFiles;
            for (it_erasedFiles = filesErased.begin(); it_erasedFiles != filesErased.end(); ++it_erasedFiles) {
                string name = it_erasedFiles->getName();
                string fname = a_Directoryname + "/" + name;

                if (it_erasedFiles->getType() != 'd')
                    result = result && removeFile(repositoryVersion, repositoryName, fname, a_User, a_Date);
                else
                    result = result && removeDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
                
                if(result)
                  log(fname, a_User,toString<int>(repositoryVersion),a_Date);
            }
            filesErased.clear();      
        }

        delete ultimaVersion; // elimino la ultima version, ya no la voy a necesitar

        list<string>::iterator it_includedFiles;

        if (result)
            for (it_includedFiles = fileIncludedLst.begin(); it_includedFiles != fileIncludedLst.end(); ++it_includedFiles) { 
                // versiono todos los archivos/directorios que pertenecen al directorio
                string fname = a_Directoryname + "/" + *it_includedFiles;

                t_filetype ftype = getFiletype(fname);
                char type;

                if (nuevaVersion->searchFile(it_includedFiles->c_str())) {
                    bool modified = false;
                    if (ftype == DIRECTORY) {
                        type = 'd';
                        modified = addDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
                    }
                    else if (ftype == TEXT) {
                        type = 't';
                        modified = addFile(repositoryVersion, repositoryName, fname, a_User, a_Date, type);
                    }
                    else if (ftype == BINARY) {
                        type = 'b';
                        modified = addFile(repositoryVersion, repositoryName, fname, a_User, a_Date, type);
                    }
                    if (modified) {
                        nuevaVersion->update((*it_includedFiles).c_str(), repositoryVersion, type);
                        log(fname, a_User,toString<int>(repositoryVersion),a_Date);
                    }
                }
                else {                    
                    if (ftype == DIRECTORY) {
                        type = 'd';
                        result = result && addDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
                    }
                    else if (ftype == TEXT) {
                        type = 't';
                        result = result && addFile(repositoryVersion, repositoryName, fname, a_User, a_Date, type);
                    }
                    else if (ftype == BINARY) {
                        type = 'b';
                        result = result && addFile(repositoryVersion, repositoryName, fname, a_User, a_Date, type);
                    }
                    else result = false;

                    if (result) {  // agrego el archivo al directorio
                        nuevaVersion->addFile((*it_includedFiles).c_str(), repositoryVersion, type);
                        log(fname, a_User,toString<int>(repositoryVersion),a_Date);
                     }                                     
                }
            }

        if (result)
            result = indexADirectory(repositoryVersion, key, nuevaVersion, bloque);                             

        delete nuevaVersion;
        return result;
    }

    else {   // es la 1ra vez que se va a agregar el directorio, por lo tanto, no debo cotejar los cambios
          nuevaVersion = new DirectoryVersion(repositoryVersion,a_User.c_str(),*date,DirectoryVersion::MODIFICACION);

          if ((dir = opendir(a_Directoryname.c_str())) == NULL) {
              delete nuevaVersion;
              cerr << "El directorio: " << a_Directoryname << " no existe" << endl;
              return false;
          }

          // obtengo una lista con todos los nombres de los archivos/directorios pertenecientes al directorio que quiero agregar
          list<string> fileIncludedLst;
          while((myDirent = readdir(dir)) != NULL) {
              string filename = myDirent->d_name;
              if ((filename.compare(".") != 0) && (filename.compare("..") != 0))
                  fileIncludedLst.push_back(filename);          
          }
            
          closedir(dir);

          list<string>::iterator it_includedFiles;
          bool result = true;

          for (it_includedFiles = fileIncludedLst.begin(); it_includedFiles != fileIncludedLst.end(); ++it_includedFiles) {  
              // versiono todos los archivos/directorios que pertenecen al directorio
              string fname = a_Directoryname + "/" + *it_includedFiles;

              t_filetype ftype = getFiletype(fname);
              char type;
              if (ftype == DIRECTORY) {
                  type = 'd';
                  result = result && addDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
              }
              else if ((ftype == TEXT) || (ftype == BINARY)) {
                  type = (ftype == TEXT ? 't' : 'b');
                  result = result && addFile(repositoryVersion, repositoryName, fname, a_User, a_Date, type);
              }
              else result = false;

              if (result) {  // agrego el archivo al directorio
                  nuevaVersion->addFile((*it_includedFiles).c_str(), repositoryVersion, type);
                  log(fname, a_User,toString<int>(repositoryVersion), a_Date);
               }                  
          }

        if (result) {
            int nroNuevoBloque;
            _dirVersions.insertVersion(nuevaVersion, &nroNuevoBloque);
            key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
            result = (_dirIndex.insert(key.c_str(), nroNuevoBloque));
        }
        delete nuevaVersion;
        return result;      
    }
    return false;
}

bool VersionManager::add(int repositoryVersion, const string& repositoryName, const string& a_Target, const string& a_Username, time_t a_Date, t_filetype a_Type)
{
    int cantComponentesPath = countComponents(a_Target);
    string pathActual = repositoryName;   
    return addRec(a_Target,1, pathActual, repositoryName, repositoryVersion, cantComponentesPath, a_Username, a_Date, a_Type);
}

bool VersionManager::addRec(const string& a_Target, int componenteALeer, const  string& pathActual, const string& repositoryName, int repositoryVersion, int cantComponentesPath, const string& a_Username, time_t  a_Date, t_filetype a_Type)
{
    bool ret = false;
    string key;

    string finalPath = repositoryName;

    for (int i = 1; i <= countComponents(a_Target); ++i)
        finalPath = finalPath + "//" + getComponent(a_Target,i);

    if (pathActual.compare(finalPath) == 0) {   
        if ((a_Type == BINARY) || (a_Type == TEXT))      
            ret = addFile(repositoryVersion,repositoryName, a_Target, a_Username, a_Date, (a_Type == TEXT ? 't' : 'b'));
        else    
            ret = addDirectory(repositoryVersion,repositoryName, a_Target, a_Username,a_Date);

        if(ret)
            log(a_Target, a_Username, toString<int>(repositoryVersion), a_Date);
        return ret;
    }
    else {
        // identifico si hay alguna version del directorio actual (por el que voy siguiendo el camino hasta target)
        int bloque = _dirIndex.searchFile(pathActual.c_str());

        //obtengo la fecha          
        tm* date = localtime(&a_Date);      

        // creo una nueva version de directorio
        DirectoryVersion* nuevaVersion = new DirectoryVersion(repositoryVersion, a_Username.c_str(), *date, DirectoryVersion::MODIFICACION);

        // tomo la componente correspondiente                   
        string componente = getComponent(a_Target,componenteALeer);

        // defino el tipo de archivo que es la componente leida
        char tipoArchivo;
        string caminoRecorrido = pathActual + "//" + componente;
        if (caminoRecorrido.compare(finalPath) == 0) {
            if (a_Type == TEXT) 
                tipoArchivo = 't';
            else if (a_Type == BINARY) 
                tipoArchivo = 'b';
            else tipoArchivo = 'd';             
        }
        else tipoArchivo = 'd';
                    
        if (bloque >= 0) {       
            //obtengo el ultimo nro de version del directorio
            int lastVersionNumber = _dirVersions.getLastVersionNumber(bloque);
            DirectoryVersion* oldVersion;

            //trato de obtener la ultima version del directorio
            if (!_dirVersions.getVersion(lastVersionNumber,bloque,&oldVersion))
                ret = false;    
            else {
                ret = addRec(a_Target,componenteALeer + 1,pathActual + "//" + componente, repositoryName, repositoryVersion, cantComponentesPath, a_Username, a_Date, a_Type);

                if (ret) {
                    //aca tengo que cotejar los cambios que se realizan en la version                                       
                    list<File>* lst = oldVersion->getFilesList();
                    
                    list<File>::iterator it;

                    debug("comienzo a copiar la lista");
                    //copio los archivos que tenia en la version anterior a la nueva para luego actualizarlos
                    for (it = lst->begin();it != lst->end(); ++it)    
                        nuevaVersion->addFile(it->getName(), it->getVersion(), it->getType());

                    debug("termine de copiar la lista");
                    debug("elimino la version vieja");
                    //libero el puntero a la ultima version
                    delete oldVersion;

                    //actualizo la version de la componente leida
                    nuevaVersion->update(componente.c_str(), repositoryVersion, tipoArchivo);
                    debug("actualizo la version");

                    ret = indexADirectory(repositoryVersion, key, nuevaVersion, bloque);                    
                }                                       
            }           
        }
        else {   
            ret = addRec(a_Target, componenteALeer + 1, pathActual + "//" + componente, repositoryName, repositoryVersion, cantComponentesPath, a_Username,a_Date, a_Type);
            if (ret) {       
                int nuevoBloque;
                nuevaVersion->addFile(componente.c_str(), repositoryVersion, tipoArchivo);
                _dirVersions.insertVersion(nuevaVersion,&nuevoBloque);
               
                //genero la clave
                key = pathActual + zeroPad(repositoryVersion,VERSION_DIGITS);
                ret = _dirIndex.insert(key.c_str(),nuevoBloque);                
            }           
        }
        if(ret)
            log(pathActual, a_Username,toString<int>(repositoryVersion), a_Date);
        delete nuevaVersion;
        return ret;     
    }

    return ret; // never gets here
}

bool VersionManager::create()
{
    if (_isOpen)
        return true;

    debug("creating VersionManager for Repositorio '" + _repository + "' in Almacen '" + _almacen + "'\n");
    string path = _almacen + "//" + _repository + "//";
    _isOpen = (_fileIndex      .create((path + FILE_INDEX_FILENAME)     .c_str()) &&
               _fileVersions   .create((path + FILE_VERSION_FILENAME)   .c_str()) &&
               _textContainer  .create((path + TXT_DIFFS_FILENAME)      .c_str()) &&
               _binaryContainer.create((path + BIN_DIFFS_FILENAME)      .c_str()) &&
               _dirIndex       .create((path + DIR_INDEX_FILENAME)      .c_str()) &&
               _dirVersions    .create((path + DIR_VERSION_FILENAME)    .c_str()) &&
               _dateIndex      .create((path + DATE_INDEX_FILENAME)     .c_str()) &&
               _dateLog        .create((path + DATE_LOG_FILENAME)       .c_str()) &&
               _usersIndex     .create((path + USERS_INDEX_FILENAME)    .c_str()) &&
               _usersReg       .create((path + USERS_REGISTER_FILENAME) .c_str())
              );
   
    debug("VersionManager creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool VersionManager::getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version,const string& repositoryName)
{
    if (!_isOpen)
        return false;

    FileVersion* versionBuscada;
    int bloque;

     if (!getFileVersionAndBlock(&bloque, &versionBuscada, a_Filename, a_Version))
        return false;   

     int RepNameEnd = a_Filename.find_first_not_of(repositoryName + "//");
     string path = a_Filename;
     path.erase(0,RepNameEnd);

     if (versionBuscada->getVersionType() == FileVersion::BORRADO) { // si la version buscada es una version de borrado entonces no hago nada
        delete versionBuscada;
        return false;
     }

    char ftype = versionBuscada->getTipo();
    ifstream fileToCheck;
    char option;
    if (ftype == 't') {
        int original = versionBuscada->getOriginal();
        // busco en el indice a ver si esta el archivo
        list<FileVersion> versionsList;
        int final = versionBuscada->getNroVersion();
        delete versionBuscada;
        if (_fileVersions.getVersionFrom(original, final, bloque, versionsList)) {
            //chequeo si el archivo ya existe o no. Si ya existe, debo preguntar si lo reescribo o no.
            fileToCheck.open((a_TargetDir + "//" + path).c_str());
            fileToCheck.close();
            if(fileToCheck.fail())  //si el archivo no existia lo escribo
                return (buildVersion(versionsList, a_TargetDir + "//" + path));
            else {
                do {
                    // TODO: desde aca interaccion con usuario!?
                    cerr << "El archivo: " << a_TargetDir << "/" << path << " ya existe. Desea sobreescribirlo? S/N" << endl;
                    cin >> option;
                    cout << endl;
                    option = toupper(option);
                } while( (option != 'N') && (option != 'S') );

                if (option == 'S')
                    return (buildVersion(versionsList, a_TargetDir + "//" + path));
                else
                    return true;
            }
        }
        else
            return false;
    }
    else if (ftype == 'b') {
        int offset = versionBuscada->getOffset();
        delete versionBuscada;
        //chequeo si el archivo ya existe o no. Si ya existe, debo preguntar si lo reescribo o no.
        fileToCheck.open((a_TargetDir + "//" + path).c_str());
        fileToCheck.close();
        if (fileToCheck.fail()) {  //si el archivo no existia lo escribo
            std::ofstream os((a_TargetDir + "//" + path).c_str());
            if (!os.is_open())
                return false;
            if (!_binaryContainer.get(offset, os))
                return false;
            os.close();
            return true;
        }
        else {  // si el archivo ya existia pregunto si hay que sobreescribirlo
            do {
                // TODO: desde aca interaccion con usuario!?
                cerr << "El archivo: " << a_TargetDir << "/" << path << " ya existe. Desea sobreescribirlo? S/N" << endl;
                cin >> option;
                cout << endl;
                option = toupper(option);
            } while((option != 'N') && (option != 'S'));

            if (option == 'S') {   
                // sobreescribo el archivo
                std::ofstream os((a_TargetDir + "//" + path).c_str());
                if (!os.is_open())
                    return false;
                if (!_binaryContainer.get(offset, os))
                    return false;
                os.close();
                return true;
            }
            else
                return true;
        }
    }
    return false; // never gets here
}

bool VersionManager::getDirectory(const string& a_TargetDir,const string& pathToFile, const string& a_Path, const string& a_DirName, const string& a_Version,const string& repositoryName)
{
        string fullDirName = a_Path + "//" + a_DirName;
        if (!_isOpen)
                return false;

        DirectoryVersion* versionBuscada;
        if (!getDirVersion(&versionBuscada,fullDirName, a_Version))
            return false;

        if (versionBuscada->getType() == DirectoryVersion::BORRADO) {   
            //no puedo recuperar una version de borrado
            delete versionBuscada;
            return false;
        }

        bool ret = true;    //el valor que voy a devolver

        //obtengo la lista de archivos/directorios del directorio que quiero obtener
        list<File>* filesLst = versionBuscada->getFilesList();  
        list<File>::iterator it;

        //si el directorio que voy a obtener no esta creado en el destino --> lo creo
        bool creado = false;

        string currentDir = get_current_dir_name();

        if (chdir((a_TargetDir + "//" + pathToFile + "//" + a_DirName).c_str()) != 0) {
            if (mkdir((a_TargetDir + "//" + pathToFile + "//" + a_DirName).c_str(),0755) != 0) {
                cout << "Imposible crear: " << a_TargetDir << "//" << pathToFile << "//" << a_DirName << endl;
                delete versionBuscada;               
                return false;
            }   
            else {
                creado = true;
                chdir(currentDir.c_str());
            }
        }

        for(it = filesLst->begin(); it != filesLst->end(); ++it) {
            string FName = it->getName();
            string version_number = toString<int>(it->getVersion());

            if (it->getType() != 'd')
                ret = ret && getFile(a_TargetDir, fullDirName + "//" + FName, version_number, repositoryName);
            else
                ret = ret && getDirectory(a_TargetDir,pathToFile + "//" + a_DirName, fullDirName, FName, version_number, repositoryName);
        }

        // si fallo la recuperacion y cree un directorio --> lo elimino
        if ((ret == false) && (creado == true))
            remove((a_TargetDir + "//" + pathToFile + "//" + a_DirName).c_str());

        delete versionBuscada;
        chdir(currentDir.c_str());
        return ret;
}

bool VersionManager::get(const string& a_Version, const string& a_Target,const string& repositoryName, const string& a_TargetDestiny)
{
    if (!_isOpen)
        return false;

    // voy a tener que la version del directorio que contiene a ese archivo/directorio para cotejar que existe la version que estoy buscando
    DirectoryVersion* versionDirectorioContenedor;

    // tengo que armar el path del directorio contenerdor al archivo/directorio que estoy buscando
    // para todos los casos el directorio raiz es el repositorio por lo tanto todos los paths de los archivos empiezan con:
    // "nombre_repositorio//" y van seguidos del path correspondiente...

    string searchingPath = repositoryName;

    // voy agregando componente a componente para saber donde debo buscar
    for(int i = 1; i < countComponents(a_Target); ++i)
        searchingPath = searchingPath + "//" + getComponent(a_Target, i);

    // obtengo la version del directorio que contiene al archivo/directorio objetivo con el mismo nro de version que deseo que tenga el 
    // archivo/directorio si es que voy a querer una version en particular o la ultima version del directorio que contiene al archivo/ 
    // directorio si esa que no especifique ninguna
    if(!getDirVersion(&versionDirectorioContenedor, searchingPath,a_Version))
        return false;

    if (a_Target != "") {
        if (versionDirectorioContenedor->getType() == DirectoryVersion::BORRADO) {
            delete versionDirectorioContenedor;
            return false;
        }

        string filename = getComponent(a_Target,countComponents(a_Target));

        File* file;
        if (!versionDirectorioContenedor->searchFile(filename.c_str(),&file)) {   
            delete versionDirectorioContenedor;
            return false;
        }

        // una vez obtenida la version del directorio voy a tener que armar la estructura de directorios que contienen al archivo/directorio
        // objetivo dentro del directorio destino para luego "bajar" la version solicitada del archivo/directorio objetivo

        // aca voy a guardar el path actual para luego volver al directorio de trabajo
        string currentDirectory = get_current_dir_name();

        // empiezo a armar la estructura
        int existentes  = 0;

        // trato de cambiar de directorio al directorio destino para chequear que existe
        if(chdir(a_TargetDestiny.c_str()) != 0) {
            // si el directorio destion no existe -> vuelvo al directorio de trabajo actual y elimino las referencias que tengo en memoria
            chdir(currentDirectory.c_str());
            delete versionDirectorioContenedor;
            cout << "El directorio elegido como destino no existe." << endl;
            return false;       
        }

        chdir(currentDirectory.c_str()); //vuelvo al directorio de trabajo

        //ahora tengo que armar la estructura de directorios a donde va a ir a parar el archivo/directorio objetivo
        int RepNameEnd = searchingPath.find_first_not_of(repositoryName + "//");

        string path = searchingPath;
        path.erase(0,RepNameEnd);
        string pathAuxiliar = a_TargetDestiny;
        int components = countComponents(path);
        if(path.length() > 0) {
            //empiezo a armar la estructura de directorios
            int components = countComponents(path);

            debug("creando directorios \n");

            for(int j = 1; j <= components; ++j) {
                pathAuxiliar = pathAuxiliar + "/" + getComponent(path,j);

                //si no existe el directorio lo creo
                if(chdir(pathAuxiliar.c_str()) != 0) {                       
                    if(mkdir(pathAuxiliar.c_str(),0755) != 0)
                        cout << "error al crear: " << pathAuxiliar << endl;
                }
                else {
                    existentes++;
                    chdir(currentDirectory.c_str());
                }
            }       
        }

        bool ret;                   

        if(file->getType() != 'd')
            ret = getFile(a_TargetDestiny,searchingPath + "//" + filename ,a_Version,repositoryName);
        else
            ret = getDirectory(a_TargetDestiny,path , searchingPath, filename, a_Version,repositoryName);

        if(ret == false) {
            //elimino los directorios que cree para albergar el archivo/directorio que queria obtener
            for(int j = components; j > existentes; --j) {
                remove(pathAuxiliar.c_str());
                int index = pathAuxiliar.find_last_of("/");
                pathAuxiliar.erase(index);          
            }
        }

        delete versionDirectorioContenedor;
        return ret;
    }
    else {
        bool ret = true;
        list<File>* filesLst = versionDirectorioContenedor->getFilesList();
        list<File>::iterator it_files;
        string fname;
        for(it_files = filesLst->begin(); it_files != filesLst->end(); ++it_files) {
            fname = it_files->getName();
            if(it_files->getType() == 'd')
                ret = ret && getDirectory(a_TargetDestiny, "" , searchingPath, fname, a_Version, repositoryName);
            else
                ret == ret && getFile(a_TargetDestiny,searchingPath + "//" + fname, a_Version, repositoryName);
        }

        if (!ret) {
            for(it_files = filesLst->begin(); it_files != filesLst->end(); ++it_files) {
                fname = it_files->getName();
                remove((a_TargetDestiny + "/" + fname).c_str());
            }
        }
        delete versionDirectorioContenedor;
        return ret; 
    }
}

bool VersionManager::removeFileOrDirectory(int repositoryVersion, const string& repositoryName, const string& pathActual, const string& a_User, time_t a_Date)
{
    // TODO
    return false;
}

bool VersionManager::removeFile(int repositoryVersion, const string& repositoryName, const string& a_Filename, const string& a_User, time_t a_Date)
{
    if (!_isOpen)
        return false;

    string key = repositoryName;
    for(int i = 1; i <= countComponents(a_Filename); ++i)
        key = key + "//" + getComponent(a_Filename,i);

    debug("clave a borrar: "+key+"\n");

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el archivo
    int bloque = _fileIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el archivo esta en el indice, entonces, se puede borrar
        FileVersion* ultimaVersion;
        _fileVersions.getLastVersion(&ultimaVersion, bloque);
        if (ultimaVersion->getVersionType() == FileVersion::BORRADO) { 
            // si ya esta borrado no puedo volver a borrarlo             
            delete ultimaVersion;
            return false;
        }

        debug("obtengo el fileType \n");
        char tipoArchivo = ultimaVersion->getTipo();

        delete ultimaVersion;
        log(a_Filename, a_User,toString<int>(repositoryVersion), a_Date);
        return indexAFile(repositoryVersion, key, a_User, date, -1, tipoArchivo, FileVersion::BORRADO, bloque);        
    }
    debug("bloque < 0 \n"); 
    return false;
}

bool VersionManager::removeDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date)
{
    debug("ingreso en removeDirectory \n");
    if (!_isOpen)
        return false;

    DirectoryVersion* nuevaVersion;
    string key = repositoryName;

    for(int i = 1; i <= countComponents(a_Directoryname); ++i)
        key = key + "//" + getComponent(a_Directoryname,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el directorio
    int bloque = _dirIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el directorio esta en el indice
        DirectoryVersion* ultimaVersion;
        int lastVersion  = _dirVersions.getLastVersionNumber(bloque); //obtengo el numero de la ultima version

        _dirVersions.getVersion(lastVersion, bloque, &ultimaVersion);   //obtengo la ultima version

        debug("bloque >= 0 \n");
        if (ultimaVersion->getType() == DirectoryVersion::BORRADO) {
            //no puedo volver a borrar algo ya borrado
            delete ultimaVersion;
            debug("ultima version fue borrado \n");
            return false;
        }

        bool result = true;
        //creo la nueva version
        nuevaVersion = new DirectoryVersion(repositoryVersion,a_User.c_str(),*date,DirectoryVersion::BORRADO);

        list<File>* filesLst = ultimaVersion->getFilesList();

        list<File>::iterator it_filesToRemove;
        // a cada archivo/directorio de la version anterior les debo hacer un borrado
        for(it_filesToRemove = filesLst->begin();it_filesToRemove != filesLst->end(); ++it_filesToRemove) {

            string fname = it_filesToRemove->getName();
            string fullPath = a_Directoryname + "/" + fname;

            debug("archivo a eliminar: "+fullPath+"\n");
            if(it_filesToRemove->getType() == 'd') {
                result = result && removeDirectory(repositoryVersion,repositoryName,fullPath,a_User,a_Date);
                debug("entro en removeDirectory \n");
            }
            else {
                result = result && removeFile(repositoryVersion,repositoryName,fullPath,a_User,a_Date);
                debug("entro en removeFile \n");
            }
            if(!result)
                debug("el borrado falla en: "+fullPath+"\n");           
        }

        if (!result) {
            debug("fallo el borrado \n");
            delete ultimaVersion;
            delete nuevaVersion;

            return false;
        }

        result = indexADirectory(repositoryVersion, key, nuevaVersion, bloque);

        if(result)
            log(a_Directoryname, a_User,toString<int>(repositoryVersion), a_Date);
        delete nuevaVersion;
        return result;
    }

    // si llega aca es porque no habia una version previa del directorio, por lo tanto, no se puede realizar el borrado        
    return false;
}

bool VersionManager::getFileVersionAndBlock(int* bloque, FileVersion** versionBuscada, const string& a_Filename, const string& a_Version)
{
    if (a_Version != "") {
        int version = fromString<int>(a_Version);
        *bloque = _fileIndex.searchFileAndVersion(a_Filename.c_str(), version);
        if (!_fileVersions.searchVersion(versionBuscada, version, *bloque)) {
            *bloque = _fileIndex.searchFile(a_Filename.c_str());
            if (!_fileVersions.getLastVersion(versionBuscada, *bloque))
                return false;
            else if ((*versionBuscada)->getNroVersion() > version) {
                return false;
            }
        }        
    }
    else {
        *bloque = _fileIndex.searchFile(a_Filename.c_str());
        if (!_fileVersions.getLastVersion(versionBuscada, *bloque))
           return false; 
    }
    return true;
}

bool VersionManager::getDirVersion(DirectoryVersion** versionBuscada, const string& a_Dirname, const string& a_Version)
{
    int bloque;
    if (a_Version != "") {
        int version = fromString<int>(a_Version);
        bloque = _dirIndex.searchFileAndVersion(a_Dirname.c_str(), version);
        if (bloque < 0) 
            return false;
        if (!_dirVersions.searchVersion(versionBuscada, version, bloque)) {
            bloque = _dirIndex.searchFile(a_Dirname.c_str());
            if (bloque < 0) 
                return false;
            version = _dirVersions.getLastVersionNumber(bloque);
            if (!_dirVersions.getVersion(version, bloque, versionBuscada))
                return false;
            else if((*versionBuscada)->getNroVersion() > version) {
                delete (*versionBuscada);
                return false;
            }
        }        
    }
    else {
        bloque = _dirIndex.searchFile(a_Dirname.c_str());
        if(bloque < 0) return false;
        int lastVersion = _dirVersions.getLastVersionNumber(bloque);
        if (!_dirVersions.getVersion(lastVersion, bloque, versionBuscada))
            return false; 
    }
    return true;
}

bool VersionManager::indexAFile(int repositoryVersion, const string& key, const string& a_User, tm* date, int offset, char a_Type, FileVersion::t_versionType a_VersionType, int bloque)
{
    int nroNuevoBloque;
    string newKey;

    FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, a_VersionType, bloque, &nroNuevoBloque);
    switch (status) {
        case FileVersionsFile::OK :
            return true;
            break;
        case FileVersionsFile::OVERFLOW :
            // tengo que generar la clave a partir de a_File y repositoryVersion
            newKey = key + zeroPad(repositoryVersion, VERSION_DIGITS);
            return _fileIndex.insert(newKey.c_str(), nroNuevoBloque);
            break;
        default:
            return false;
            break;
    }
    return false;
}

bool VersionManager::indexADirectory(int repositoryVersion, const string& key, DirectoryVersion* nuevaVersion,int bloque)
{
    int nroNuevoBloque;
    string newKey;

    DirectoryVersionsFile::t_status status = _dirVersions.insertVersion(nuevaVersion, bloque, &nroNuevoBloque);
    switch (status) {
       case DirectoryVersionsFile::OK :
          return true;
          break;
       case DirectoryVersionsFile::OVERFLOW :
          // tengo que generar la clave a partir de a_Directoryname y repositoryVersion
            newKey = key + zeroPad(repositoryVersion, VERSION_DIGITS);
          return _dirIndex.insert(key.c_str(), nroNuevoBloque);
            break;

        default:
            return false;
            break;
    }
    return false;
}

bool VersionManager::buildTextVersion(int bloque, FileVersion* versionBuscada, const string& a_Filename)
{
    int original = versionBuscada->getOriginal();
    int final    = versionBuscada->getNroVersion();

    list<FileVersion> versionsList;
    if (_fileVersions.getVersionFrom(original, final, bloque, versionsList))
        return (buildVersion(versionsList, a_Filename));

    return false;
}

bool VersionManager::getFileDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Filename)
{
    if (!_isOpen)
        return false;

    FileVersion* versionBuscadaA;
    int bloqueA;

    FileVersion* versionBuscadaB;
    int bloqueB;

     
    if (!getFileVersionAndBlock(&bloqueA, &versionBuscadaA, a_Filename, a_VersionA))
        return false;

    if (!getFileVersionAndBlock(&bloqueB, &versionBuscadaB, a_Filename, a_VersionB)) {
        delete versionBuscadaA;
        return false;
    }

    char ftypeA = versionBuscadaA->getTipo();
    char ftypeB = versionBuscadaB->getTipo();
    if (ftypeA != ftypeB)
        return false;

    string tmpA    = randomFilename("tmp_");
    string tmpB    = randomFilename("tmp_");
    string tmpDiff = randomFilename("tmp_");
    if (ftypeA == 't') {
        bool ret = buildTextVersion(bloqueA, versionBuscadaA, tmpA);
        ret = ret && buildTextVersion(bloqueB, versionBuscadaB, tmpB);
        delete versionBuscadaA;
        delete versionBuscadaB;

        if (ret) {
            // perform diff between tmpA and tmpB
            string cmd = "diff " + tmpA + " " + tmpB + " > " + tmpDiff;
            if (system(cmd.c_str()) == -1)
                return false;

            is.open(tmpDiff.c_str());

            // remove temporary files
            remove(tmpA.c_str());
            remove(tmpB.c_str());
            remove(tmpDiff.c_str());
        }
        return ret;
    }
    else if (ftypeA == 'b') {
        // obtener ambos archivos y compararlos
        int offsetA = versionBuscadaA->getOffset();
        int offsetB = versionBuscadaB->getOffset();
        delete versionBuscadaA;
        delete versionBuscadaB;

        std::ofstream osA(tmpA.c_str());
        std::ofstream osB(tmpB.c_str());
        if (!osA.is_open() || !osB.is_open())
            return false;
        if (!_binaryContainer.get(offsetA, osA) ||
            !_binaryContainer.get(offsetB, osB))
            return false;
        osA.close();
        osB.close();

        string cmd = "cmp " + tmpA + " " + tmpB + "| wc -c > " + tmpDiff;
        if (system(cmd.c_str()) == -1)
            return false;

        is.open(tmpDiff.c_str());
        int different;
        is >> different;
        is.close();
        std::ofstream os(tmpDiff.c_str());
        if (different == 0)
            os << "Ambas versiones son identicas.\n";
        else
            os << "Las versiones difieren.\n";
        os.close();
        is.open(tmpDiff.c_str());

        // remove temporary files
        remove(tmpA.c_str());
        remove(tmpB.c_str());
        remove(tmpDiff.c_str());

        return true;
    }
    return false;
}


bool VersionManager::getDirectoryDiff(const string& a_DirName, const string& a_VersionA, const string& a_VersionB, int tabs)
{
    string espacio;
    for(int i = 1; i <= tabs; ++i)
        espacio = espacio + "\t";

    string dirname = a_DirName;
    int index = dirname.find_last_of("//",dirname.length());
    dirname.erase(0,index + 1);

    bool ret = true;

    DirectoryVersion* versionDirectorioA;
    DirectoryVersion* versionDirectorioB;

    if(!getDirVersion(&versionDirectorioA, a_DirName, a_VersionA))
        return false;

    if(!getDirVersion(&versionDirectorioB, a_DirName, a_VersionB)) {
        delete versionDirectorioA;
        return false;
    }

    list<File>* lstA = versionDirectorioA->getFilesList();
    list<File>* lstB = versionDirectorioB->getFilesList();

    list<File>::iterator it_listA;
    list<File>::iterator it_listB;

    string fname;

    cout << espacio << dirname << endl;

    for(it_listA = lstA->begin(); it_listA != lstA->end(); ++it_listA) {
        bool found = false;
        for (it_listB = lstB->begin(); it_listB != lstB->end(); ++it_listB) {
            if (strcmp(it_listA->getName(),it_listB->getName()) == 0) {
                found = true;
                fname = it_listA->getName();

                if (it_listA->getVersion() != it_listB->getVersion()) {

                    if((it_listA->getType() != 'd') && (it_listB->getType() != 'd')) {
                        FileVersion* file_versionA;
                        FileVersion* file_versionB;                                 

                        int bloqueA;
                        int bloqueB;

                        if(!getFileVersionAndBlock(&bloqueA, &file_versionA, a_DirName + "//" + fname, a_VersionA))
                            break;              

                        if(!getFileVersionAndBlock(&bloqueB, &file_versionB, a_DirName + "//" + fname, a_VersionB)) {
                            delete file_versionA;
                            break;
                        }                                                                                   

                        cout << espacio + "   " << fname << " version: " << file_versionA->getNroVersion() << ", tipo: " << file_versionA->getTipo()
                             << ", usuario: " << file_versionA->getUser() << "   ";
                        cout << espacio + "   " << fname << " version: " << file_versionB->getNroVersion() << ", tipo: " << file_versionB->getTipo()
                             << ", usuario: " << file_versionB->getUser() << endl;

                        delete file_versionA;
                        delete file_versionB;
                    }

                    else if ((it_listA->getType() == 'd') && (it_listB->getType() == 'd'))
                        ret = ret && getDirectoryDiff(a_DirName + "//" + fname, a_VersionA, a_VersionB, tabs + 1);

                    else if ((it_listA->getType() == 'd') && (it_listB->getType() != 'd')) {
                        DirectoryVersion* dir_version;
                        FileVersion* file_version;
                        int bloque;

                        if (!getDirVersion(&dir_version, a_DirName + "//" + fname, a_VersionA))
                            break;

                        if (!getFileVersionAndBlock(&bloque, &file_version, a_DirName + "//" + fname, a_VersionB)) {
                            delete dir_version;
                            break;
                        }

                        cout << espacio + "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                             << ", usuario: " << dir_version->getUser() << "   ";
                        cout << espacio + "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                             << ", usuario: " << file_version->getUser() << endl;

                        delete file_version;
                        delete dir_version;
                    }
                    else {
                        DirectoryVersion* dir_version;
                        FileVersion* file_version;
                        int bloque;

                        if (!getDirVersion(&dir_version, a_DirName + "//" + fname, a_VersionB))
                            break;

                        if (!getFileVersionAndBlock(&bloque, &file_version, a_DirName + "//" + fname, a_VersionA)) {
                            delete file_version;
                            break;
                        }

                        cout << espacio + "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                             << ", usuario: " << file_version->getUser() << "   ";
                        cout << espacio + "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                             << ", usuario: " << dir_version->getUser() << endl;

                        delete file_version;
                        delete dir_version;
                    }                   
                }
                else {
                    DirectoryVersion* dir_version;
                    FileVersion* file_version;
                    int bloque;

                    if (it_listA->getType() == 'd') {
                        if(!getDirVersion(&dir_version, a_DirName + "//" + fname, a_VersionA))
                            break;

                        cout << espacio + "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                             << ", usuario: " << dir_version->getUser() << endl;

                        delete dir_version;         
                    }                       
                    else {
                        if(!getFileVersionAndBlock(&bloque, &file_version, a_DirName + "//" + fname, a_VersionB))
                            break;
                        cout << espacio + "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                             << ", usuario: " << file_version->getUser() << endl;

                        delete file_version;                    
                    }
                }
            }
        }

        if (!found) {   
            if(it_listA->getType() == 'd') {
                DirectoryVersion* dir_version;
                if(!getDirVersion(&dir_version, a_DirName + "//" + fname, a_VersionA))
                    break;

                cout << espacio + "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                     << ", usuario: " << dir_version->getUser() << "-> borrado" << endl;

                delete dir_version;                 
            }
            else {
                FileVersion* file_version;
                int bloque;

                if (!getFileVersionAndBlock(&bloque, &file_version, a_DirName + "//" + fname, a_VersionA))
                    break;

                cout << espacio + "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                     << ", usuario: " << file_version->getUser() << "-> borrado" << endl;    

                delete file_version;
            }
        }           
    }

    for (it_listB = lstB->begin(); it_listB != lstB->end(); ++it_listB) {
        bool found = false;
        for(it_listA = lstA->begin(); it_listA != lstA->end(); ++it_listA) {
            if(strcmp(it_listA->getName(),it_listB->getName()) == 0)
                found = true;   
        }

        if (!found) {
            fname = it_listB->getName();

            if (it_listB->getType() == 'd') {
                DirectoryVersion* dir_version;
                if(!getDirVersion(&dir_version, a_DirName + "//" + fname, a_VersionB))
                    break;

                cout << espacio + "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                     << ", usuario: " << dir_version->getUser() << "-> agregado" << endl;

                showAddedDirectory(dir_version, a_DirName + "//" + fname, tabs + 1);
                delete dir_version;                 
            }
            else {
                FileVersion* file_version;
                int bloque;

                if(!getFileVersionAndBlock(&bloque, &file_version, a_DirName + "//" + fname, a_VersionB))
                    break;

                cout << espacio + "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                     << ", usuario: " << file_version->getUser() << "-> agregado" << endl;   

                delete file_version;
            }
        }
    }

    delete versionDirectorioA;
    delete versionDirectorioB;
    return ret; 
}

bool VersionManager::getDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Target, const string& repositoryName)
{
    if (!_isOpen)
        return false;

    string searchingPath = repositoryName;

    // voy agregando componente a componente para saber donde debo buscar
    for (int i = 1; i < countComponents(a_Target); ++i)
        searchingPath = searchingPath + "//" + getComponent(a_Target,i);

    DirectoryVersion* versionDirectorioA;
    DirectoryVersion* versionDirectorioB;

    if (!getDirVersion(&versionDirectorioA, searchingPath,a_VersionA))
        return false;

    if (!getDirVersion(&versionDirectorioB,searchingPath,a_VersionB)) {
        delete versionDirectorioA;
        return false;
    }

    bool ret = false;

    if (a_Target != "") {
        if (versionDirectorioA->getType() == DirectoryVersion::BORRADO) {
            delete versionDirectorioA;
            delete versionDirectorioB;
            cout << "No existe la version: " << a_VersionA << " de " << searchingPath << endl;
            return false;
        }

        if (versionDirectorioB->getType() == DirectoryVersion::BORRADO) {
            delete versionDirectorioA;
            delete versionDirectorioB;
            cout << "No existe la version: " << a_VersionB << " de " << searchingPath << endl;
            return false;
        }

        string filename = getComponent(a_Target,countComponents(a_Target));

        File* fileA;
        if(!versionDirectorioA->searchFile(filename.c_str(),&fileA)) {   
            delete versionDirectorioA;
            delete versionDirectorioB;
            cout << "No existe la version: " << a_VersionA << " de " << searchingPath << "//" << filename << endl;
            return false;
        }

        File* fileB;
        if(!versionDirectorioB->searchFile(filename.c_str(),&fileB)) {
            delete fileA;   
            delete versionDirectorioA;
            delete versionDirectorioB;
            cout << "No existe la version: " << a_VersionB << " de " << searchingPath << "//" << filename << endl;
            return false;
        }

        if(fileA->getType() != fileB->getType()) {
            delete fileA;
            delete fileB;   
            delete versionDirectorioA;
            delete versionDirectorioB;
            cout << "Las versiones " << a_VersionA << " y " << a_VersionB << " de " << filename << " son tipos de archivos diferentes" << endl;
            return false;
        }

        if(fileA->getType() != 'd')
            ret = getFileDiff(is, a_VersionA, a_VersionB,searchingPath + "//" + filename);

        else
            ret = getDirectoryDiff(searchingPath + "//" + filename, a_VersionA, a_VersionB,0);

        delete versionDirectorioA;
        delete versionDirectorioB;
        delete fileA;
        delete fileB;           

        return ret;
    }
    else {
        list<File>* lstA = versionDirectorioA->getFilesList();
        list<File>* lstB = versionDirectorioB->getFilesList();

        list<File>::iterator it_listA;
        list<File>::iterator it_listB;

        string fname;

        ret = true;
        cout << repositoryName << endl;

        for (it_listA = lstA->begin(); it_listA != lstA->end(); ++it_listA) {
            bool found = false;
            for(it_listB = lstB->begin(); it_listB != lstB->end(); ++it_listB) {
                if(strcmp(it_listA->getName(),it_listB->getName()) == 0) {
                    found = true;
                    fname = it_listA->getName();

                    if(it_listA->getVersion() != it_listB->getVersion()) {

                        if((it_listA->getType() != 'd') && (it_listB->getType() != 'd')) {
                            FileVersion* file_versionA;
                            FileVersion* file_versionB;                                 

                            int bloqueA;
                            int bloqueB;

                            if(!getFileVersionAndBlock(&bloqueA, &file_versionA, searchingPath + "//" + fname, a_VersionA))
                                break;              

                            if(!getFileVersionAndBlock(&bloqueB, &file_versionB, searchingPath + "//" + fname, a_VersionB)) {
                                delete file_versionA;
                                break;
                            }                                                                                   

                            cout << "   " << fname << " version: " << file_versionA->getNroVersion() << ", tipo: " << file_versionA->getTipo()
                                 << ", usuario: " << file_versionA->getUser() << "   ";
                            cout << "   " << fname << " version: " << file_versionB->getNroVersion() << ", tipo: " << file_versionB->getTipo()
                                 << ", usuario: " << file_versionB->getUser() << endl;

                            delete file_versionA;
                            delete file_versionB;
                        }

                        else if( (it_listA->getType() == 'd') && (it_listB->getType() == 'd') )
                            ret = ret && getDirectoryDiff(searchingPath + "//" + fname, a_VersionA, a_VersionB,1);

                        else if ( (it_listA->getType() == 'd') && (it_listB->getType() != 'd') ) {
                            DirectoryVersion* dir_version;
                            FileVersion* file_version;
                            int bloque;

                            if(!getDirVersion(&dir_version, searchingPath + "//" + fname, a_VersionA))
                                break;

                            if(!getFileVersionAndBlock(&bloque, &file_version, searchingPath + "//" + fname, a_VersionB)) {
                                delete dir_version;
                                break;
                            }

                            cout << "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                                 << ", usuario: " << dir_version->getUser() << "   ";
                            cout << "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                                 << ", usuario: " << file_version->getUser() << endl;

                            delete file_version;
                            delete dir_version;
                        }
                        else {
                            DirectoryVersion* dir_version;
                            FileVersion* file_version;
                            int bloque;

                            if (!getDirVersion(&dir_version, searchingPath + "//" + fname, a_VersionB))
                                break;

                            if (!getFileVersionAndBlock(&bloque, &file_version, searchingPath + "//" + fname, a_VersionA)) {
                                delete file_version;
                                break;
                            }

                            cout << "   " << fname << ", version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                                 << " usuario: " << file_version->getUser() << "   ";
                            cout << "   " << fname << ", version: " << dir_version->getNroVersion() << ", tipo: d"
                                 << " usuario: " << dir_version->getUser() << endl;

                            delete file_version;
                            delete dir_version;
                        }                   
                    }
                    else {
                        DirectoryVersion* dir_version;
                        FileVersion* file_version;
                        int bloque;

                        if (it_listA->getType() == 'd') {
                            if(!getDirVersion(&dir_version, searchingPath + "//" + fname, a_VersionA))
                                break;

                            cout << "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                                 << ", usuario: " << dir_version->getUser() << endl;

                            delete dir_version;
                        }                       
                        else {
                            if(!getFileVersionAndBlock(&bloque, &file_version, searchingPath + "//" + fname, a_VersionB))
                                break;
                            cout << "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                                 << ", usuario: " << file_version->getUser() << endl;

                            delete file_version;                    
                        }
                    }
                }
            }

            if (!found) {   
                if(it_listA->getType() == 'd') {
                    DirectoryVersion* dir_version;
                    if(!getDirVersion(&dir_version, searchingPath + "//" + fname, a_VersionA))
                        break;

                    cout << "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                         << ", usuario: " << dir_version->getUser() << "-> borrado" << endl;

                    delete dir_version;                 
                }
                else {
                    FileVersion* file_version;
                    int bloque;

                    if(!getFileVersionAndBlock(&bloque, &file_version, searchingPath + "//" + fname, a_VersionA))
                        break;

                    cout << "   " << fname << " version: " << file_version->getNroVersion() << " tipo: " << file_version->getTipo() << 
                        " usuario: " << file_version->getUser() << " borrado" << endl;    

                    delete file_version;
                }
            }           
        }

        for (it_listB = lstB->begin(); it_listB != lstB->end(); ++it_listB) {
            bool found = false;
            for(it_listA = lstA->begin(); it_listA != lstA->end(); ++it_listA) {
                if(strcmp(it_listA->getName(),it_listB->getName()) == 0)
                    found = true;   
            }

            if (!found) {
                fname = it_listB->getName();

                if(it_listB->getType() == 'd') {
                    DirectoryVersion* dir_version;
                    if(!getDirVersion(&dir_version, searchingPath + "//" + fname, a_VersionB))
                        break;

                    cout << "   " << fname << " version: " << dir_version->getNroVersion() << ", tipo: d"
                         << " usuario: " << dir_version->getUser() << "-> agregado" << endl;

                    showAddedDirectory(dir_version, searchingPath + "//" + fname, 1);
                    delete dir_version;                 
                }
                else {
                    FileVersion* file_version;
                    int bloque;

                    if(!getFileVersionAndBlock(&bloque, &file_version, searchingPath + "//" + fname, a_VersionB))
                        break;

                    cout << "   " << fname << " version: " << file_version->getNroVersion() << ", tipo: " << file_version->getTipo()
                         << ", usuario: " << file_version->getUser() << "-> agregado" << endl;   

                    delete file_version;
                }
            }
        }

        delete versionDirectorioA;
        delete versionDirectorioB;
        return ret; 
    }

    return false;
}


bool VersionManager::getDiffByDate(std::ifstream& is, const string& a_Date)
{
   if(!_isOpen)      
      return false;

   int offset = _dateIndex.search(a_Date.c_str());
   if(offset < 0)
      return false;

   return _dateLog.showDate(a_Date,offset);      
}

bool VersionManager::getHistory(std::ifstream& is, const string& a_Filename)
{
    if (!_isOpen)
        return false;

    string searchingPath = _repository;

    if(!a_Filename.empty()) {
        for(int i = 1; i <= countComponents(a_Filename); ++i)
            searchingPath += "//" + getComponent(a_Filename, i);

        int bloque = _dirIndex.getFirstBlock(searchingPath.c_str());

        if (bloque >= 0) {
            cout << a_Filename << endl;
            _dirVersions.getHistory(is, bloque);
            return true;
        }
        else {
            bloque = _fileIndex.getFirstBlock(searchingPath.c_str());
            if (bloque < 0)
                return false;

            cout << a_Filename << endl;
            _fileVersions.getHistory(is, bloque);
            return true;
        }
    }
    else {
        int bloque = _dirIndex.searchFile(_repository.c_str());
        int lastVersion = _dirVersions.getLastVersionNumber(bloque);
        DirectoryVersion* dv;
        if (getDirVersion(&dv, _repository, toString<int>(lastVersion))) {
           cout << _repository << endl;
           showDirectory(dv, _repository, 0); 
           delete dv;
           return true;
        }
    }
    return false;
}

void VersionManager::showAddedDirectory(DirectoryVersion* dirVersion, const string& path, int tabs)
{
    string espacio;
    for (int i = 1; i <= tabs; ++i)
        espacio = espacio + "   ";

    list<File>* filesLst = dirVersion->getFilesList();
    list<File>::iterator it_files;

    for (it_files = filesLst->begin(); it_files != filesLst->end(); ++it_files) {
        string fname = it_files->getName();

        if (it_files->getType() != 'd') { 
            FileVersion* fileVersion;
            int bloque;

            if (!getFileVersionAndBlock(&bloque, &fileVersion, path + "//" + fname,toString<int>(it_files->getVersion()) ) )
                break;

            cout << espacio + "   " << fname << " version: " << fileVersion->getNroVersion() << ", tipo: " << fileVersion->getTipo() << ", usuario: "
                 << fileVersion->getUser() << "-> agregado" << endl;

            delete fileVersion;
        }
        else {
            DirectoryVersion* dirVersion;
            if(!getDirVersion(&dirVersion, path + "//" + fname, toString<int>(it_files->getVersion()) ) )
                break;

            cout << espacio + "   " << fname << " version: " << dirVersion->getNroVersion() << ", tipo: " << "d, usuario: " << dirVersion->getUser()
                 << "-> agregado" << endl;

            showAddedDirectory(dirVersion,path + "//" + fname, tabs + 1);
            delete dirVersion;
        }      
    }
}

void VersionManager::showDirectory(DirectoryVersion* dirVersion, const string& path, int tabs)
{
    string espacio;
    for (int i = 1; i <= tabs; ++i)
        espacio = espacio + "   ";

    list<File>* filesLst = dirVersion->getFilesList();
    list<File>::iterator it_files;

    for (it_files = filesLst->begin(); it_files != filesLst->end(); ++it_files) {
        string fname = it_files->getName();

        if (it_files->getType() != 'd') { 
            FileVersion* fileVersion;
            int bloque;

            if (!getFileVersionAndBlock(&bloque, &fileVersion, path + "//" + fname,toString<int>(it_files->getVersion()) ) )
                break;

            cout << espacio + "   " << fname << " version: " << fileVersion->getNroVersion() << ", tipo: " << fileVersion->getTipo() << ", usuario: "
                 << fileVersion->getUser() << endl;

            delete fileVersion;
        }
        else {
            DirectoryVersion* dirVersion;
            if(!getDirVersion(&dirVersion, path + "//" + fname, toString<int>(it_files->getVersion()) ) )
                break;

            cout << espacio + "   " << fname << " version: " << dirVersion->getNroVersion() << ", tipo: " << "d, usuario: " << dirVersion->getUser()
                 << endl;

            showDirectory(dirVersion,path + "//" + fname, tabs + 1);
            delete dirVersion;
        }      
    }
}

void VersionManager::log(const string& a_Filename, const string& a_Username, const string& a_Version, time_t a_Date)
{
   tm* date = localtime(&a_Date);
   int anio = date->tm_year + 1900;
   int mes = date->tm_mon;
   int dia = date->tm_mday;

   string fecha = toString<int>(anio) + "/" + zeroPad(mes + 1, 2) + "/" + zeroPad(dia, 2) + " - " 
                  + toString<int>(date->tm_hour) + ":" + toString<int>(date->tm_min) + ":" + toString<int>(date->tm_sec);
 
   int offset = _dateLog.append(a_Username, fecha, a_Version, a_Filename);   
   
   // indexo la fecha en el indice por fechas   
   if (_dateIndex.search(fecha.c_str()) < 0)
      _dateIndex.insert(fecha.c_str(),offset);

   // indexo el usuario en el indice por usuario
   int ref = _usersIndex.search(a_Username.c_str());
   int nuevoBloque;
   if(ref < 0) {
      _usersReg.insertRef(offset, &nuevoBloque);
      _usersIndex.insert((a_Username + zeroPad(nuevoBloque,VERSION_DIGITS)).c_str(),nuevoBloque);
      
      return;
   }
   
   UsersRegisterFile::t_status status = _usersReg.insertRef(offset, ref, &nuevoBloque);
   switch (status) {
      case UsersRegisterFile::OK :
         return;
         break;
       case UsersRegisterFile::OVERFLOW :
         _usersIndex.insert((a_Username + zeroPad(nuevoBloque, VERSION_DIGITS)).c_str(), nuevoBloque);
         return;
         break;

        default:
            return;
            break;
    }
    
   return;
}


bool VersionManager::getListOfChanges(std::ifstream& is, const string& a_Username, int a_Num)
{
    if (!a_Username.empty()) {
        int ref = _usersIndex.search(a_Username.c_str());
        if (ref < 0) 
            return false;

        list<int> lstChanges;
        if (a_Num > 0)
            lstChanges = _usersReg.getReferences(ref, a_Num);
        else
            lstChanges = _usersReg.getAllReferences(ref);

        list<int>::iterator it;
        for (it = lstChanges.begin(); it != lstChanges.end(); ++it) {
            if (!_dateLog.show(*it))
                return false;
        }

        return true;
    }

    return (_dateLog.showAll());
}
