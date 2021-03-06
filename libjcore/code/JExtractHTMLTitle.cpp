/******************************************************************************
 JExtractHTMLTitle.cpp

	Extracts the title (correctly interpreting escape sequences) from
	the given HTML data.

	BASE CLASS = JHTMLScanner

	Copyright � 2000 by John Lindal. All rights reserved.

 ******************************************************************************/

#include <JCoreStdInc.h>
#include <JExtractHTMLTitle.h>
#include <JString.h>
#include <jAssert.h>

/******************************************************************************
 Constructor

	This is protected because JExtractHTMLTitle() is our only use.

 ******************************************************************************/

JExtractHTMLTitle::JExtractHTMLTitle()
	:
	JHTMLScanner()
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JExtractHTMLTitle::~JExtractHTMLTitle()
{
}

/******************************************************************************
 ExtractTitle

	Returns kJFalse if there is no title.

 ******************************************************************************/

JBoolean
JExtractHTMLTitle::ExtractTitle
	(
	const JCharacter*	text,
	JString*			title
	)
{
	title->Clear();

	itsInTitleFlag = kJFalse;
	itsNeedWSFlag  = kJFalse;
	itsTitle       = title;
	LexHTML(text);
	itsTitle       = NULL;

	return JNegate(title->IsEmpty());
}

/******************************************************************************
 Handle something (virtual protected)

 ******************************************************************************/

JBoolean
JExtractHTMLTitle::HandleHTMLTag
	(
	const JString&					name,
	const JStringPtrMap<JString>&	attr,
	const JIndexRange&				range
	)
{
	if (name == "title")
		{
		itsInTitleFlag = kJTrue;
		return kJTrue;
		}
	else if (name == "/title" || name == "/head")
		{
		return kJFalse;	// stop
		}
	else
		{
		return kJTrue;
		}
}

JBoolean
JExtractHTMLTitle::HandleHTMLWord
	(
	const JCharacter*	word,
	const JIndexRange&	range
	)
{
	assert( itsTitle != NULL );

	if (itsInTitleFlag)
		{
		if (itsNeedWSFlag && !itsTitle->IsEmpty())
			{
			itsTitle->AppendCharacter(' ');
			}
		itsTitle->Append(word);
		}
	itsNeedWSFlag = kJFalse;
	return kJTrue;
}

JBoolean
JExtractHTMLTitle::HandleHTMLWhitespace
	(
	const JCharacter*	space,
	const JIndexRange&	range
	)
{
	itsNeedWSFlag = kJTrue;
	return kJTrue;
}

/******************************************************************************
 SwitchCharSet (virtual protected)

	We want the alternate version.

 ******************************************************************************/

JBoolean
JExtractHTMLTitle::SwitchCharSet
	(
	const JCharacter* charSet
	)
{
	return kJFalse;
}
