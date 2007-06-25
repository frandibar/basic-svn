#include "Node.h"

#include <iostream>

// constructor
Node::Node(int id, int nivel, int padre)
{
    _nivel        = nivel;
    _id           = id;
    _padre        = padre;
    _nclaves      = 0;
}
