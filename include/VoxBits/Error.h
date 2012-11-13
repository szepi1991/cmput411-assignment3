/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_ERROR_H
#define VOXBITS_ERROR_H

// include this file for error handling!

#include <VoxBits/Exceptions.h>
#include <VoxBits/ErrorMacros.h>

namespace VoxBits
{

/** \brief Macros for library level exceptions.
  *
  * Library developer throws exceptions, user catches them whenever
  * appropriate.. Decision is to use exception subclassing via
  * multiple inheritence to implement levelled exception handling
  * (libraries derive their own exceptions from a common
  * base exception of the library + a standard VoxBits exception)
  */
//@{
//*! Defines LIB_BASE base exception, derived from VoxBits::Exception */
#define VOXBITS_LIBRARY_EXCEPTION(LIB_BASE)                             \
    class LIB_BASE : public VoxBits::Exception                          \
    {                                                                   \
    public:                                                             \
        LIB_BASE( const std::string& s ) : VoxBits::Exception( s ) {}   \
        LIB_BASE( const char* m )        : VoxBits::Exception( m ) {}   \
        MAKE_CLONEABLE( LIB_BASE )                                      \
    };
//*! Defines EXC exception, derived from LIB_BASE and VB_BASE */
#define VOXBITS_LIB_DERIVED_EXCEPTION(EXC,LIB_BASE,VB_BASE)             \
    class EXC : public LIB_BASE                                         \
              , public VB_BASE                                          \
    {                                                                   \
    public:                                                             \
        EXC( const std::string& s ) : LIB_BASE( s ) , VB_BASE( s ) {}   \
        EXC( const char* m )        : LIB_BASE( m ) , VB_BASE( m ) {}   \
        MAKE_CLONEABLE( EXC )                                           \
    };
//@}

} //namespace VoxBits

#endif // VOXBITS_ERROR_H
