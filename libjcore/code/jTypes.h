/******************************************************************************
 jTypes.h

	Defines portable types

	Copyright @ 1994-97 by John Lindal. All rights reserved.

 ******************************************************************************/

#ifndef _H_jTypes
#define _H_jTypes

#if !defined _J_UNIX && !defined ACE_LACKS_PRAGMA_ONCE
#pragma once
#endif

#include <iostream>	// templates
using std::ios;
using std::istream;
using std::ostream;
using std::iostream;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ws;

#include <limits.h>
#include <float.h>

#ifndef NULL
#define NULL	0L
#endif

#ifdef _MSC_VER
#pragma warning ( disable : 4786 4660 4661 )
#endif

	// general types

typedef char								JCharacter;
typedef JCharacter							JCharacter8;

const JCharacter kJCharacterMin				= CHAR_MIN;
const JCharacter kJCharacterMax				= (JCharacter) CHAR_MAX;	// cast required for VCPP

typedef unsigned short						JCharacter16;

const JCharacter16 kJCharacter16Min			= 0;
const JCharacter16 kJCharacter16Max			= (JCharacter16) USHRT_MAX;	// cast required for VCPP

typedef long								JInteger;

const JInteger kJIntegerMin					= LONG_MIN;
const JInteger kJIntegerMax					= LONG_MAX;

typedef unsigned long						JUInt;

const JUInt kJUIntMin						= 0;
const JUInt kJUIntMax						= ULONG_MAX;

typedef double								JFloat;

const JFloat kJFloatMin						= DBL_MIN;
const JFloat kJFloatMax						= DBL_MAX;

const long kJFloatExponentMin				= DBL_MIN_10_EXP;
const long kJFloatExponentMax				= DBL_MAX_10_EXP;

	// specialized types

typedef unsigned long						JSize;

const JSize kJSizeMin						= 0;
const JSize kJSizeMax						= ULONG_MAX;

typedef unsigned long						JIndex;

const JIndex kJIndexMin						= 0;
const JIndex kJIndexMax						= ULONG_MAX;

typedef long								JSignedIndex;

const JSignedIndex kJSignedIndexMin			= LONG_MIN;
const JSignedIndex kJSignedIndexMax			= LONG_MAX;

typedef unsigned long						JUnsignedOffset;

const JUnsignedOffset kJUnsignedOffsetMin	= 0;
const JUnsignedOffset kJUnsignedOffsetMax	= ULONG_MAX;

typedef long								JSignedOffset;

const JSignedOffset kJSignedOffsetMin		= LONG_MIN;
const JSignedOffset kJSignedOffsetMax		= LONG_MAX;

typedef long								JCoordinate;

const JCoordinate kJCoordinateMin			= LONG_MIN;
const JCoordinate kJCoordinateMax			= LONG_MAX;

typedef unsigned long						JFileVersion;

const JFileVersion kJFileVersionMin			= 0;
const JFileVersion kJFileVersionMax			= ULONG_MAX;

	// used mainly by JHistogram.tmpl

typedef JIndex								JBinIndex;

const JBinIndex kJBinIndexMin				= kJIndexMin;
const JBinIndex kJBinIndexMax				= kJIndexMax;

	// boolean type

#if 0
enum JBoolean
{
	kJFalse = 0,
	kJTrue  = 1
};

const JSize kJBooleanDataStreamLength = sizeof(JCharacter);

istream& operator>>(istream& input, JBoolean& jbool);
ostream& operator<<(ostream& output, const JBoolean jbool);
#else
typedef bool JBoolean;
const bool kJFalse = false;
const bool kJTrue = true;
const bool kFalse = false;
const bool kTrue = true;
#endif

inline JBoolean
JConvertToBoolean
	(
	const int comparison
	)
{
	return (comparison ? kJTrue : kJFalse);
}

inline JBoolean
JI2B
	(
	const int comparison
	)
{
	return (comparison ? kJTrue : kJFalse);
}

inline JBoolean
JNegate
	(
	const int comparison
	)
{
	return (comparison ? kJFalse : kJTrue);
}

// This is provided because 0.1 should be considered "true" since
// it is non-zero.  JConvertToBoolean() would truncate it to zero.
// Of course, you shouldn't be using floating-point values to store
// flags, but somebody else might force you to.

inline JBoolean
JFloatToBoolean
	(
	const double x
	)
{
	return (x == 0.0 ? kJFalse : kJTrue);
}

inline JBoolean
JF2B
	(
	const double x
	)
{
	return (x == 0.0 ? kJFalse : kJTrue);
}

/******************************************************************************

 J?Int... types

	These types provide fixed and machine-specific sized signed and
	unsigned integral types for algorithms which require explicit knowledge
	of the size of their integers, or simply a guarantee that they are
	exactly one machine word.  The JInt... types are signed, while the
	JUInt... quantities are unsigned.  The suffix indicates the actual
	width of the type in bits.

	Specifically, JUInt32 provides an unsigned integer guaranteed to be 32
	bits on all supported machines, a bit like the FORTRAN (gasp!) *N style
	idiom; it is similar to an unsigned version of a FORTRAN INTEGER*4.
	Similarly, JUInt64 provides a 64-bit integer, but *only on machines
	which provide such a native type*.  Thus it does not always exist and
	must be tested for with the preprocessor.  Normally one only needs to
	do this to ensure that a variable is a machine word, which is provided
	automatically through the JUWord type and always exists.  JInt32,
	JInt64, and JWord are their signed bretheren.

	Considerations:

	All unix machines seem to define an int as 32 bits, regardless of
	wordsize, while a long is the machine's native wordsize, 32 bits on
	32-bit architectures and 64 on 64-bit processors like the Alpha.  This
	is consistent with the traditional (i.e. before 64-bit processor) C
	programmer's assumption that pointers and longs can convert back and
	forth without loss of information.

	MetroWerks uses 16-bit ints (perhaps the original Mac had 16-bit words?),
	so on the Mac (at least) we'll need to use a long.

	For strict ANSI compliance it can be a good idea to use a signed integer
	when the quantity may be interpreted as either a signed or unsigned value.
	The reason for this is that if a signed integral quantity is converted to
	an unsigned quantity ANSI guarantees that it behaves exactly as you think
	it should, i.e. as though the signed quantity was stored in two's complement
	representation and the bit pattern was simply re-interpreted as an unsigned
	integer.

	By contrast, ANSI does *not* guarantee that the reverse is true.  An unsigned
	quantity converted to a signed quantity overflows when its value is too large
	to fit into the signed quantity, *and the result is undefined*.  The fact
	that every C programmer on the planet behaves as though ANSI mandates the
	usual unix/two's complement behavior of re-interpreting the bit pattern as a
	two's complement value apparently did not sway the standards committee, and
	code which depends on such behavior is not strictly portable.  On the other
	hand, I wonder how many C compliers would dare to break this behavior.

 ******************************************************************************/

#if SIZEOF_LONG == 4

	typedef unsigned long JUInt32;
	typedef   signed long JInt32;

	const JUInt32 kJUInt32Min = 0;
	const JUInt32 kJUInt32Max = ULONG_MAX;

	const JInt32 kJInt32Min = LONG_MIN;
	const JInt32 kJInt32Max = LONG_MAX;

#elif SIZEOF_INT == 4

	typedef unsigned int  JUInt32;
	typedef   signed int  JInt32;

	const JUInt32 kJUInt32Min = 0;
	const JUInt32 kJUInt32Max = UINT_MAX;

	const JInt32 kJInt32Min = INT_MIN;
	const JInt32 kJInt32Max = INT_MAX;

#else
	*** Trouble; we must always have 32-bit types.  Generalize the test to
	*** your architecture before compiling, and contact the JX team so the
	*** next version does it correctly!
#endif

#if SIZEOF_LONG == 8

	// Need a way to detect that the type exists
	#define JUInt64_EXISTS
	#define  JInt64_EXISTS

	typedef unsigned long JUInt64;
	typedef   signed long JInt64;

	const JUInt64 kJUInt64Min = 0;
	const JUInt64 kJUInt64Max = ULONG_MAX;

	const JInt64 kJInt64Min = LONG_MIN;
	const JInt64 kJInt64Max = LONG_MAX;

#elif SIZEOF_INT == 8

	// I know, but gotta look ahead to 128-bit processors

	// Need a way to detect that the type exists
	#define JUInt64_EXISTS
	#define  JInt64_EXISTS

	typedef unsigned int JUInt64;
	typedef   signed int JInt64;

	const JUInt64 kJUInt64Min = 0;
	const JUInt64 kJUInt64Max = UINT_MAX;

	const JInt64 kJInt64Min = INT_MIN;
	const JInt64 kJInt64Max = INT_MAX;

#endif

	// So far, all machines have word-sized longs
	#define JWORDSIZE SIZEOF_LONG
	typedef unsigned long JUWord;
	typedef   signed long JWord;

/******************************************************************************

	Defines standard hash value types, e.g. for use in JHashTable.

 ******************************************************************************/

// JHashValue must be unsigned to guarantee that >> shifts in zero bits

typedef JUWord JHashValue;

// JDualHashValue must be signed so the step can go either way

typedef  JWord JDualHashValue;

/******************************************************************************

	Hides compiler dependent iostream information.

 ******************************************************************************/

#if __GNUG__ >= 3

	using std::ios_base;
	using std::streampos;
	using std::streamoff;

	typedef ios_base::openmode JFStreamOpenMode;
	typedef ios_base::seekdir  JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios_base::binary;

#elif defined __GNUG__

	#error "g++ 3.x or later is required to avoid thunks bugs"

	#include <string>

	namespace std
	{
		typedef ::string	string;
	};

	typedef int           JFStreamOpenMode;
	typedef ios::seek_dir JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios::bin;

#elif defined _MSC_VER

	using std::ios_base;
	using std::streampos;
	using std::streamoff;

	typedef ios_base::openmode JFStreamOpenMode;
	typedef ios_base::seekdir  JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios_base::binary;

#elif defined __KCC

	typedef ios::openmode JFStreamOpenMode;
	typedef ios::seekdir  JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios::binary;

#elif defined __SUNPRO_CC

	typedef int           JFStreamOpenMode;
	typedef ios::seek_dir JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios::binary;

#else


	#include <string>

	namespace std
	{
		typedef ::string	string;
	};

	typedef int           JFStreamOpenMode;
	typedef ios::seek_dir JIOStreamSeekDir;

	const JFStreamOpenMode kJBinaryModifier = ios::bin;

#endif

const JFStreamOpenMode kJBinaryFile = ios::in | ios::out | kJBinaryModifier;
const JFStreamOpenMode kJTextFile   = ios::in | ios::out;

#endif
