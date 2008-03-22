
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_NamingService_Binding_iterator_impl__
#define __gnu_CORBA_NamingService_Binding_iterator_impl__

#pragma interface

#include <org/omg/CosNaming/_BindingIteratorImplBase.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
      namespace NamingService
      {
          class Binding_iterator_impl;
      }
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace CosNaming
      {
          class Binding;
          class BindingHolder;
          class BindingListHolder;
      }
    }
  }
}

class gnu::CORBA::NamingService::Binding_iterator_impl : public ::org::omg::CosNaming::_BindingIteratorImplBase
{

public:
  Binding_iterator_impl(JArray< ::org::omg::CosNaming::Binding * > *);
  virtual void destroy();
  virtual jboolean next_n(jint, ::org::omg::CosNaming::BindingListHolder *);
  virtual jboolean next_one(::org::omg::CosNaming::BindingHolder *);
private:
  static ::org::omg::CosNaming::Binding * no_more_bindings;
  JArray< ::org::omg::CosNaming::Binding * > * __attribute__((aligned(__alignof__( ::org::omg::CosNaming::_BindingIteratorImplBase)))) bindings;
  jint p;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_NamingService_Binding_iterator_impl__
