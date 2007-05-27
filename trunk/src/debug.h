// debug.h

#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#define DEBUG

#ifdef DEBUG
#   include <iostream>
    using std::cout;
    using std::endl;
    using std::cerr;
#   define debug(x) cout << "DEBUG: " << x;
#else
#   define debug(x)
#endif

#endif // DEBUG_H_INCLUDED
