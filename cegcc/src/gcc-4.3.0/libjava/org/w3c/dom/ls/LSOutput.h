
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_w3c_dom_ls_LSOutput__
#define __org_w3c_dom_ls_LSOutput__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
        namespace ls
        {
            class LSOutput;
        }
      }
    }
  }
}

class org::w3c::dom::ls::LSOutput : public ::java::lang::Object
{

public:
  virtual ::java::io::Writer * getCharacterStream() = 0;
  virtual void setCharacterStream(::java::io::Writer *) = 0;
  virtual ::java::io::OutputStream * getByteStream() = 0;
  virtual void setByteStream(::java::io::OutputStream *) = 0;
  virtual ::java::lang::String * getSystemId() = 0;
  virtual void setSystemId(::java::lang::String *) = 0;
  virtual ::java::lang::String * getEncoding() = 0;
  virtual void setEncoding(::java::lang::String *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __org_w3c_dom_ls_LSOutput__
