// Almacen.h

#ifndef ALMACEN_H_INCLUDED
#define ALMACEN_H_INCLUDED

#include "ConfigData.h"
#include "Repositorio.h"
#include "User.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <list>
#include <sstream>

using std::string;

class Almacen
{
public:
    static string CONFIG_FILE;

    Almacen(); // constructor
    ~Almacen();

    bool create          (const string& a_Name);
    bool destroy();
    bool exists() const { return _exists; }

    bool addRepository   (const string& a_Name) throw();
    bool removeRepository(const string& a_Name) throw();
    bool repositoryExists(const string& a_Name) throw();

    bool addUser(const string& a_Reposit, const string& a_Username, const string& a_Password, const string& a_Fullname) throw();
    bool removeUser(const string& a_Reposit, const string& a_Username) throw();
    bool userExists(const string& a_Reposit, const string& a_Username) const;
    bool validatePassword(const string& a_Reposit, const string& a_Username, const string& a_Password) const;
    std::list<User> getListOfUsers(const string& a_Reposit) const;

    bool add(const string& a_Reposit, const string& a_Target, const string& a_Username, const string& a_Password);
    bool remove(const string& a_Reposit, const string& a_Target, const string& a_Username, const string& a_Password);
    bool get(const string& a_Reposit, const string& a_TargetDestiny, const string& a_Target, const string& a_Version, 
                 const string& a_Username, const string& a_Password) const;
    bool getDiff(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_VersionA, const string& a_VersionB, const string& a_Filename = "");

    // getters
    string getName() const { return _name; }

    bool createConfigFile(const string& a_Dir);

private:
    bool load() throw(xercesc::XMLException&);
    Repositorio* getRepository(const string& a_Name) const;

    // member variables
    string                  _name;
    XMLConfigData*          _config;
    std::list<Repositorio*> _lReposit;
    bool                    _exists;
};

#endif

