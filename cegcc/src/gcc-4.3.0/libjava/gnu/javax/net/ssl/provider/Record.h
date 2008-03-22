
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_Record__
#define __gnu_javax_net_ssl_provider_Record__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace net
      {
        namespace ssl
        {
          namespace provider
          {
              class ContentType;
              class ProtocolVersion;
              class Record;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
    }
  }
}

class gnu::javax::net::ssl::provider::Record : public ::java::lang::Object
{

public:
  Record(::java::nio::ByteBuffer *);
  virtual ::gnu::javax::net::ssl::provider::ContentType * getContentType();
  virtual ::gnu::javax::net::ssl::provider::ContentType * contentType();
  virtual jint fragment(::java::nio::ByteBuffer *);
  virtual ::java::nio::ByteBuffer * fragment();
  virtual jint length();
  virtual ::gnu::javax::net::ssl::provider::ProtocolVersion * version();
  virtual void setContentType(::gnu::javax::net::ssl::provider::ContentType *);
  virtual void setLength(jint);
  virtual void setVersion(::gnu::javax::net::ssl::provider::ProtocolVersion *);
  virtual ::java::lang::String * toString();
private:
  ::java::nio::ByteBuffer * __attribute__((aligned(__alignof__( ::java::lang::Object)))) buffer;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_Record__
