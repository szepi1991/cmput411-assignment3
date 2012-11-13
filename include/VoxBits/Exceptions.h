/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_EXCEPTIONS_H
#define VOXBITS_EXCEPTIONS_H

#include <VoxBits/Cloneable.h>
#include <string>

namespace VoxBits
{
  class Exception : virtual public std::exception, virtual public Cloneable
  {
  public:
    Exception() {}
    Exception(const std::string& message ) 
    {
      m_what = message; // we *must* copy the message
    }
    Exception(char const* message ) 
    {
      m_what = message; // we *must* copy the message
    }
    virtual ~Exception() throw() {}
    virtual const char* what()  const throw()
    {
      return m_what.c_str();
    }
    void setMessage( const std::string& msg ) { m_what = msg; }
    MAKE_CLONEABLE(Exception)
  private:
    std::string m_what;
  };

#define VOXBITS_EXC1(T,U) \
  class T : public U \
  { \
  public: \
    T() : U() {} \
    T( const std::string& s ) : U( s ) {} \
    T( const char* m ) : U( m ) {} \
    virtual ~T() throw() {}

#define VOXBITS_EXC2(T) };

VOXBITS_EXC1(CannotInitialize,Exception)        MAKE_CLONEABLE(CannotInitialize) VOXBITS_EXC2(CannotInitialize)
VOXBITS_EXC1(WrongParameter,Exception)          MAKE_CLONEABLE(WrongParameter) VOXBITS_EXC2(WrongParameter)
VOXBITS_EXC1(UnknownIdentifier,Exception)       MAKE_CLONEABLE(UnknownIdentifier) VOXBITS_EXC2(UnknownIdentifier)
VOXBITS_EXC1(UnknownValue,Exception)            MAKE_CLONEABLE(UnknownValue) VOXBITS_EXC2(UnknownValue)
VOXBITS_EXC1(MissingValue,Exception)            MAKE_CLONEABLE(MissingValue) VOXBITS_EXC2(MissingValue)
VOXBITS_EXC1(OutOfBounds,WrongParameter)        MAKE_CLONEABLE(OutOfBounds) VOXBITS_EXC2(OutOfBounds)
VOXBITS_EXC1(OutOfDomain,WrongParameter)        MAKE_CLONEABLE(OutOfDomain) VOXBITS_EXC2(OutOfDomain)
VOXBITS_EXC1(ContractViolation,Exception)       MAKE_CLONEABLE(ContractViolation) VOXBITS_EXC2(ContractViolation)
VOXBITS_EXC1(BadState,Exception)                MAKE_CLONEABLE(BadState) VOXBITS_EXC2(BadState)
VOXBITS_EXC1(LogicError,Exception)              MAKE_CLONEABLE(LogicError) VOXBITS_EXC2(LogicError)

VOXBITS_EXC1(BadResult,Exception)               MAKE_CLONEABLE(BadResult) VOXBITS_EXC2(BadResult)
VOXBITS_EXC1(Numerics,Exception)                MAKE_CLONEABLE(Numerics) VOXBITS_EXC2(Numerics)
VOXBITS_EXC1(NumUnderflow,Numerics)             MAKE_CLONEABLE(NumUnderflow) VOXBITS_EXC2(NumUnderflow)
VOXBITS_EXC1(NumOverflow,Numerics)              MAKE_CLONEABLE(NumOverflow) VOXBITS_EXC2(NumOverflow)
VOXBITS_EXC1(NonConvergence,Numerics)           MAKE_CLONEABLE(Numerics) VOXBITS_EXC2(Numerics)

VOXBITS_EXC1(IOException,Exception)             MAKE_CLONEABLE(IOException) VOXBITS_EXC2(IOException)
VOXBITS_EXC1(BadFormatException,Exception)      MAKE_CLONEABLE(BadFormatException) VOXBITS_EXC2(BadFormatException)
VOXBITS_EXC1(BadFileStructure,Exception)        MAKE_CLONEABLE(BadFileStructure) VOXBITS_EXC2(BadFileStructure)
VOXBITS_EXC1(BadVersionException,Exception)     MAKE_CLONEABLE(BadVersionException) VOXBITS_EXC2(BadVersionException)
VOXBITS_EXC1(NoSystemResources,Exception)       MAKE_CLONEABLE(NoSystemResources) VOXBITS_EXC2(NoSystemResources)
VOXBITS_EXC1(NoSuchResourceException,Exception) MAKE_CLONEABLE(NoSuchResourceException) VOXBITS_EXC2(NoSuchResourceException)

VOXBITS_EXC1(NotImplementedException,Exception) MAKE_CLONEABLE(NotImplementedException) VOXBITS_EXC2(NotImplementedException)

VOXBITS_EXC1(UnknownException,Exception)        MAKE_CLONEABLE(UnknownException) VOXBITS_EXC2(UnknownException)
 
//#undef VOXBITS_EXC1
//#undef VOXBITS_EXC2

}

#endif // VOXBITS_EXCEPTIONS_H
