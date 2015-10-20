#ifndef __cmpException_h
#define __cmpException_h

#ifdef _WIN32
#pragma warning(push,3)
#endif

#include <QDebug>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <string>
#include <iostream>

namespace cmp
{

/** \class Exception
    A simple exception class that may be thrown and caught.  The optional text
    string provides additional information about the error.
*/
class Exception
{
public:
  Exception(std::string s) : info(s) {}
  Exception() { info = "no additional information available"; }
  std::string info;
};


inline QDebug operator<<(QDebug dbg, const Exception &e)
{
  dbg.nospace() << e.info;
  
  return dbg.space();
}

inline std::ostream& operator<<(std::ostream& s, const Exception &f)
{ s << "CMP exception: " << f.info; return s; }

} // end namespace cmp

#endif // __cmpException_h
