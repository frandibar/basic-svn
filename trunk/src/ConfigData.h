#ifndef CONFIGDATA_H_INCLUDED
#define CONFIGDATA_H_INCLUDED

#include "helper-classes.h"
#include "User.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

// xerces includes
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMWriterFilter.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

using xercesc::DOMDocument;
using xercesc::DOMElement;
using xercesc::DOMNode;
using xercesc::DOMNodeList;
using xercesc::XercesDOMParser;

using std::pair;
using std::string;
using std::vector;

// xerces errors
enum {
	ERROR_ARGS = 1,
	ERROR_XERCES_INIT,
	ERROR_PARSE,
	ERROR_XERCES_TERMINATE
};

// takes care of finding elements in the tree.
//
// For code/design purity, perhaps this class should manage the parent parser
// or DOMDocument objects; but that would require some other methods, etc

class ElementFinder 
{
public:

	const DualString _TAG_ALMACEN;
	const DualString _TAG_REPOSIT;
	const DualString _TAG_USER;

	const DualString _ATTR_DIR;
	const DualString _ATTR_NAME;
	const DualString _ATTR_FULLNAME;
	const DualString _ATTR_PASSWORD;

	ElementFinder() :
		// we could have made these static members; but
		// DualString can't call XMLString::transcode() until
		// we've initialized Xerces...
		_TAG_ALMACEN("almacen"),
		_TAG_REPOSIT("repositorio"),
		_TAG_USER("user"),

		_ATTR_DIR("dir"),
		_ATTR_NAME("name") ,
		_ATTR_FULLNAME("fullname") ,
		_ATTR_PASSWORD("password"),
		_xmlDoc(NULL)
	{
		return;
	}

	DOMElement* getElementAlmacen() {
        // We could also have called xml->getFirstChild() and worked
        // with DOMNode objects (DOMDocument is also a DOMNode); but
        // DOMNode only lets us get at the tree using other abstract
        // DOMNodes.  In turn, that would require us to walk the tree
        // and query each node for its name before we do anything with
        // the data.

        // <almacen> element
        DOMElement* element = _xmlDoc->getDocumentElement();
        return element;	
	}

	DOMElement* getElementRepositorio() {
		return getElement(_TAG_REPOSIT.asXMLString());
	}

	DOMElement* getElement(const XMLCh* name);

	void setDocument(DOMDocument* const doc) {
		_xmlDoc = doc;
	}

private:
	DOMDocument* _xmlDoc;
};


class XMLConfigData
{
public:
	typedef vector<User> UsersList;
    typedef vector<pair<string, UsersList> > RepositoriosList;

    // constructor
	XMLConfigData(const string& fromFile) :
        _xmlFile(fromFile)
	{
        load();
	}

	~XMLConfigData() throw() {
		// notice, the calls to XMLString::release are gone
		// that's handled in each DualString's destructor
	}

	void load() throw(std::runtime_error);
	void commit() throw(std::runtime_error);

	bool addRepository   (const string& a_Name);
	bool removeRepository(const string& a_Name);
	bool repositoryExists(const string& a_Name);
    bool addUser   (const string& a_Reposit, const string& a_Username, const string& a_Password, const string& a_Fullname);
    bool removeUser(const string& a_Reposit, const string& a_Username);
    bool userExists(const string& a_Reposit, const string& a_Username);
    bool isPasswordValid(const string& a_Reposit, const string& a_Username, const string& a_Password);
    bool changePassword (const string& a_Reposit, const string& a_Username, const string& a_NewPassword);
    string getDirAlmacen();
    UsersList getUsersList(const string& a_Reposit);
    RepositoriosList getRepositories() const { return _repositorios; };

private:

    bool addUser(const string& a_Reposit, const User& a_User);
	void handleElement(const DOMElement* element);
	void updateXMLTree();

    // member variables
	string           _xmlFile;
	XercesDOMParser  _parser;
	ElementFinder    _finder;
	RepositoriosList _repositorios;

};

#endif
