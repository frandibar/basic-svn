// File.h

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <string>
#include <cstdlib>

class File
{
public:
	File();
	File(const char* name, int versionNumber,char type);

	~File();

	char*	getName()		{	return	_name; }
	int	getVersion()	{	return _versionNumber;	}
	char	getType()		{	return _type;	}

	void	setVersion(int Version)	{ _versionNumber = Version; }

	int	getTamanioEnDisco();

	void	read(char** buffer);
	void	write(char* buffer);

private:

	char*	_name;
	int	_versionNumber;
	char	_type;
};

#endif

