// userlog.cpp

#include "UserLog.h"
#include "debug.h"
#include "helpers.h"

#include <ctime>
#include <iostream>

using std::ios;
using std::cout;
using std::cerr;
using std::endl;

bool UserLog::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("creating UserLog in '" + a_Filename + "'\n");
    _fstream.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_fstream) {
		_fstream.open(a_Filename.c_str(), ios::out | ios::binary);
		_fstream.close();
		_fstream.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
	}

    _isOpen = _fstream.is_open();
    debug("UserLog creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool UserLog::destroy()
{
    debug("destroying UserLog in '" + _filename + "'\n");
    int ret = remove(_filename.c_str());
    debug("UserLog destroy " + string((ret == 0) ? "successfull" : "failed") + "\n");
    return ret == 0;
}

bool UserLog::open(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("opening UserLog in '" + a_Filename + "'\n");
    _fstream.open(a_Filename.c_str(), ios::binary | ios::in | ios::out);
    _isOpen = _fstream.is_open();
    _filename = a_Filename;
    debug("UserLog open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool UserLog::close()
{
    if (!_isOpen)
        return true;

    _fstream.close();
    _isOpen = _fstream.is_open();
    return !_isOpen;
}

long int UserLog::append(const string& a_Username,time_t a_Date, const string& a_Action)
// returns the offset where is was written    
{
    if (!_fstream.is_open())
        return -1;

    _fstream.seekg(0, ios::end);
    _fstream.seekp(0, ios::end);
    
	// get the offset where the archive is copied
   long int pos = _fstream.tellp();

	if (pos < 0) pos = 0;
   
   tm* date = localtime(&a_Date);
   int anio = date->tm_year + 1900;
   int mes = date->tm_mon + 1;
   int dia = date->tm_mday;

   string fecha = toString<int>(anio) + "/" + zeroPad(mes,2) + "/" + zeroPad(dia,2);

   _fstream<<a_Username<<"\t"<<fecha<<"\t"<<a_Action<<"\n";
   
   return pos;
}

bool UserLog::showAction(int offset)
{
   if(!_fstream.is_open())
      return false;

   _fstream.seekg(offset,ios::beg);
   _fstream.seekp(offset,ios::beg);

   //comienzo a mostrar, al menos va a haber una linea
   string actualDate;
   string strLine;
   char line[1024];
   
   if(!_fstream.fail())
   {
      _fstream.getline(line,1023);
      strLine = line;
      cout<<strLine<<endl;

      return true;
   }

   return false;         
}

