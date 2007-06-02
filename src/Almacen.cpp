// Almacen.cpp

#include "Almacen.h"
#include "debug.h"

#include <iostream>
#include <sys/stat.h>

using std::cout;
using std::endl;

string Almacen::CONFIG_FILE = "";

// constructor
Almacen::Almacen() : _name(""), _config(NULL)
{ 
    _exists = load();
}

Almacen::~Almacen() 
{ 
    // TODO
    //delete _config;
    //std::list<Repositorio*>::iterator it;
    //for (it = _lReposit.begin(); it != _lReposit.end(); ++it)
        //delete *it;
    debug("Destroying Almacen '" + _name + "'\n");
}

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

    try {
        // initialize xerces
        xercesc::XMLPlatformUtils::Initialize();
        
        CONFIG_FILE = "//home//" + string(getenv("USER")) + "//.svn_grupo_config";
        _config = new XMLConfigData(CONFIG_FILE);
        _name = _config->getDirAlmacen();
        debug("Loading Almacen '" + _name + "'\n");

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
    }
    catch (...) {
        return false;
    }

    return true;
}

bool Almacen::remove()
{
    _exists = false;
    string cmd = "rm -rf " + _name;
    int ret = system(cmd.c_str());
    _name   = "";
    return (ret != -1);
}

bool Almacen::addRepository(const string& a_Name) throw()
{
    if (!_exists)
        return false;
    
    try {
        // initialize xerces
        xercesc::XMLPlatformUtils::Initialize();
        
        if (!_config->addRepository(a_Name)) 
            return false;
        _config->commit();
        _lReposit.push_back(new Repositorio(_name, a_Name));

        // terminate xerces
        xercesc::XMLPlatformUtils::Terminate();
    }
    catch (...) {
        return false;
    }

    return true;
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

Repositorio* Almacen::getRepository(const string& a_Name) const
{
    std::list<Repositorio*>::const_iterator it;
    for (it = _lReposit.begin(); it != _lReposit.end(); ++it) {
        if ((*it)->getName() == a_Name)
            return *it;
    }
    return NULL;
}


bool Almacen::removeRepository(const string& a_Name) throw()
{
    if (!_exists)
        return false;
    
    Repositorio* rep = getRepository(a_Name);
    if (rep == NULL)
        return false;

    _lReposit.remove(rep);

    try {
        // initialize xerces
        xercesc::XMLPlatformUtils::Initialize();
        
        if (!_config->removeRepository(a_Name)) 
            return false;
        _config->commit();

        // terminate xerces
        xercesc::XMLPlatformUtils::Terminate();
    }
    catch (...) {
        return false;
    }

    return true;
}

bool Almacen::addUser(const string& a_Reposit, const string& a_Username, const string& a_Password, const string& a_Fullname) throw()
{
    Repositorio* rep = getRepository(a_Reposit);
    if (rep == NULL)
        return false;

    rep->addUser(a_Username, a_Password, a_Fullname);

    try {
        // initialize xerces
        xercesc::XMLPlatformUtils::Initialize();
        
        if (!_config->addUser(a_Reposit, a_Username, a_Password, a_Fullname)) 
            return false;
        _config->commit();

        // terminate xerces
        xercesc::XMLPlatformUtils::Terminate();
    }
    catch (...) {
        return false;
    }

    return true;
}

bool Almacen::removeUser(const string& a_Reposit, const string& a_Username) throw()
{
    Repositorio* rep = getRepository(a_Reposit);
    if (rep == NULL)
        return false;

    rep->removeUser(a_Username);

    try {
        // initialize xerces
        xercesc::XMLPlatformUtils::Initialize();
        
        if (!_config->removeUser(a_Reposit, a_Username)) 
            return false;
        _config->commit();

        // terminate xerces
        xercesc::XMLPlatformUtils::Terminate();
    }
    catch (...) {
        return false;
    }

    return true;
}

bool Almacen::addFile(const string& a_Reposit, const string& a_Filename, const string& a_Username, const string& a_Password)
{
    Repositorio* rep = getRepository(a_Reposit);
    if (rep == NULL)
        return false;
    return rep->addFile(a_Filename, a_Username, a_Password);
}

bool Almacen::validatePassword(const string& a_Reposit, const string& a_Username, const string& a_Password) const
{
    Repositorio* rep = getRepository(a_Reposit);
    if (rep == NULL)
        return false;
    return rep->validatePassword(a_Username, a_Password);
}

bool Almacen::userExists(const string& a_Reposit, const string& a_Username) const
{
    Repositorio* rep = getRepository(a_Reposit);
    if (rep == NULL)
        return false;
    return rep->userExists(a_Username);
}

bool Almacen::repositoryExists(const string& a_Name) throw()
{
    return (getRepository(a_Name) != NULL);
}


std::list<User> Almacen::getListOfUsers(const string& a_Reposit) const
{
    std::list<User> users;
    Repositorio* rep = getRepository(a_Reposit);
    if (rep != NULL) {
        users = rep->getListOfUsers();
    }
    return users;
}
