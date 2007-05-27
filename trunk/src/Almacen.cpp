// Almacen.cpp

#include "Almacen.h"

#include <iostream>
#include <sys/stat.h>

using std::cout;
using std::endl;

string Almacen::CONFIG_FILE = "";

bool Almacen::createConfigFile(const string& a_Dir)
// crea el archivo de configuracion con un solo elemento:
// <almacen="...">
// </almacen>    
{
    std::ofstream file(CONFIG_FILE.c_str());
    if (!file) {
        return false;
    }
    file << "<almacen dir=\"" << a_Dir << "\">" << endl
         << "</almacen>" << endl;
    file.close();
    return true;
}

bool Almacen::load() throw(xercesc::XMLException&)
{
    // load from xml file

    // initialize xerces
    xercesc::XMLPlatformUtils::Initialize();
    
    CONFIG_FILE = "//home//" + string(getenv("USER")) + "//.svn_grupo_config";
    _config = new XMLConfigData(CONFIG_FILE);
    _name = _config->getDirAlmacen();

    // typedef vector<pair<string, UsersList> > RepositoriosList;
    XMLConfigData::RepositoriosList replist = _config->getRepositories();
    
    XMLConfigData::RepositoriosList::iterator repIt;
    for (repIt = replist.begin(); repIt != replist.end(); ++repIt) {
        Repositorio* rep = new Repositorio(_name, repIt->first);
        XMLConfigData::UsersList::iterator usIt;
        for (usIt = repIt->second.begin(); usIt != repIt->second.end(); ++usIt) {
            rep->addUser(usIt->username, usIt->password, usIt->fullname);
        }
        _lReposit.push_back(rep);
    }

   
    // terminate xerces
    xercesc::XMLPlatformUtils::Terminate();

    return true;
}

bool Almacen::addReposit(const string& a_Name)
{
    if (!_exists)
        return false;
    
    // initialize xerces
    xercesc::XMLPlatformUtils::Initialize();
    
    bool ret = _config->addReposit(a_Name);
    if (ret) _config->commit();

    // crear el archivo del repositorio, y lo trunca si ya existe
    std::ofstream os(string(_name + "//" + a_Name).c_str());
    os.close();

    // terminate xerces
    xercesc::XMLPlatformUtils::Terminate();

    return ret;
}

bool Almacen::create(const string& a_Name)
{
    if (_exists) return false;

    // create configuration file
    if (_exists = createConfigFile(a_Name)) {
        // create directory where files will be stored
        mkdir(a_Name.c_str(), 0755);
        return true;
    }
    return false;
}

Repositorio* Almacen::getReposit(const string& a_Name) const
{
    std::list<Repositorio*>::const_iterator it;
    for (it = _lReposit.begin(); it != _lReposit.end(); ++it) {
        if ((*it)->getName() == a_Name)
            return *it;
    }
    return NULL;
}
