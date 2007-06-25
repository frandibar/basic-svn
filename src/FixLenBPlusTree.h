//fixlenbplustree.h

#ifndef FIXLENBPLUSTREE_H_INCLUDED
#define FIXLENBPLUSTREE_H_INCLUDED

#include "bplustree.h"
#include "FixLenNode.h"
#include "FixLenIndexNode.h"
#include "FixLenLeafNode.h"

class FixLenBPlusTree : public BPlusTree
{
public:
   FixLenBPlusTree();
   virtual ~FixLenBPlusTree();

   bool open(const string& a_Filename);
   bool close();
   
   int search(const char* key);

   bool insert(const char* key,int reference);
   
protected:
   bool readHeader();
   bool writeHeader();

   bool readRoot();
   bool writeRoot();

   bool readNode(int id, Node** node);
   bool writeNode(Node* node);

   int searchPlace(const char* key);
   
   int searchPlaceRec(const char* key);

   void insertarEnPadre(int NroNodoPadre, int NroNodoHijo, const char* claveAlPadre);
   
   bool actualizarPadre(FixLenIndexNode* padre);

   bool isEmpty() const { return _raiz == 0; }

private:
   FixLenNode* _raiz;
   FixLenNode* _nodoActual;
};

#endif
