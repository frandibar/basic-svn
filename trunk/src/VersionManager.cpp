// VersionManager.cpp

#include "VersionManager.h"
#include "debug.h"
#include <fstream>
#include <list>
#include <iterator>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

using std::list;
using std::iterator;

using namespace std;

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

    
bool VersionManager::addFile(int repositoryVersion, const string& repositoryName, const string& a_Filename, const string& a_User, time_t a_Date, char a_Type)
{
    if (!_isOpen)
        return false;

    int bloque;
    int nroNuevoBloque;
    long int offset;
    string key;
	
	key = repositoryName;

	for(int i = 1; i <= countComponents(a_Filename); ++i)
		key = key + "//" + getComponent(a_Filename,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el archivo
    bloque = _fileIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el archivo esta en el indice
        FileVersion* ultimaVersion;
        _fileVersions.getLastVersion(&ultimaVersion, bloque);
        if (ultimaVersion->getTipo() != a_Type) {
            delete ultimaVersion;
            return false;
        }
		  
		  //analizo si la ultima version fue o no de borrado, si es asi, debo copiar todo el archivo
		  if(ultimaVersion->getVersionType() == FileVersion::BORRADO)
		  {
				delete ultimaVersion;	//elimino la ultima version

            std::ifstream is(a_Filename.c_str());
            if (!is) 
                return false;
            offset = _textContainer.append(is);
            is.close();
            if (offset == -1) 
                return false;

            FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset, a_Type, 				   FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
            switch (status) {
            	case FileVersionsFile::OK :
               	return true;
                  break;
            	case FileVersionsFile::OVERFLOW :
               	// tengo que generar la clave a partir de a_File y repositoryVersion
               	key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
                  return _fileIndex.insert(key.c_str(), nroNuevoBloque);
						break;
               default:
            		return false;
						break;
				}
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

            FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, offset,
                                                           a_Type, FileVersion::MODIFICACION, bloque, &nroNuevoBloque);
            switch (status) {
                case FileVersionsFile::OK :
                    return true;
                    break;

                case FileVersionsFile::OVERFLOW :
                    // tengo que generar la clave a partir de a_File y repositoryVersion
						  key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
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
                        key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
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
       key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
       return (_fileIndex.insert(key.c_str(), nroNuevoBloque));
    }   

    return false; // never gets here
}

bool VersionManager::addDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date)
{
    if (!_isOpen)
        return false;

    int bloque;
    int nroNuevoBloque;
    string key;
	 DIR* dir;
	 struct dirent* myDirent;
	 DirectoryVersion* nuevaVersion;
	
	key = repositoryName;

	for(int i = 1; i <= countComponents(a_Directoryname); ++i)
		key = key + "//" + getComponent(a_Directoryname,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el directorio
    bloque = _dirIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el directorio esta en el indice
        DirectoryVersion* ultimaVersion;
		  int lastVersion  = _dirVersions.getLastVersionNumber(bloque);	//obtengo el numero de la ultima version

        _dirVersions.getVersion(lastVersion, bloque, &ultimaVersion);	//obtengo la ultima version

		  //creo la nueva version
		  nuevaVersion = new DirectoryVersion(repositoryVersion,a_User.c_str(),*date,DirectoryVersion::MODIFICACION);

		  //debo cotejar los cambios que hubo en el directorio (eliminacion y agregado de nuevos archivos/directorios)

		  //abro el directorio que quiero "versionar"

		  if((dir = opendir(a_Directoryname.c_str())) == NULL)
		  {
					delete nuevaVersion;
					delete ultimaVersion;
					
					cout<<"El directorio: "<<a_Directoryname<<" no existe"<<endl;
					return false;
  	 	  }

		  //obtengo una lista con todos los nombres de los archivos/directorios pertenecientes al directorio que quiero agregar
		  list<string> fileIncludedLst;
		  while((myDirent = readdir(dir)) != NULL)
		  {
				string filename = myDirent->d_name;
				if( (filename.compare(".") != 0) && (filename.compare("..") != 0) )
					fileIncludedLst.push_back(filename);
		  }

		  closedir(dir);

		  list<File>* filesLst = ultimaVersion->getFilesList();	//lista con los archivos que pertencian a la ultima version
			  
		  list<File>::iterator it_oldFiles;		//iterador para recorrer la lista de los archivos/directorios de la ultima version
		  list<string>::iterator it_newFiles;	//iterador para recorrer la lista de los archivos/directorios de la nueva version

		  bool result = true;

		  if(ultimaVersion->getType() != DirectoryVersion::BORRADO)
		  {		
			  //ahora debo si hay algun borrado y luego generar la version del archivo
			  
			  list<File> filesErased; //lista con los nombres de los archivos/directorios que fueron borrados

			  for(it_oldFiles = filesLst->begin(); it_oldFiles != filesLst->end(); it_oldFiles++)
			  {
				 string fname = it_oldFiles->getName();
				 bool included = false;
				 
				 for(it_newFiles = fileIncludedLst.begin();it_newFiles != fileIncludedLst.end(); it_newFiles++)
					if(fname.compare(*it_newFiles)==0)
						included = true;
				
				 if(!included)
				 {
					File* file = new File(it_oldFiles->getName(),it_oldFiles->getVersion(),it_oldFiles->getType());
					filesErased.push_back(*file);
				 }				
			  }

			  list<File>::iterator it_erasedFiles;

			  for(it_erasedFiles = filesErased.begin();it_erasedFiles != filesErased.end();it_erasedFiles++)
			  {
					string name = it_erasedFiles->getName();
					string fname = a_Directoryname + "/" + name;

					if(it_erasedFiles->getType() != 'd')
						result = result && removeFile(repositoryVersion, repositoryName, fname, a_User, a_Date);
					
					else
						result = result && removeDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
			  }

			  filesErased.clear();		
	      }
			
		  delete ultimaVersion;	//elimino la ultima version, ya no la voy a necesitar

		  list<string>::iterator it_includedFiles;

		  if(result)
			  for(it_includedFiles = fileIncludedLst.begin(); it_includedFiles != fileIncludedLst.end(); it_includedFiles++)
			  {	//versiono todos los archivos/directorios que pertenecen al directorio
					
					string fname = a_Directoryname + "/" + *it_includedFiles;
					
					t_filetype ftype = getFiletype(fname);
					char type;
					
					if(ftype == DIRECTORY)
					{
						type = 'd';
						result = result && addDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
					}
					
					else if((ftype == TEXT)||(ftype == BINARY))
					{
						type = (ftype == TEXT ? 't' : 'b');
						result = result && addFile(repositoryVersion, repositoryName, fname, a_User, a_Date,type);
					}
					
					else result = false;

					if(result)// agrego el archivo al directorio
						nuevaVersion->addFile((*it_includedFiles).c_str(),repositoryVersion,type);										
			  }
			
		  if(result)
		  { 							
	        DirectoryVersionsFile::t_status status = _dirVersions.insertVersion(nuevaVersion, bloque, &nroNuevoBloque);
	        switch (status) {
	        	case DirectoryVersionsFile::OK :
	         	result = true;
	            break;

	         case DirectoryVersionsFile::OVERFLOW :
	         	// tengo que generar la clave a partir de a_Directoryname y repositoryVersion
					key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
	         	result = _dirIndex.insert(key.c_str(), nroNuevoBloque);
					break;

	          default:
	          	result = false;
					break;
	        }
			}

		  delete nuevaVersion;
			
		  return result;
	 }

	else	//es la 1º vez que se va a agregar el directorio, por lo tanto, no debo cotejar los cambios
	{
		  nuevaVersion = new DirectoryVersion(repositoryVersion,a_User.c_str(),*date,DirectoryVersion::MODIFICACION);

		  if((dir = opendir(a_Directoryname.c_str())) == NULL)
		  {
					delete nuevaVersion;
					
					cout<<"El directorio: "<<a_Directoryname<<" no existe"<<endl;
					return false;
  	 	  }

		  //obtengo una lista con todos los nombres de los archivos/directorios pertenecientes al directorio que quiero agregar
		  list<string> fileIncludedLst;
		  while((myDirent = readdir(dir)) != NULL)
		  {
				string filename = myDirent->d_name;
				if( (filename.compare(".") != 0) && (filename.compare("..") != 0) )
					fileIncludedLst.push_back(filename);		  
		  }
			
		  closedir(dir);

		  list<string>::iterator it_includedFiles;
		  bool result = true;

		  for(it_includedFiles = fileIncludedLst.begin(); it_includedFiles != fileIncludedLst.end(); it_includedFiles++)
		  {	//versiono todos los archivos/directorios que pertenecen al directorio
				
				string fname = a_Directoryname + "/" + *it_includedFiles;
				
				t_filetype ftype = getFiletype(fname);
				char type;
				
				if(ftype == DIRECTORY)
				{
					type = 'd';
					result = result && addDirectory(repositoryVersion, repositoryName, fname, a_User, a_Date);
				}
				
				else if((ftype == TEXT)||(ftype == BINARY))
				{
					type = (ftype == TEXT ? 't' : 'b');
					result = result && addFile(repositoryVersion, repositoryName, fname, a_User, a_Date,type);
				}
				
				else result = false;

				if(result)// agrego el archivo al directorio
					nuevaVersion->addFile((*it_includedFiles).c_str(),repositoryVersion,type);					
				
		  }

		if(result)
		{
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

	for(int i = 1; i <= countComponents(a_Target); ++i)
		finalPath = finalPath + "//" + getComponent(a_Target,i);

	if(pathActual.compare(finalPath) == 0)
	{	
		if((a_Type == BINARY) || (a_Type == TEXT))		
			ret = addFile(repositoryVersion,repositoryName, a_Target, a_Username, a_Date, (a_Type == TEXT ? 't' : 'b'));

		else	
			ret = addDirectory(repositoryVersion,repositoryName, a_Target, a_Username,a_Date);
	}

	else
	{
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

		int nuevoBloque;

		string caminoRecorrido = pathActual + "//" + componente;
						
		if(caminoRecorrido.compare(finalPath) == 0)
		{
			if(a_Type == TEXT) tipoArchivo = 't';

			else if(a_Type == BINARY) tipoArchivo = 'b';

			else tipoArchivo = 'd';				
		}
		
		else tipoArchivo = 'd';
					
		if(bloque >= 0)
		{		
			//obtengo el ultimo nro de version del directorio
			int lastVersionNumber = _dirVersions.getLastVersionNumber(bloque);

			DirectoryVersion* oldVersion;

			//trato de obtener la ultima version del directorio
			if(!_dirVersions.getVersion(lastVersionNumber,bloque,&oldVersion))
				ret = false;	

			else
			{
				ret = addRec(a_Target,componenteALeer + 1,pathActual + "//" + componente, repositoryName, repositoryVersion, cantComponentesPath, a_Username, a_Date, a_Type);

				if(ret){
					//aca tengo que cotejar los cambios que se realizan en la version										
					list<File>* lst = oldVersion->getFilesList();
					
					list<File>::iterator it;

					debug("comienzo a copiar la lista");
					//copio los archivos que tenia en la version anterior a la nueva para luego actualizarlos
					for(it = lst->begin();it != lst->end();it++)	
						nuevaVersion->addFile(it->getName(),it->getVersion(),it->getType());

					debug("termine de copiar la lista");
					debug("elimino la version vieja");
					//libero el puntero a la ultima version
					delete oldVersion;

					//actualizo la version de la componente leida
					nuevaVersion->update(componente.c_str(),repositoryVersion,tipoArchivo);
					debug("actualizo la version");

					switch(_dirVersions.insertVersion(nuevaVersion, bloque, &nuevoBloque))
					{
						case DirectoryVersionsFile::OK:
							ret = true;
							break;
						case DirectoryVersionsFile::OVERFLOW:
							key = pathActual + zeroPad(repositoryVersion,VERSION_DIGITS);
							ret = _dirIndex.insert(key.c_str(),nuevoBloque);
							break;
						default:
							ret = false;
							break;				
					}
				}										
			}			
		}
		
		else
		{	
			ret = addRec(a_Target, componenteALeer + 1, pathActual + "//" + componente, repositoryName, repositoryVersion, cantComponentesPath, a_Username, 						a_Date, a_Type);
			
			if(ret)
			{		
				nuevaVersion->addFile(componente.c_str(),repositoryVersion,tipoArchivo);

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

bool VersionManager::getFile(const string& a_TargetDir, const string& a_Filename, const string& a_Version,const string& repositoryName)
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

	 int RepNameEnd = a_Filename.find_first_not_of(repositoryName + "//");
	
	 string path = a_Filename;
	
	 path.erase(0,RepNameEnd);

	 if(versionBuscada->getVersionType() == FileVersion::BORRADO)	//si la version buscada es una version de borrado entonces no hago nada
	 {
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
        if (_fileVersions.getVersionFrom(original, final, bloque, versionsList))
		  {
				//chequeo si el archivo ya existe o no. Si ya existe, debo preguntar si lo reescribo o no.
				fileToCheck.open((a_TargetDir + "//" + path).c_str());
				fileToCheck.close();
				if(fileToCheck.fail())	//si el archivo no existia lo escribo
            	return (buildVersion(versionsList, a_TargetDir + "//" + path));
				else
				{
					do{
						cout<<"El archivo: "<<a_TargetDir<<"/"<<path<<" ya existe. Desea sobreescribirlo? S/N"<<endl;
						cin>>option;
						cout<<endl;
						option = toupper(option);
					}while( (option != 'N') && (option != 'S') );
					
					if(option == 'S')
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
		  if(fileToCheck.fail())	//si el archivo no existia lo escribo
		  {
	        std::ofstream os((a_TargetDir + "//" + path).c_str());
	        if (!os.is_open())
	            return false;
	        if (!_binaryContainer.get(offset, os))
	            return false;
	        os.close();
	        return true;
		   }
			else	//si el archivo ya existia pregunto si hay que sobreescribirlo
			{
				do{
					cout<<"El archivo: "<<a_TargetDir<<"/"<<path<<" ya existe. Desea sobreescribirlo? S/N"<<endl;
					cin>>option;
					cout<<endl;
					option = toupper(option);
				}while( (option != 'N') && (option != 'S') );
				
				if(option == 'S')
				{	//sobreescribo el archivo
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
    int bloque;
	 int lastVersion;

    if (a_Version != "") {
        int version = fromString<int>(a_Version);
        bloque = _dirIndex.searchFileAndVersion(fullDirName.c_str(), version);
        if (!_dirVersions.searchVersion(&versionBuscada, version, bloque)) {
            bloque = _dirIndex.searchFile(fullDirName.c_str());
				lastVersion = _dirVersions.getLastVersionNumber(bloque);
            if(!_dirVersions.searchVersion(&versionBuscada, lastVersion, bloque))
                return false;
            else if(versionBuscada->getNroVersion() > version) {
                delete versionBuscada;
                return false;
            }
        }        
    }
    else {
        bloque = _dirIndex.searchFile(fullDirName.c_str());
		  lastVersion = _dirVersions.getLastVersionNumber(bloque);
        if (!_dirVersions.searchVersion(&versionBuscada, lastVersion, bloque))
           return false; 
    }

	 if(versionBuscada->getType() == DirectoryVersion::BORRADO)
	 {	//no puedo recuperar una version de borrado
		delete versionBuscada;
		return false;
	 }

	 bool ret = true;	//el valor que voy a devolver
	 
	 //obtengo la lista de archivos/directorios del directorio que quiero obtener
	 list<File>* filesLst = versionBuscada->getFilesList();	
	 list<File>::iterator it;

	 //si el directorio que voy a obtener no esta creado en el destino --> lo creo
	 bool creado = false;
	 
	 string currentDir = get_current_dir_name();

	 if(chdir((a_TargetDir + "//" + pathToFile + "//" + a_DirName).c_str()) != 0)
	 {
			if(mkdir((a_TargetDir + "//" + pathToFile + "//" + a_DirName).c_str(),0755) != 0)
			{
				cout<<"Imposible crear: "<<a_TargetDir << "//" << pathToFile << "//" << a_DirName<<endl;
				delete versionBuscada;				 
				return false;
			}	
			
			else
			{
				creado = true;
				chdir(currentDir.c_str());
			}
	 }
		
	 for(it = filesLst->begin(); it != filesLst->end(); it++)
	 {
		string FName = it->getName();
		string version_number = toString<int>(it->getVersion());
		
		if(it->getType() != 'd')
			ret = ret && getFile(a_TargetDir, fullDirName + "//" + FName, version_number, repositoryName);
		else
			ret = ret && getDirectory(a_TargetDir,pathToFile + "//" + a_DirName, fullDirName, FName, version_number, repositoryName);

	 }
	
	 //si fallo la recuperacion y cree un directorio --> lo elimino
	 if((ret == false) && (creado == true))
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
    int bloque;

	 //tengo que armar el path del directorio contenerdor al archivo/directorio que estoy buscando
	 //para todos los casos el directorio raiz es el repositorio por lo tanto todos los paths de los archivos empiezan con:
	 //"nombre_repositorio//" y van seguidos del path correspondiente...

	 string searchingPath = repositoryName;

    //voy agregando componente a componente para saber donde debo buscar
 	 for(int i = 1; i < countComponents(a_Target);i++)
		searchingPath = searchingPath + "//" + getComponent(a_Target,i);
	 	
	 //obtengo la version del directorio que contiene al archivo/directorio objetivo con el mismo nro de version que deseo que tenga el 
 	 //archivo/directorio si es que voy a querer una version en particular o la ultima version del directorio que contiene al archivo/ 
	 //directorio si esa que no especifique ninguna
    if (a_Version != "") {
        int version = fromString<int>(a_Version);
        bloque = _dirIndex.searchFileAndVersion(searchingPath.c_str(), version);
		  if (bloque < 0) return false;
        if (!_dirVersions.searchVersion(&versionDirectorioContenedor, version, bloque)) {
            bloque = _dirIndex.searchFile(searchingPath.c_str());
				if (bloque < 0) return false;
				version = _dirVersions.getLastVersionNumber(bloque);
            if(!_dirVersions.getVersion(version, bloque, &versionDirectorioContenedor))
                return false;
            else if(versionDirectorioContenedor->getNroVersion() > version) {
                delete versionDirectorioContenedor;
                return false;
            }
        }        
    }
    else {
        bloque = _dirIndex.searchFile(searchingPath.c_str());
		  if(bloque < 0) return false;
		  int lastVersion = _dirVersions.getLastVersionNumber(bloque);
        if (!_dirVersions.getVersion(lastVersion, bloque, &versionDirectorioContenedor))
           return false; 
    }


	if(a_Target != "")
	{
		if(versionDirectorioContenedor->getType() == DirectoryVersion::BORRADO)
		{
			delete versionDirectorioContenedor;
			return false;
		}

		string filename = getComponent(a_Target,countComponents(a_Target));
		
		File* file;
		if(!versionDirectorioContenedor->searchFile(filename.c_str(),&file))
		{	
			delete versionDirectorioContenedor;
			return false;
		}

		//una vez obtenida la version del directorio voy a tener que armar la estructura de directorios que contienen al archivo/directorio
		//objetivo dentro del directorio destino para luego "bajar" la version solicitada del archivo/directorio objetivo
		
		//aca voy a guardar el path actual para luego volver al directorio de trabajo
		string currentDirectory = get_current_dir_name();

		//empiezo a armar la estructura
		int existentes  = 0;

		//trato de cambiar de directorio al directorio destino para chequear que existe
		if(chdir(a_TargetDestiny.c_str()) != 0)
		{
			//si el directorio destion no existe -> vuelvo al directorio de trabajo actual y elimino las referencias que tengo en memoria
			chdir(currentDirectory.c_str());
			delete versionDirectorioContenedor;
			cout<<"El directorio elegido como destino no existe"<<endl;
			return false;		
		}

		chdir(currentDirectory.c_str()); //vuelvo al directorio de trabajo
			
		//ahora tengo que armar la estructura de directorios a donde va a ir a parar el archivo/directorio objetivo
		int RepNameEnd = searchingPath.find_first_not_of(repositoryName + "//");
			
		string path = searchingPath;
			
		path.erase(0,RepNameEnd);
			
		string pathAuxiliar = a_TargetDestiny;
			
		int components = countComponents(path);

		if(path.length() > 0)
		{
			//empiezo a armar la estructura de directorios
			int components = countComponents(path);

			debug("creando directorios \n");

			for(int j = 1; j <= components;j++)
			{
				pathAuxiliar = pathAuxiliar + "/" + getComponent(path,j);

				//si no existe el directorio lo creo
				if(chdir(pathAuxiliar.c_str()) != 0)
				{						
					if(mkdir(pathAuxiliar.c_str(),0755) != 0)
					cout<<"error al crear: "<<pathAuxiliar<<endl;
				}
					
				else
				{
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

		if(ret == false)
		{
			//elimino los directorios que cree para albergar el archivo/directorio que queria obtener
				
			for(int j = components; j > existentes; j--)
			{
				remove(pathAuxiliar.c_str());
					
				int index = pathAuxiliar.find_last_of("/");
				
				pathAuxiliar.erase(index);			
			}
		}

		delete versionDirectorioContenedor;
			
		return ret;
	}
	
	else
	{
		bool ret = true;
		list<File>* filesLst = versionDirectorioContenedor->getFilesList();

		list<File>::iterator it_files;

		string fname;
		for(it_files = filesLst->begin(); it_files != filesLst->end(); it_files++)
		{
			fname = it_files->getName();
			
			if(it_files->getType() == 'd')
				ret = ret && getDirectory(a_TargetDestiny, "" , searchingPath, fname, a_Version, repositoryName);
			
			else
				ret == ret && getFile(a_TargetDestiny,searchingPath + "//" + fname, a_Version, repositoryName);
		}

		if(!ret)
		{
			for(it_files = filesLst->begin(); it_files != filesLst->end(); it_files++)
			{
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

    int bloque;
    int nroNuevoBloque;
    string key;
	
	key = repositoryName;

	for(int i = 1; i <= countComponents(a_Filename); ++i)
		key = key + "//" + getComponent(a_Filename,i);
	
	debug("clave a borrar: "+key+"\n");

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el archivo
    bloque = _fileIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el archivo esta en el indice, entonces, se puede borrar
        FileVersion* ultimaVersion;
		  
        _fileVersions.getLastVersion(&ultimaVersion, bloque);
	
		  if(ultimaVersion->getVersionType() == FileVersion::BORRADO)
		  { //si ya esta borrado no puedo volver a borrarlo				
			 delete ultimaVersion;
			 return false;
		  }

		  debug("obtengo el fileType \n");
		  char tipoArchivo = ultimaVersion->getTipo();
			
		  delete ultimaVersion;
			
		  // si el archivo no fue borrado, creo una version de borrado
        FileVersionsFile::t_status status = _fileVersions.insertVersion(repositoryVersion, a_User.c_str(), *date, -1,
        tipoArchivo, FileVersion::BORRADO, bloque, &nroNuevoBloque);
		  debug("version de borrado creada \n");
        switch (status) {
        	case FileVersionsFile::OK :
				debug("status OK \n");
         	return true;
         	break;
         case FileVersionsFile::OVERFLOW :
            // tengo que generar la clave a partir de a_File y repositoryVersion
			   key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
				debug("status OVERFLOW \n");
            return _fileIndex.insert(key.c_str(), nroNuevoBloque);

         default:
				debug("status ERROR \n");
            return false;
				break;
         }
		}
	
	debug("bloque < 0 \n");	
		        	
	return false;
}

bool VersionManager::removeDirectory(int repositoryVersion, const string& repositoryName, const string& a_Directoryname, const string& a_User, time_t a_Date)
{
	debug("ingreso en removeDirectory \n");
    if (!_isOpen)
        return false;

    int bloque;
    int nroNuevoBloque;
    string key;
	 DirectoryVersion* nuevaVersion;
	
	key = repositoryName;

	for(int i = 1; i <= countComponents(a_Directoryname); ++i)
		key = key + "//" + getComponent(a_Directoryname,i);

    tm* date = localtime(&a_Date);
    // busco en el indice a ver si esta el directorio
    bloque = _dirIndex.searchFile(key.c_str());

    if (bloque >= 0) { // el directorio esta en el indice
        DirectoryVersion* ultimaVersion;
		  int lastVersion  = _dirVersions.getLastVersionNumber(bloque);	//obtengo el numero de la ultima version

        _dirVersions.getVersion(lastVersion, bloque, &ultimaVersion);	//obtengo la ultima version

		  debug("bloque >= 0 \n");
		  if(ultimaVersion->getType() == DirectoryVersion::BORRADO)
		  {
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
		  //a cada archivo/directorio de la version anterior les debo hacer un borrado
		  for(it_filesToRemove = filesLst->begin();it_filesToRemove != filesLst->end(); it_filesToRemove++)
		  {
			
			 string fname = it_filesToRemove->getName();
			 string fullPath = a_Directoryname + "/" + fname;

			 debug("archivo a eliminar: "+fullPath+"\n");
			 if(it_filesToRemove->getType() == 'd')
			 {
				result = result && removeDirectory(repositoryVersion,repositoryName,fullPath,a_User,a_Date);
				debug("entro en removeDirectory \n");
			 }
			 else
			 {
				result = result && removeFile(repositoryVersion,repositoryName,fullPath,a_User,a_Date);
				debug("entro en removeFile \n");
			 }
			 if(!result)
				debug("el borrado falla en: "+fullPath+"\n");			
		  }
	
		  if(!result)
		  {
				debug("fallo el borrado \n");
				delete ultimaVersion;
				delete nuevaVersion;
				
				return false;
		  }
			
		  DirectoryVersionsFile::t_status status = _dirVersions.insertVersion(nuevaVersion, bloque, &nroNuevoBloque);
	     	switch (status) {
	      	case DirectoryVersionsFile::OK :
					debug("status OK \n");
	         	result = true;
	            break;
	         case DirectoryVersionsFile::OVERFLOW :
	         	// tengo que generar la clave a partir de a_Directoryname y repositoryVersion
					key = key + zeroPad(repositoryVersion, VERSION_DIGITS);
	         	result = _dirIndex.insert(key.c_str(), nroNuevoBloque);
					debug("status OK \n");
					debug("nueva clave: "+key+"\n");
					break;

	          default:
	          	result = false;
					break;
			}
		return result;
	}

	//si llega aca es porque no habia una vesion previa del directorio, por lo tanto, no se puede realizar el borrado		  
	return false;
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


bool VersionManager::getDiffByDate(std::ifstream& is, const string& a_Date)
{
    // TODO
    return false;
}

bool VersionManager::getHistory(std::ifstream& is, const string& a_Filename)
{
    // TODO
    return false;

}

