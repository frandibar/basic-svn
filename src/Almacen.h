// Almacen.h

#ifndef ALMACEN_H_INCLUDED
#define ALMACEN_H_INCLUDED

#include "ConfigData.h"
#include "Repositorio.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <list>

using std::string;

class Almacen
{
public:
    static string CONFIG_FILE;

    // constructor
    Almacen() : _name(""), _exists(false) { 
        _exists = load();
    };

    ~Almacen() { 
        delete _config;
        std::list<Repositorio*>::iterator it;
        for (it = _lReposit.begin(); it != _lReposit.end(); ++it)
            delete *it;
    }

    bool addReposit   (const string& a_Name);
    bool removeReposit(const string& a_Name);
    bool create       (const string& a_Name);
    bool remove();
    bool exists() const { return (_name.length() > 0); }

    // getters
    Repositorio* getReposit(const string& a_Name) const;
    string getName() const { return _name; }

    bool createConfigFile(const string& a_Dir);

private:
    bool load() throw(xercesc::XMLException&);

    // member variables
    string                  _name;
    XMLConfigData*          _config;
    std::list<Repositorio*> _lReposit;
    bool                    _exists;
};

#endif

