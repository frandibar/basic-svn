// debug.h

#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#define DEBUG

#ifdef DEBUG
#   include <iostream>
#   include <string>
#   include "helpers.h"
    using std::cout;
    using std::endl;
    using std::cerr;
    using std::string;
#   define debug(x) cout << "DEBUG: " << x;
#else
#   define debug(x)
#endif

#endif // DEBUG_H_INCLUDED
