/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_CLONE_H
#define VOXBITS_CLONE_H

// include this file if you need Cloneable classes, use the clone helper function provided here!
                                           
#include <typeinfo>
#include <VoxBits/Cloneable.h>
#include <VoxBits/Error.h>

namespace VoxBits
{
  /** \brief Clone helper function. 
    *
    * Prefer this function to <tt>xx->clone()<tt>!
    * 
    * The clone function will clone its parameter if the parameter's
    * type is a descendant of Cloneable and otherwise will make a copy
    * of it using the copy constructor of the type of its argument.
    * 
    * Useful when working with templates.
    * 
    * @parameter t pointer to the object to be cloned.
    * 
    * @exception bad_alloc	
    * @exception bad_cast		
    * @return pointer to the cloned object.
    */  
  template <class T> 
  T* clone( const T* toClone )
  {
    REQUIRE( toClone!=0, ::VoxBits::ContractViolation, "Caller's mistake" );
    const Cloneable* ptr = 0;
    ptr = dynamic_cast< const VoxBits::Cloneable* >( toClone );
    if (ptr == 0)  // deliberately new line to help debugging
      return GOODPTR( new T( *toClone ) );
    T* s = dynamic_cast<T*>( ptr->clone() );
    if(s == 0) 
      throw std::bad_cast();
    return s;
  }


} // end of namespace VoxBits

//@}

#endif // VOXBITS_CLONE_H
