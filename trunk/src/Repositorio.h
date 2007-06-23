// Repositorio.h

#ifndef REPOSITORIO_H_INCLUDED
#define REPOSITORIO_H_INCLUDED

#include "arbolbmas.h"
#include "ConfigData.h"
#include "SVNException.h"
#include "User.h"
#include "VersionManager.h"

#include <list>

class Repositorio
{
public:
    Repositorio(const string& a_Almacen, const string& a_Name); 
    ~Repositorio() {};

    bool create();
    bool destroy();

    bool open();
    bool close();
   
    bool add(const string& a_Target, const string& a_Username, const string& a_Password);
    bool removeFileOrDirectory(const string& a_Target, const string& a_Username, const string& a_Password);
    bool get(const string& a_TargetDestiny, const string& a_Target, const string& a_Version, 
                 const string& a_Username, const string& a_Password);

    bool addUser   (const string& a_Username, const string& a_Password, const string& a_Fullname);
    bool removeUser(const string& a_Username);
    bool userExists(const string& a_Username) const;
    bool validateUser    (const string& a_Username, const string& a_Password) const;
    bool validatePassword(const string& a_Username, const string& a_Password) const;
    bool getDiff(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_VersionA, const string& a_VersionB, const string& a_Filename = "");
    bool getDiffByDate(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_Date);
    bool getHistory(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_Filename);
    bool getListOfChanges(std::ifstream& is, const string& a_Username, const string& a_Password, int a_Num, bool isAdmin);

    // getters
    string getName() const { return _name; }
    std::list<User> getListOfUsers() const { return _lUsers; }

protected:
    bool saveVersion();
    bool loadVersion();

private:
    Repositorio();

    // member variables
    int             _version;
    string          _name;
    string          _almacen;
    std::list<User> _lUsers;
    VersionManager  _versionManager;
    bool            _isOpen;
};

#endif

