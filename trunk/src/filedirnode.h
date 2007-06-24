//filedirnode.h

#ifndef FILEDIRNODE_H_INCLUDED
#define FILEDIRNODE_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <string>

#include "varlennode.h"

class FileDirNode : public VarLenNode 
{
public:
   // constructors
   FileDirNode(int id = 0, int nivel = 0, int padre = -1);

   virtual int searchFileAndVersion(const char* fileName, int version) = 0;
   virtual int getFirstOf(const char* key) = 0;
};
#endif
