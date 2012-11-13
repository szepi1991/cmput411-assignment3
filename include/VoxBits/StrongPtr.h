/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_STRONGPTR_H
#define VOXBITS_STRONGPTR_H

#include <VoxBits/Clone.h>
#include <memory>             // for std::auto_ptr

namespace VoxBits
{

//note: a workaround using boost template metaprogramming
//is included at the end of the file showing how
//to implement 'partial template specialization'
//based on a bool template argument - here
//this could be used to chooce between the two ways of cloning.
//Such an implementation would allow one to use
//strong_ptr with non-polymorphic objects - 
//presently this is not possible.


  /// Smart pointer supporting copy-always semantics for polymorph objects
  /** \brief If polymorphic is set to true then pointee is cloned.
    *
    * Requirements on T:
    * - T cannot be abstract
    * - T must be polymorphic (has at least one virtual method)
    */
  template <class T, bool polymorphic=true>
  class strong_ptr 
  {
    typedef T             element_type;
    typedef strong_ptr<T> this_type;

  public:
    explicit 
             strong_ptr( T* t = 0 ) : ptr( t )   
             { 
             }
             strong_ptr( const this_type& p )   
             { 
               ptr = clone( p.ptr ); 
             }
             strong_ptr( const T& p )               
             { 
               ptr = clone( &p ); 
             }
#ifndef BOOST_NO_AUTO_PTR
    template<class Y>
    explicit strong_ptr( std::auto_ptr<Y>& r): ptr(r.release()) 
    { 
    }
#endif 

    virtual ~strong_ptr()                           
    { 
      delete ptr; 
    }

    this_type& operator=( const this_type& p )
            {
              if (p.ptr!=ptr) 
              {
                T* cloned;
                if (polymorphic) cloned = clone( p.ptr );
                else             cloned = new T(*p.ptr);
                delete ptr;
                ptr = cloned;
              }
              return *this;
            }

#ifndef BOOST_NO_AUTO_PTR
  template<class Y>
   this_type& operator=(std::auto_ptr<Y> & r)
              {
                this_type(r).swap(*this);
                return *this;
              }
#endif


    void    reset( T* t = 0 ) 
            {
              if (ptr==t) return;
              delete ptr;
              ptr = t;
            }

    const T& operator*()  const { VOXBITS_ASSERT( ptr!=0 ); return *ptr; }
          T& operator*()        { VOXBITS_ASSERT( ptr!=0 ); return *ptr; }
          T* operator->()       { VOXBITS_ASSERT( ptr!=0 ); return  ptr; }
    const T* operator->() const { VOXBITS_ASSERT( ptr!=0 ); return ptr; }
          T* get()              { return ptr; }
    const T* get()  const       { return ptr; }

    friend 
       bool operator==( this_type const& p1, this_type const& p2 ) {	return p1.ptr==p2.ptr;	}

    friend 
       bool operator!=( this_type const& p1, this_type const& p2 ) {	return p1.ptr!=p2.ptr;	}

    bool    operator==( const T* p )  {	return ptr==p;	}
            operator bool()     const { return ptr!=0; }
            operator const T*() const { return ptr; }

  inline
     void   swap( this_type& that ) 
            {
              std::swap( ptr, that.ptr ); 
            }		
  private:

    virtual T* clone( const T* p )
    {
      if (p==0) return 0;
      if (polymorphic) 
        return VoxBits::clone(p);
      return GOODPTR(new T(*p));  
    }

  private:

    T * ptr;
  };

  /// Smart pointer supporting copy-always semantics for non-polymorph objects.
  /** \warning Never(!!!) use this smart pointer with class hierarchies!
    * Typical usage will be when a class is initialized in its constructor,
    * we want to declare a member of the given type
    * and the parameters of construction are not known at the construction
    * of the enclosing type.
    */
  template <class T>
  class strong_ptr_np
  {
    typedef T             element_type;
    typedef strong_ptr_np<T> this_type;

  public:
    explicit 
             strong_ptr_np( T* t = 0 ) : ptr( t )   
             { 
             }
             strong_ptr_np( const this_type& p )   
             { 
               if (p.ptr!=0)
                 ptr = new T( *p.ptr ); 
               else
                 ptr = 0;
             }
             strong_ptr_np( const T& p )               
             { 
               ptr = new T( p ); 
             }
#ifndef BOOST_NO_AUTO_PTR
    template<class Y>
    explicit strong_ptr_np( std::auto_ptr<Y>& r): ptr(r.release()) 
    { 
    }
#endif 

    virtual ~strong_ptr_np()                           
    { 
      delete ptr; 
    }

    this_type& operator=( const this_type& p )
            {
              if (p.ptr!=ptr) 
              {
                T* cloned;
                cloned = new T(*p.ptr);
                delete ptr;
                ptr = cloned;
              }
              return *this;
            }

#ifndef BOOST_NO_AUTO_PTR
  template<class Y>
   this_type& operator=(std::auto_ptr<Y> & r)
              {
                this_type(r).swap(*this);
                return *this;
              }
#endif


    void    reset( T* t = 0 ) 
            {
              if (ptr==t) return;
              delete ptr;
              ptr = t;
            }

    const T& operator*()  const { VOXBITS_ASSERT( ptr!=0 ); return *ptr; }
          T& operator*()        { VOXBITS_ASSERT( ptr!=0 ); return *ptr; }
          T* operator->()       { VOXBITS_ASSERT( ptr!=0 ); return  ptr; }
    const T* operator->() const { VOXBITS_ASSERT( ptr!=0 ); return ptr; }
          T* get()              { return ptr; }

    friend 
       bool operator==( this_type const& p1, this_type const& p2 ) {	return p1.ptr==p2.ptr;	}

    friend 
       bool operator!=( this_type const& p1, this_type const& p2 ) {	return p1.ptr!=p2.ptr;	}

    bool    operator==( const T* p )  {	return ptr==p;	}
            operator bool()     const { return ptr!=0; }
            operator const T*() const { return ptr; }

  inline
     void   swap( this_type& that ) 
            {
              std::swap( ptr, that.ptr ); 
            }		

  private:

    T * ptr;
  };


} // end namespace VoxBits


// EXAMPLE--------------
/*
#include "boost/mpl/apply_if.hpp"
#include "boost/mpl/bool.hpp"
#include "boost/mpl/identity.hpp"
#include "boost/mpl/assert_is_same.hpp"
#include "boost/static_assert.hpp"
#include "boost/type_traits/is_base_and_derived.hpp"

namespace mpl = boost::mpl;

struct Base2 {};
struct Base {};
struct Derived1 : public Base {};
struct Derived2 : public Derived1, public Base2 {};

struct Unrelated {};

template <class T>
struct PolymorphicCopier
{
  static void f() 
  { 
    BOOST_STATIC_ASSERT((::boost::is_base_and_derived<Base,T>::value));
    std::cout << "polymorphic\n";
  }
};

template <class T>
struct NonPolymorphicCopier
{
  static void f()
  { 
    std::cout << "non-polymorphic\n"; 
  }
};

template <class T,bool isPolymorphic>
class A
{
public:
  typedef typename boost::mpl::if_c<
          isPolymorphic
        , PolymorphicCopier<T>
        , NonPolymorphicCopier<T>
        >::type Copier;
//  Copier copier;
  void f() { Copier::f(); }
  // could actually use MPL and Type Traits 
  // to implement clone in a better way:
  // Cloner struct would define appropriate Copier based on whether
  // its template argument is derived from Cloneable.
  // 
};

int main()
{
  A<Derived2,true> a1;
  A<int,false> a2;
  a1.f();
  a2.f();

  //A<Unrelated,true> a3;
  //a3.f();

  //A<int,true> a4;
  //a4.f();

  return 0;
}

*/


#endif // VOXBITS_STRONGPTR_H
