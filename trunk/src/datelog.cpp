
#include "datelog.h"
#include "debug.h"

#include <iostream>

using std::ios;
using std::cout;
using std::cerr;
using std::endl;

bool DateLog::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("creating DateLog in '" + a_Filename + "'\n");
    _fstream.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_fstream) {
		_fstream.open(a_Filename.c_str(), ios::out | ios::binary);
		_fstream.close();
		_fstream.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
	}

    _isOpen = _fstream.is_open();
    debug("DateLog creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool DateLog::destroy()
{
    debug("destroying DateLog in '" + _filename + "'\n");
    int ret = remove(_filename.c_str());
    debug("DateLog destroy " + string((ret == 0) ? "successfull" : "failed") + "\n");
    return ret == 0;
}

bool DateLog::open(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("opening DateLog in '" + a_Filename + "'\n");
    _fstream.open(a_Filename.c_str(), ios::binary | ios::in | ios::out);
    _isOpen = _fstream.is_open();
    _filename = a_Filename;
    debug("DateLog open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool DateLog::close()
{
    if (!_isOpen)
        return true;

    _fstream.close();
    _isOpen = _fstream.is_open();
    return !_isOpen;
}

long int DateLog::append(const string& a_Username, const string& a_Date, const string& a_Version, const string& a_Filename)
// returns the offset where is was written    
{
    if (!_fstream.is_open())
        return -1;

    _fstream.seekg(0, ios::end);
    _fstream.seekp(0, ios::end);
    
	// get the offset where the archive is copied
   long int pos = _fstream.tellp();

	if (pos < 0) 
        pos = 0;

   _fstream << a_Date << "\t" << a_Filename << "\t" << a_Version << "\t" << a_Username << "\n";
   
   return pos;
}

bool DateLog::showDate(const string& a_Date, int offset)
{
   if(!_fstream.is_open())
      return false;

   _fstream.seekg(offset,ios::beg);
   _fstream.seekp(offset,ios::beg);

   //comienzo a mostrar, al menos va a haber una linea
   string actualDate;
   string strLine;
   char line[1024];
   do{
      _fstream.getline(line,1023);
      strLine = line;
      actualDate = strLine.substr(0,10);
      
      if(actualDate == a_Date)
         cout << strLine << endl;
   }while((actualDate == a_Date)&&(!_fstream.eof()));
   
   return true;   
}

bool DateLog::show(int offset)
{
    if (!_fstream.is_open())
        return false;

    _fstream.seekg(offset, ios::beg);
    _fstream.seekp(offset, ios::beg);

    string line;
    getline(_fstream, line);

    if (_fstream.fail())
        return false;

    cout << line << endl;
    return true;
}

bool DateLog::showAll()
{
    if (!_fstream.is_open())
        return false;

    _fstream.seekg(0, ios::beg);
    _fstream.seekp(0, ios::beg);

    string line;
    getline(_fstream, line);
    while (!_fstream.eof()) {
        cout << line << endl;
        getline(_fstream, line);
    }

    return true;
}
