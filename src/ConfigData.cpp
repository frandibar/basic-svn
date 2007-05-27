// ConfigData.cpp

#include "ConfigData.h"

#include <iostream>
#include <fstream>
#include <string>

using xercesc::DOMDocument;
using xercesc::DOMElement;
using xercesc::DOMException;
using xercesc::DOMImplementation;
using xercesc::DOMNodeList;
using xercesc::DOMNode;
using xercesc::DOMWriter;
using xercesc::LocalFileFormatTarget;
using xercesc::XercesDOMParser;
using xercesc::XMLException;
using xercesc::XMLFormatTarget;
using xercesc::XMLString;
using xercesc::XMLUni;

void removeBlankLines(const string& a_Filename)
{
    std::ifstream is(a_Filename.c_str());
    std::ofstream os("tmp");

    while (!is.eof()) {
        std::string line;
        getline(is, line);
        if (line.find_first_not_of(' ') != string::npos)
            os << line << std::endl;
    }
    is.close();
    os.close();
    rename("tmp", a_Filename.c_str());
}

DOMElement* ElementFinder::getElement(const XMLCh* a_Name) 
{
    DOMElement* result = NULL;
    DOMNodeList* list = _xmlDoc->getElementsByTagName(a_Name);
    DOMNode* node = list->item(0);
    if (node != NULL) {
        if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
            result = dynamic_cast<DOMElement*>(node);
        }
    }
    return result;
}


string XMLConfigData::getDirAlmacen()
{
    DOMElement* e = _finder.getElementAlmacen();
    StringManager sm;
    string dir = sm.convert(e->getAttribute(_finder._ATTR_DIR.asXMLString()));
    return dir;
}


void XMLConfigData::handleElement(const DOMElement* a_Element)
{
    assert(a_Element != NULL);
    if (XMLString::equals(_finder._TAG_ALMACEN.asXMLString(), a_Element->getTagName())) {
        
        // get a list of <repositorio> elements
        DOMNodeList* repositNodes = a_Element->getElementsByTagName(_finder._TAG_REPOSIT.asXMLString());
        const XMLSize_t repositCount = repositNodes->getLength();

        for (XMLSize_t i = 0; i < repositCount; ++i) {
            DOMNode* repositNode = repositNodes->item(i);
            DOMElement* elementReposit = dynamic_cast<DOMElement*>(repositNode);
            StringManager sm;
            string repositName = sm.convert(elementReposit->getAttribute(_finder._ATTR_NAME.asXMLString()));
            addReposit(repositName);
           
            // get a list of <user> elements
            DOMNodeList* userNodes = elementReposit->getElementsByTagName(_finder._TAG_USER.asXMLString());
            const XMLSize_t userCount = userNodes->getLength();

            for (XMLSize_t i = 0; i < userCount; ++i) {
                DOMNode* userNode = userNodes->item(i);
                DOMElement* elementUser = dynamic_cast<DOMElement*>(userNode);
                User u;
                u.username = sm.convert(elementUser->getAttribute(_finder._ATTR_NAME.asXMLString()));
                u.password = sm.convert(elementUser->getAttribute(_finder._ATTR_PASSWORD.asXMLString()));
                u.fullname = sm.convert(elementUser->getAttribute(_finder._ATTR_FULLNAME.asXMLString()));
                addUser(repositName, u);
            }
        }
    } 
}


void XMLConfigData::updateXMLTree()
{
    DOMDocument* xmlDoc = _parser.getDocument();

    // remove all repositories
    DOMElement* root = xmlDoc->getDocumentElement();
    DOMNodeList* repositNodes = root->getElementsByTagName(_finder._TAG_REPOSIT.asXMLString());
    while (repositNodes->getLength() > 0) {
        DOMNode* repositNode = repositNodes->item(0);
        DOMElement* elementReposit = dynamic_cast<DOMElement*>(repositNode);
        root->removeChild(elementReposit);
        elementReposit->release();
    }

    // add repositories
    const int nreps = _repositorios.size();
    for (int i = 0; i < nreps; ++i) {
        StringManager sm;
        DOMElement* newElementReposit = xmlDoc->createElement(_finder._TAG_REPOSIT.asXMLString()) ;
        newElementReposit->setAttribute(_finder._ATTR_NAME.asXMLString(), sm.convert(_repositorios.at(i).first));

        const int nusers = _repositorios[i].second.size();
        for (int j = 0; j < nusers; ++j) {

            DOMElement* element = xmlDoc->createElement(_finder._TAG_USER.asXMLString());

            element->setAttribute(_finder._ATTR_NAME.asXMLString(),     sm.convert(_repositorios[i].second.at(j).username));
            element->setAttribute(_finder._ATTR_PASSWORD.asXMLString(), sm.convert(_repositorios[i].second.at(j).password));
            element->setAttribute(_finder._ATTR_FULLNAME.asXMLString(), sm.convert(_repositorios[i].second.at(j).fullname));
            newElementReposit->appendChild(element);
        }

		DOMElement* elementAlmacen = _finder.getElementAlmacen();
		elementAlmacen->appendChild(newElementReposit);
    }
}


void XMLConfigData::load() throw(std::runtime_error)
{
    _parser.setValidationScheme(XercesDOMParser::Val_Never);
    _parser.setDoNamespaces(false);
    _parser.setDoSchema(false);
    _parser.setLoadExternalDTD(false);
    // _parser.setIncludeIgnorableWhitespace(false);

    try {
        _parser.parse(_xmlFile.c_str());

        // there's no need to free this pointer -- it's
        // owned by the parent parser object
        DOMDocument* xmlDoc = _parser.getDocument();

        if (xmlDoc->getDocumentElement() == NULL) {
            throw(std::runtime_error("empty XML document"));
        }

        _finder.setDocument(xmlDoc);

        handleElement(_finder.getElementAlmacen());	
    }
    catch (XMLException& e) {
       
        // believe it or not, XMLException is not
        // a parent class of DOMException
        std::ostringstream buf;
        buf << "Error parsing file: " << DualString(e.getMessage()) << std::flush;
        throw (std::runtime_error(buf.str()));
    } 
    catch (const DOMException& e) {

        std::ostringstream buf;
        buf << "Encountered DOM Exception: " << DualString(e.getMessage()) << std::flush;
        throw (std::runtime_error(buf.str()));
    }
}


void XMLConfigData::commit() throw(std::runtime_error)
{
    try {
        updateXMLTree();

        // other XMLFormatTarget impls: MemBufFormatTarget, StdOutFormatTarget

        // for good or ill, this ctor creates the file on the spot;
        // so if the program bails before anything is written to the
        // target file, you can have a lot of zero-byte files laying
        // around.
 
        // More robust code would first write to a temp file, then move
        // it into place after a successful write.
        
        XMLFormatTarget* outfile = new LocalFileFormatTarget(DualString(_xmlFile).asXMLString());

        // write it out
        DOMDocument* xmlDoc = _parser.getDocument();
        DOMImplementation* impl = xmlDoc->getImplementation();

        if (impl == NULL)
            throw(std::runtime_error("DOMImplementation is null!"));

        DOMWriter* writer = impl->createDOMWriter();

        // add spacing and such for human-readable output
        if (writer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
            writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
        }

        xmlDoc->normalizeDocument();
        writer->writeNode(outfile, *xmlDoc);

        delete (outfile);
        delete (writer);

        removeBlankLines(_xmlFile);
    }
    catch (const XMLException& e) {

        // believe it or not, XMLException is not
        // a parent class of DOMException

        std::ostringstream buf;
        buf << "Error storing file: "
            << DualString(e.getMessage())
            << std::flush;

        throw(std::runtime_error(buf.str()));
    }
    catch (...) {
        throw(std::runtime_error("unknown exception caught"));
    }
}


bool XMLConfigData::addReposit(const std::string& a_Name)
{
    // check if repository already exists
    RepositoriosList::iterator repIt;
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Name) {
            return false; // reposit already exists
        }
    }

    UsersList users;
    _repositorios.push_back(std::make_pair(a_Name, users));
    return true;
}

bool XMLConfigData::removeReposit(const string& a_Name)
{
    RepositoriosList::iterator repIt;
    RepositoriosList newRepositoriosList;
    // find repository
    bool found = false;
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first != a_Name)
            newRepositoriosList.push_back(*repIt);
        else
            found = true;
    }
    if (!found)
        return false;
    _repositorios = newRepositoriosList;
    return true;
}


bool XMLConfigData::removeUser(const std::string& a_Reposit, const string& a_Username)
{
    RepositoriosList::iterator repIt;
    // find repository
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Reposit) {
            // check that the user doesn't exist
            UsersList newUsersList;
            // add to newUsersList each user except a_User
            UsersList::iterator usIt;
            bool found = false;
            for (usIt = repIt->second.begin(); usIt != repIt->second.end(); ++usIt) {
                if (usIt->username != a_Username)
                    newUsersList.push_back(*usIt);
                else
                    found = true;
            }
            if (!found)
                return false;
            repIt->second = newUsersList;
            return true;
        }
    }
    return false; // repositorio not found
}

bool XMLConfigData::addUser(const std::string& a_Reposit, const User& a_User)
{
    RepositoriosList::iterator repIt;
    // find repository
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Reposit) {
            // check that the user doesn't exist
            UsersList::iterator usIt;
            for (usIt = repIt->second.begin(); usIt != repIt->second.end(); ++usIt) {
                if (usIt->username == a_User.username)
                    return false;  // user already exists
            }
            (repIt->second).push_back(a_User);
            return true;
        }
    }
    return false; // repositorio not found
}

bool XMLConfigData::userExists(const std::string& a_Reposit, const string& a_Username)
{
    RepositoriosList::iterator repIt;
    // find repository
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Reposit) {
            // check that the user doesn't exist
            UsersList::iterator usIt;
            for (usIt = repIt->second.begin(); usIt != repIt->second.end(); ++usIt) {
                if (usIt->username == a_Username)
                    return true;
            }
            return false;
        }
    }
    return false; // repositorio not found
}

XMLConfigData::UsersList XMLConfigData::getUsersList(const string& a_Reposit)
{
    RepositoriosList::iterator repIt;
    // find repository
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Reposit) {
            return repIt->second;
        }
    }
    UsersList emptylist;
    return emptylist; // repositorio not found
}


bool XMLConfigData::repositExists(const string& a_Name)
{
    RepositoriosList::iterator repIt;
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Name) {
            return true;
        }
    }
    return false;
}

bool XMLConfigData::isPasswordValid(const string& a_Reposit, const string& a_Username, const string& a_Password)
// assumes repository and user exist    
{
    RepositoriosList::iterator repIt;
    // find repository
    for (repIt = _repositorios.begin(); repIt != _repositorios.end(); ++repIt) {
        if (repIt->first == a_Reposit) {
            // check that the user doesn't exist
            UsersList::iterator usIt;
            for (usIt = repIt->second.begin(); usIt != repIt->second.end(); ++usIt) {
                if (usIt->username == a_Username)
                    return (usIt->password == a_Password);
            }
            return false; // user not found
        }
    }
    return false; // repositorio not found

}
