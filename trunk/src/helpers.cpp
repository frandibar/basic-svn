// helpers.cpp

#include "helpers.h"

#include <cstdlib>      // rand
#include <fstream>
#include <sys/stat.h>

std::string zeroPad(int number, int ndigits)
{
    std::string ret(ndigits, '0');
    std::string num = toString<int>(number);

    int j = 0;
    for (int i = ndigits - num.length(); i < ndigits; ++i) {
        ret[i] = num[j];
        j++;
    }
    return ret;
}

std::string randomFilename(const std::string& prefix)
// returns a string with a random integer following the prefix    
{
    // a call to seed with a time_t could be used
    // if not, this is quite trivial
    std::string filename(prefix);
    int i = rand();
    return filename + toString(i);
}

t_filetype getFiletype(const std::string& filename)
{
    struct stat ss;
    if (stat(filename.c_str(), &ss) == -1)
        return INVALID; // error, file does not exist

    if ((ss.st_mode & S_IFMT) == S_IFDIR)
        return DIRECTORY;

    // determine if filename is a text or binary file
    // by reading first 512 bytes and searching for chars < 30
    // text files have majority of bytes > 30 while binary files don't
    std::ifstream is(filename.c_str());
    if (!is) return INVALID; // error, could not open file

    int bytesRead = 0;
    int nChars    = 0;
    int nNonchars = 0;
    while (!is.eof() && bytesRead++ < 512) {
        char c;
        is >> c;
        (c > 30) ? nChars++ : nNonchars++;
    }
    is.close();
    return (nChars > nNonchars) ? TEXT : BINARY;
}

int countComponents(const std::string& a_Target)
{
  int cantComponentes = 0;
  unsigned int length = a_Target.length();
  unsigned int index = 0;

  while(index < length)
    {
      index = a_Target.find_first_of("/",index);      
      cantComponentes++;
      if(index < length)
	index++;
    }

  return cantComponentes;
}

std::string getComponent(const std::string& a_Target,int component)
{
	int cantidadComponentes = countComponents(a_Target);
	
	std::string res;	

	if(component > cantidadComponentes)
 		res = "";
	
	else
	{
		int componenteLeida = 0;
		unsigned int comienzo = 0;
		unsigned int fin = 0;
	

		while(componenteLeida != component)
		{
			comienzo = fin;
			fin = a_Target.find_first_of("/",fin);
			componenteLeida++;
			
			if(fin < a_Target.length())
				fin++;
		}

		res = a_Target.substr(comienzo,fin - comienzo - 1);		
	}

	return res;	
}


bool isEmptyFile(const std::string& a_Filename)
{
    std::ifstream is(a_Filename.c_str());
    if (!is.is_open())
        return true;

    std::string line;
    getline(is, line);
    if (!is.eof())
        return false;        
    return line.empty();
}


bool areDifferentFiles(const std::string& f1, const std::string& f2)
// returns true if f1 and f2 are different
// assumes both files exist    
{
    std::string tmp = randomFilename(".tmp_");
    system(("diff " + f1 + " " + f2 + " > " + tmp).c_str());
    bool empty = !isEmptyFile(tmp); 
    remove(tmp.c_str());
    return !empty;
}
