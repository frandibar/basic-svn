// VersionManager.cpp

#include "VersionManager.h"
#include "debug.h"


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

bool VersionManager::add(int repositoryVersion, const string& a_Target, const string& a_Username, time_t a_Date, t_filetype a_Type, const string& repositoryName)
{
  int componenteALeer = 1;
  
  string pathActual = repositoryName;	//este tendria que ser el repository name.

  int cantComponentesPath = countComponents(a_Target);

  return addRec(a_Target, componenteALeer, pathActual, repositoryVersion, cantComponentesPath, a_Username, a_Date, a_Type, repositoryName);

}

bool VersionManager::addRec(const string& a_Target, int componenteALeer, const  string& pathActual, int repositoryVersion, int cantComponentesPath, const string& a_Username, time_t  a_Date, t_filetype a_Type, const string& repositoryName)
{
	bool ret = false;
	string key;	

	string finalTarget = repositoryName +  "//" + a_Target;
	if(pathActual == finalTarget)
	{	
		if((a_Type == BINARY) || (a_Type == TEXT))		
			ret = addFile(repositoryVersion, a_Target, a_Username, a_Date, a_Type);

		else	//TODO ADD_DIRECTORY
			ret = false;
	}

	else
	{
		// identifico si hay alguna version del directorio actual (por el que voy siguiendo el camino hasta target)
		int bloque = _dirIndex.searchFile(pathActual.c_str());

		//obtengo la fecha		    
		tm* date = localtime(&a_Date);		

		// creo una nueva version de directorio
		DirectoryVersion* nuevaVersion = new DirectoryVersion(repositoryVersion, a_Username.c_str(), *date, DirectoryVersion::MODIFICACION);
				
		if(bloque >= 0)
		{
			//aca tengo que cotejar los cambios que se realizan en la version
			ret = false;			
		}
		
		else
		{	
			// tomo la componente correspondiente					
			string componente = getComponent(a_Target,componenteALeer);

			ret = addRec(a_Target, componenteALeer + 1, pathActual + "//" + componente, repositoryVersion, cantComponentesPath, a_Username, 						a_Date, a_Type,repositoryName);
			
			if(ret)
			{
				char tipoArchivo;

				string caminoRecorrido = pathActual + "//" + componente;
				
				debug("camino recorrido: "+caminoRecorrido+"\n");
				debug("final target: "+finalTarget+"\n");
				if(caminoRecorrido == finalTarget)
				{
					if(a_Type == TEXT) tipoArchivo = 't';

					else if(a_Type == BINARY) tipoArchivo = 'b';

					else tipoArchivo = 'd';				
				}
			
				else tipoArchivo = 'd';
				

				nuevaVersion->addFile(componente.c_str(),repositoryVersion,tipoArchivo);

				int nuevoBloque;
				_dirVersions.insertVersion(nuevaVersion,&nuevoBloque);

				//genero la clave
				key = pathActual + zeroPad(repositoryVersion,VERSION_DIGITS);
				ret = _dirIndex.insert(key.c_str(),nuevoBloque);				
			}			
		}
		
		delete nuevaVersion;
		
		return ret;		
	}

	return ret;	//never gets here
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

    if (a_Version != "") {
        int version = fromString<int>(a_Version);
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
        int offset = versionBuscada->getOffset();
        delete versionBuscada;
        std::ofstream os((a_TargetDir + "//" + a_Filename).c_str());
        if (!os.is_open())
            return false;
        if (!_binaryContainer.get(offset, os))
            return false;
        os.close();
        return true;
    }
    return false; // never gets here
}

bool VersionManager::getVersionAndBlock(int* bloque, FileVersion** versionBuscada, const string& a_Filename, const string& a_Version)
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

bool VersionManager::buildTextVersion(int bloque, FileVersion* versionBuscada, const string& a_Filename)
{
    int original = versionBuscada->getOriginal();
    int final    = versionBuscada->getNroVersion();

    list<FileVersion> versionsList;
    if (_fileVersions.getVersionFrom(original, final, bloque, versionsList))
        return (buildVersion(versionsList, a_Filename));

    return false;
}

bool VersionManager::getDiff(std::ifstream& is, const string& a_VersionA, const string& a_VersionB, const string& a_Filename)
{
    if (!_isOpen)
        return false;

    FileVersion* versionBuscadaA;
    int bloqueA;

    FileVersion* versionBuscadaB;
    int bloqueB;

    if (!getVersionAndBlock(&bloqueA, &versionBuscadaA, a_Filename, a_VersionA))
        return false;

    if (!getVersionAndBlock(&bloqueB, &versionBuscadaB, a_Filename, a_VersionB)) {
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
