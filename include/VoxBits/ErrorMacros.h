/*************************************************************************
*                                                                        *
*  (C) Copyright 2002 VoxBits and affiliates.                            *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by VoxBits and affiliates.                                  *
*                                                                        *
*************************************************************************/
#ifndef VOXBITS_ERRORMACROS_H
#define VOXBITS_ERRORMACROS_H

#include <stdexcept>
#include <string>
#include <sstream>

/** \brief Define REQUIRE, ENSURE, INVARIANT, ASSERT, FAIL and ERROR_IF, VOXBITS_TRY,VOXBITS_CATCH macros.
  *
  * Library developer throws exceptions, user catches them whenever
  * appropriate.. Decision is to use exception subclassing via
  * multiple inheritence to implement levelled exception handling
  * (libraries derive their own exceptions from a common
  * base exception of the library + a standard VoxBits exception)
  *
  * It is the responsability of the library user to log errors
  * upon catching them! These macros do not support logging.
  * (They could?)
  */
//@{

// some helper macros first

#define VOXBITS_CHECK_DETAILS2( cond, exc, detail ) \
  if (!(cond)) \
  { \
    std::ostringstream errmsg;                          \
    errmsg << "**************Exception**************\n" \
              "File: " << __FILE__ << "\n"              \
              "Line: " << __LINE__ << "\n"              \
              "Cause: " #cond " does not hold\n"        \
              "Details: " << detail << "\n";            \
    throw exc( errmsg.str() );                          \
  }

#define VOXBITS_CHECK_DETAILS( exc, detail ) \
  { \
    std::ostringstream errmsg;                          \
    errmsg << "**************Exception**************\n" \
              "File: " << __FILE__ << "\n"              \
              "Line: " << __LINE__ << "\n"              \
              "Details: " << detail << "\n";            \
    throw exc( errmsg.str() );                          \
  }

// concepts borrowed from "programming by contract" (a la Eiffel)

/// Check caller's responsibility (a la precondition)
#define REQUIRE( cond, exc, detail ) VOXBITS_CHECK_DETAILS2( cond, exc, detail )

/// Check methods's internal responsibility (a la postcondition)
#define ENSURE( cond, exc, detail ) VOXBITS_CHECK_DETAILS2( cond, exc, detail )

/// Check methods's internal responsibility
#define INVARIANT( cond, exc, detail ) VOXBITS_CHECK_DETAILS2( cond, exc, detail )

/// Check "vis maior" conditions (memory exhausted)
#define VB_ASSERT( cond, exc, detail ) VOXBITS_CHECK_DETAILS2( cond, exc, detail )

/// 
#define VOXBITS_ASSERT( cond ) VOXBITS_CHECK_DETAILS2( cond, ::VoxBits::LogicError, "VoxBits assertion failed" )

/// unconditional failure (throws exception provided)
#define FAIL( exc, detail ) VOXBITS_CHECK_DETAILS( exc, detail )

/** \brief Macro ERROR_IF.
  *
  * ERROR_IF is to be used with local error handling (error handling close
  * to the space where the problem arose).
  * 
  * Example:
  * \code
  * ERROR_IF(a!=0)
  * {
  *    // error handling
  * }
  * \endcode
  * 
  * ERROR_IF's behavior is controlled by whether VOXBITS_DEBUG
  * is defined.
  *
  * If VOXBITS_DEBUG is defined, ERROR_IF throws an exception
  * when the error condition holds (so in this case the error handling
  * block following ERROR_IF does not execute at all.
  * 
  * On the other hand, if <TT>VOXBITS_DEBUG</TT> is undefined, <TT>ERROR_IF</TT> works
  * like a normal <TT>if</TT> statement.
  * 
  * The rationale is as follows: local error handling is applied when the error
  * can be corrected locally. However, this is still an error (even if it can be handled
  * locally), so it may help if in "debug" mode an exception is thrown.
  *
  * Todo: In the non-debug mode one still might want to include code for tracing.
  *
  */


#if defined(VOXBITS_DEBUG)

# define ERROR_IF( cond, exc, detail ) \
    ASSERT( !(cond), exc, detail ) \
    if (cond) 

#else

# define ERROR_IF( cond ) \
    if (cond)

#endif

#ifdef  NDEBUG
#define VOXBITS_TRY try
#define VOXBITS_CATCH(x1,x2) catch(const x1& x2)
#else
#define VOXBITS_TRY if (0) {} else
#define VOXBITS_CATCH(x1,x2) if (1) {} else for (x1 x2("");;)
#endif


//@}

#endif // VOXBITS_ERRORMACROS_H
