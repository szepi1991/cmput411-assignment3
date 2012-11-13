/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_GOODPTR_H
#define VOXBITS_GOODPTR_H

#include <new>

namespace VoxBits
{

  /** \brief Throw an exception when ptr is zero.
    *
    * Some old compilers do not throw bad_alloc when new fails; 
    * Programmers are required to put every new invocation into
    * a goodPtr check.
    */
  template <class T>
  inline T * goodPtr(T * ptr)
  {
    if (ptr == 0) throw std::bad_alloc();
    return ptr;
  }

  /// 
  #define GOODPTR(p)     ::VoxBits::goodPtr(p)
}
#endif // VOXBITS_GOODPTR_H
