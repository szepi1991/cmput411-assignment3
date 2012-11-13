/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_CLONEABLE_H
#define VOXBITS_CLONEABLE_H

#include <VoxBits/GoodPtr.h>

namespace VoxBits
{
  /// Base class for all cloneable classes.
  class Cloneable 
  {
  public:
    virtual ~Cloneable() {};
  /** \brief Return a copy of the derived class.
    * \see MAKE_CLONEABLE
    */
    virtual Cloneable* clone() const = 0;
  };
  
  /// Helper macro to define the clone method in Cloneable derived classes.
  #define MAKE_CLONEABLE(T) \
    virtual VoxBits::Cloneable* clone() const \
    { \
    return GOODPTR(new T(*this)); \
    }


} // end of namespace VoxBits

//@}

#endif //VOXBITS_CLONEABLE_H
