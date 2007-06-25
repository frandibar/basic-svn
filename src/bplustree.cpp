//bplustree.cpp

#include "bplustree.h"
#include "debug.h"

using std::ios;

BPlusTree::BPlusTree() : _nNodos(0), _isOpen(false)
{
}

BPlusTree::~BPlusTree()
{
   delete _buffer;
}

bool BPlusTree::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    debug("creating bplustree in " + a_Filename + "\n");
    _filestr.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_filestr.is_open()) {
		_filestr.open(a_Filename.c_str(), ios::out | ios::binary);
		_filestr.close();

		_filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
		if (!_filestr.is_open()) {
            _isOpen = false;
            debug("bplustree creation failed\n");
			return false;
        }
	}

    _nNodos = 0;
    _isOpen = writeHeader();
    debug("bplustree creation " + string(_isOpen ? "successfull" : "failed") + "\n");

    return _isOpen;
}

bool BPlusTree::destroy()
{
    if (_isOpen)
        return false;

    debug("destroying bplustree '" + _filename + "'\n");
    int ret = remove(_filename.c_str());
    debug("arbolbmas bplustree " + string((ret == 0) ? "successfull" : "failed") + "\n");
    return (ret == 0);
}

