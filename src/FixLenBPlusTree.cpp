//fixlenbplustree.cpp

#include "FixLenBPlusTree.h"
#include "debug.h"

using std::ios;

FixLenBPlusTree::FixLenBPlusTree() : BPlusTree()
{
   _raiz = 0;
   _nodoActual = 0;
   _buffer = new char[FixLenNode::FIXLEN_NODE_SIZE];
}

FixLenBPlusTree::~FixLenBPlusTree()
{
   delete _buffer;
   delete _nodoActual;
   delete _raiz;
}

bool FixLenBPlusTree::open(const string& a_Filename)
{
    if (_isOpen) 
        return false;

    debug("opening bplustree '" + a_Filename + "'\n");
    _filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
    _isOpen = _filestr.is_open() && readHeader();

    if (_nNodos > 0)
        _isOpen = _isOpen && readRoot();

    if (_isOpen) {
        _nodoActual = _raiz;
        _filename = a_Filename;
    }
    debug("bplustree open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool FixLenBPlusTree::close()
{
    if (!_isOpen)
        return true;

    bool ret = true;
    debug("closing bplustree " + _filename + "\n");
    if (writeHeader()) {
        if((_raiz != 0)  && writeRoot())
         if (_raiz != _nodoActual)
            ret = writeNode(_nodoActual);
    }
    else 
        ret = false;

    _filestr.close();
    _isOpen = _filestr.is_open();
    debug("bplustree close " + string((!_isOpen && ret) ? "successfull" : "failed") + "\n");
    return (!_isOpen && ret);
}

bool FixLenBPlusTree::insert(const char* key,int reference)
{
	FixLenLeafNode* hojaNueva;
    if (_nNodos) {
        int nodoReceptor = searchPlace(key);
        if (nodoReceptor) {              // voy a insertar en un nodo != _raiz
            switch (_nodoActual->insert(key, reference)) {
            case 1:
                return true;
				break;

            case 2:                 
                int nroNuevoNodo;
                    
                // creo una nueva hoja para insertar la clave que no entra por overflow
                hojaNueva = new FixLenLeafNode(_nNodos, _nodoActual->getPadre(),_nodoActual->getId(), -1);
                
				// le inserto la clave que me sobro
                hojaNueva->insert(key, reference);

				(static_cast<FixLenLeafNode*>(_nodoActual))->setHnoDerecho(hojaNueva->getId());

                // incremento la cantidad de nodos
                _nNodos++;
                    
                nroNuevoNodo = hojaNueva->getId();

                // escribo los dos nodos
                writeNode(_nodoActual);
                writeNode(hojaNueva);

                delete hojaNueva;

                insertarEnPadre(_nodoActual->getPadre(), nroNuevoNodo, key);
                return true;
				break;

            default:
                return false;
				break;
            }
        }
        else {
            FixLenIndexNode* nuevaRaiz;
            int idH1;
            int idH2;
            FixLenLeafNode* nuevaHoja;
            FixLenLeafNode* nuevaHoja2;
            switch (_raiz->insert(key, reference)) {
            case 1:
                return true;

            case 2:
                nuevaHoja = 0;
                nuevaHoja2 = 0;
                
                _raiz->promoteRoot((FixLenNode**)(&nuevaHoja),_nNodos);
				    nuevaHoja2 = new FixLenLeafNode(_nNodos + 1, _raiz->getId(), nuevaHoja->getId(), -1);
				    nuevaHoja->setHnoDerecho(nuevaHoja2->getId());
                // inserto la clave que me genero el overflow en la nueva hoja
                nuevaHoja2->insert(key,reference);
				
                _nNodos += 2;

                // escribo las 2 nuevas hojas
                writeNode(nuevaHoja);
                writeNode(nuevaHoja2);

                idH1 = nuevaHoja->getId();
                idH2 = nuevaHoja2->getId();

                nuevaRaiz = (static_cast<FixLenLeafNode*>(_raiz))->convertirAIndice(idH1,idH2,key);

                delete(_raiz);

                _nodoActual = _raiz = nuevaRaiz;

                return true;

            default:
                return false;
            }
        }
    }

    else {   
        _raiz = new FixLenLeafNode();
        _nNodos++;
        _nodoActual = _raiz;
        _raiz->insert(key, reference);
    }
    return true;
}

bool FixLenBPlusTree::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, FixLenNode::FIXLEN_NODE_SIZE);
        // leo la cantidad de nodos
        memcpy(&_nNodos, nextByte, sizeof(int));
        return true;
    }
    return false;
}

bool FixLenBPlusTree::writeHeader()
{
    if (_filestr.is_open()) {
        // clear buffer
        memset(_buffer,'\0', FixLenNode::FIXLEN_NODE_SIZE);
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_nNodos, sizeof(int));
        // volcar al archivo
        _filestr.seekg(0, ios::beg);
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, FixLenNode::FIXLEN_NODE_SIZE);
        return true;
    }
    return false;
}

bool FixLenBPlusTree::readRoot()
{
    if (_filestr.is_open()) {
        _filestr.seekg(FixLenNode::FIXLEN_NODE_SIZE, ios::beg);
        _filestr.read(_buffer, FixLenNode::FIXLEN_NODE_SIZE);

        if (_nNodos > 1)
            _raiz = new FixLenIndexNode();
        else
            _raiz = new FixLenLeafNode();

        _raiz->read(_buffer);
        return true;
    }
    return false;
}

bool FixLenBPlusTree::writeRoot()
{
    if (_filestr.is_open()) {
        if (_raiz != 0)
            _raiz->write(_buffer);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.seekg(FixLenNode::FIXLEN_NODE_SIZE, ios::beg);
        _filestr.seekp(FixLenNode::FIXLEN_NODE_SIZE, ios::beg);
        _filestr.write(_buffer, FixLenNode::FIXLEN_NODE_SIZE);
        return true;
    }
    return false;
}

bool FixLenBPlusTree::readNode(int id, Node** node)
{
    if (_filestr.is_open()) {
        if ((*node != _raiz) && (*node != 0)) {
            writeNode(*node);
            delete (*node);
        }
        else
            *node = 0;
        
        _filestr.seekg(FixLenNode::FIXLEN_NODE_SIZE * (id + 1), ios::beg);
        _filestr.read(_buffer, FixLenNode::FIXLEN_NODE_SIZE);

        int nivelNodo;
        memcpy(&nivelNodo, _buffer, sizeof(int));

        if (nivelNodo != 0)
            (*node) = new FixLenIndexNode();
        else
            (*node) = new FixLenLeafNode();

        (*node)->read(_buffer);
        return true;
    }
    return false;
}

bool FixLenBPlusTree::writeNode(Node* node)
{
    if (_filestr.is_open()) {
        node->write(_buffer);

        if(_filestr.fail())
            _filestr.clear();

        _filestr.seekp(FixLenNode::FIXLEN_NODE_SIZE * (node->getId() + 1), ios::beg);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.write(_buffer, FixLenNode::FIXLEN_NODE_SIZE);
        return true;
    }
    return false;
}

void FixLenBPlusTree::insertarEnPadre(int idNodoPadre, int idNodoHijo,const char* claveAlPadre)
{
   FixLenIndexNode* nuevoIndice = 0;
   FixLenIndexNode* nuevoIndice2 = 0;
	FixLenIndexNode* nuevaRaiz = 0;
    
    if (idNodoPadre != 0) { // el padre es distinto de la raiz
        readNode(idNodoPadre, (Node**)&_nodoActual);

        if (_nodoActual->insert(claveAlPadre, idNodoHijo) == 2) {
            nuevoIndice = new FixLenIndexNode(_nNodos, _nodoActual->getNivel(), 
											_nodoActual->getPadre(), idNodoHijo);

            _nNodos++;
            // escribo los nodos en disco
            writeNode(_nodoActual);
            writeNode(nuevoIndice);

            // actualizo el puntero al padre en los hijos del nuevo nodo indice
            actualizarPadre(nuevoIndice);

            insertarEnPadre(nuevoIndice->getPadre(), nuevoIndice->getId(),claveAlPadre);

			delete nuevoIndice;

            return;
        }
        return;
    }

    // si llego hasta aca es porque el que esta "rebalsado" es la raiz
    
    if (_raiz->insert(claveAlPadre, idNodoHijo) == 2) {

        _raiz->promoteRoot( (FixLenNode**)(&nuevoIndice),
                            _nNodos);

        nuevoIndice2 = new FixLenIndexNode(_nNodos + 1, _raiz->getNivel(), _raiz->getId(), idNodoHijo);

        nuevaRaiz = new FixLenIndexNode(_raiz->getId(), _raiz->getNivel() + 1, 
						_raiz->getPadre(), nuevoIndice->getId(), claveAlPadre, nuevoIndice2->getId());

		if(_nodoActual != _raiz) {		
			delete _raiz;
			_raiz = nuevaRaiz;
		}

		else {
			delete _raiz;
			_raiz = _nodoActual = nuevaRaiz;
		}
				
        writeNode(nuevoIndice);
        writeNode(nuevoIndice2);

        _nNodos += 2;

        actualizarPadre(nuevoIndice);
        actualizarPadre(nuevoIndice2);
        
        if (nuevoIndice) delete nuevoIndice;
        if (nuevoIndice2) delete nuevoIndice2;
    }
}

bool FixLenBPlusTree::actualizarPadre(FixLenIndexNode* padre)
{
    // le seteo el padre al hijo izquierdo
    readNode(padre->getHijoIzquierdo(), (Node**)&_nodoActual);
    _nodoActual->setPadre(padre->getId());

    for(int i = 0; i < padre->getCantClaves(); ++i) {
        readNode(padre->getRef(i), (Node**)&_nodoActual);
        _nodoActual->setPadre(padre->getId());
    }

    return writeNode(_nodoActual);
}

int FixLenBPlusTree::searchPlace(const char* key)
{
    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    return searchPlaceRec(key);
}

int FixLenBPlusTree::searchPlaceRec(const char* key)
{
    if (_nodoActual->getType() == FixLenNode::LEAF)
        return _nodoActual->getId();
    else {
        int indice = _nodoActual->search(key);

        if (readNode(indice, (Node**)&_nodoActual))
            return searchPlaceRec(key);

        return -1;
    }
}

int FixLenBPlusTree::search(const char* key)
{
    if (isEmpty())
        return -1;

    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    while (_nodoActual->getType() != FixLenNode::LEAF) {
        int proximoALeer = _nodoActual->search(key);
        readNode(proximoALeer, (Node**)&_nodoActual);
    }

    return _nodoActual->search(key);
}
