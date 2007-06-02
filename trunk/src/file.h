#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <stdlib.h>
#include <string>

class File{

public:
	File();
	File(char* name,int versionNumber,char type);

	~File();

	char*	getName()		{	return	_name;			}
	int		getVersion()	{	return _versionNumber;	}
	char	getType()		{	return _type;			}

	int		getTamanioEnDisco();

	void	read(char** buffer);
	void	write(char** buffer);

private:

	char*	_name;
	int		_versionNumber;
	char	_type;
};

#endif

