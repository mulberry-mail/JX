/******************************************************************************
 JXFileInput.cpp

	Input field for entering a path + file.

	BASE CLASS = JXInputField

	Copyright � 1999 by John Lindal. All rights reserved.

 ******************************************************************************/

#include <JXStdInc.h>
#include <JXFileInput.h>
#include <JXSelectionManager.h>
#include <JXDNDManager.h>
#include <JXPathInput.h>
#include <JXStringCompletionMenu.h>
#include <JXColormap.h>
#include <jXGlobals.h>
#include <jXUtil.h>
#include <JDirInfo.h>
#include <jFileUtil.h>
#include <jDirUtil.h>
#include <jAssert.h>

static const JCharacter* kFontName = "6x13";
const JSize kFontSize              = 12;

// string ID's

static const JCharacter* kHintID         = "Hint::JXFileInput";
static const JCharacter* kNoRelPathID    = "NoRelPath::JXFileInput";
static const JCharacter* kDoesNotExistID = "DoesNotExist::JXFileInput";
static const JCharacter* kUnreadableID   = "Unreadable::JXFileInput";
static const JCharacter* kUnwritableID   = "Unwritable::JXFileInput";
static const JCharacter* kCannotExecID   = "CannotExec::JXFileInput";

/******************************************************************************
 Constructor

 ******************************************************************************/

JXFileInput::JXFileInput
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXInputField(enclosure, hSizing, vSizing, x,y, w,h),
	itsCompleter(NULL),
	itsCompletionMenu(NULL)
{
	itsAllowInvalidFileFlag = kJFalse;
	itsRequireReadFlag      = kJTrue;
	itsRequireWriteFlag     = kJTrue;
	itsRequireExecFlag      = kJFalse;
	itsExpectURLDropFlag    = kJFalse;
	SetIsRequired();
	SetCharacterInWordFunction(SIsCharacterInWord);
	SetDefaultFontName(kFontName);
	ShouldBroadcastCaretLocationChanged(kJTrue);
	SetHint(JGetString(kHintID));
	ListenTo(this);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXFileInput::~JXFileInput()
{
	delete itsCompleter;
}

/******************************************************************************
 GetFont (static)

 ******************************************************************************/

const JCharacter*
JXFileInput::GetFont
	(
	JSize* size
	)
{
	*size = kFontSize;
	return kFontName;
}

/******************************************************************************
 HandleUnfocusEvent (virtual protected)

 ******************************************************************************/

void
JXFileInput::HandleUnfocusEvent()
{
	JXInputField::HandleUnfocusEvent();
	SetCaretLocation(GetTextLength() + 1);
}

/******************************************************************************
 ApertureResized (virtual protected)

 ******************************************************************************/

void
JXFileInput::ApertureResized
	(
	const JCoordinate dw,
	const JCoordinate dh
	)
{
	JXInputField::ApertureResized(dw,dh);
	if (!HasFocus())
		{
		SetCaretLocation(GetTextLength() + 1);
		}
}

/******************************************************************************
 BoundsMoved (virtual protected)

	Show the full text in the hint if part of it is not visible in the widget.

 ******************************************************************************/

void
JXFileInput::BoundsMoved
	(
	const JCoordinate dx,
	const JCoordinate dy
	)
{
	JXInputField::BoundsMoved(dx, dy);

	const JRect ap = GetAperture();
	if (ap.left != 0)
		{
		SetHint(GetText());
		}
	else
		{
		ClearHint();
		}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
JXFileInput::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == this && message.Is(JTextEditor::kTextSet))
		{
		SetCaretLocation(GetTextLength() + 1);
		}
	else if (sender == this && message.Is(JTextEditor::kCaretLocationChanged))
		{
		JIndex i;
		WantInput(kJTrue,
				  JI2B(GetCaretLocation(&i) && i == GetTextLength()+1),
				  WantsModifiedTab());
		}

	JXInputField::Receive(sender, message);
}

/******************************************************************************
 SetBasePath

	This is used if a relative path is entered.  It must be an absolute path.

 ******************************************************************************/

void
JXFileInput::SetBasePath
	(
	const JCharacter* path
	)
{
	if (JStringEmpty(path))
		{
		ClearBasePath();
		}
	else
		{
		assert( !JIsRelativePath(path) );
		itsBasePath = path;
		RecalcAll(kJTrue);
		}
}

/******************************************************************************
 GetFile (virtual)

	Returns kJTrue if the current file name is valid.  In this case, *fullName
	is set to the full path + name, relative to the root directory.

	Use this instead of GetText(), because that may return a relative path.

 ******************************************************************************/

JBoolean
JXFileInput::GetFile
	(
	JString* fullName
	)
	const
{
	const JString& text = GetText();
	return JI2B(!text.IsEmpty() &&
				(!JIsRelativePath(text) || HasBasePath()) &&
				JConvertToAbsolutePath(text, itsBasePath, fullName) &&
				JFileExists(*fullName) &&
				(!itsRequireReadFlag  || JFileReadable(*fullName)) &&
				(!itsRequireWriteFlag || JFileWritable(*fullName)) &&
				(!itsRequireExecFlag  || JFileExecutable(*fullName)));
}

/******************************************************************************
 InputValid (virtual)

 ******************************************************************************/

JBoolean
JXFileInput::InputValid()
{
	if (itsAllowInvalidFileFlag)
		{
		return kJTrue;
		}
	else if (!JXInputField::InputValid())
		{
		return kJFalse;
		}

	const JString& text = GetText();
	if (text.IsEmpty())
		{
		return JNegate(IsRequired());
		}

	JString fullName;
	const JCharacter* errID = NULL;
	if (JIsRelativePath(text) && !HasBasePath())
		{
		errID = kNoRelPathID;
		RecalcAll(kJTrue);
		}
	else if (!JConvertToAbsolutePath(text, itsBasePath, &fullName) ||
			 !JFileExists(fullName))
		{
		errID = kDoesNotExistID;
		}
	else if (itsRequireReadFlag && !JFileReadable(fullName))
		{
		errID = kUnreadableID;
		}
	else if (itsRequireWriteFlag && !JFileWritable(fullName))
		{
		errID = kUnwritableID;
		}
	else if (itsRequireExecFlag && !JFileExecutable(fullName))
		{
		errID = kCannotExecID;
		}

	if (JStringEmpty(errID))
		{
		return kJTrue;
		}
	else
		{
		(JGetUserNotification())->ReportError(JGetString(errID));
		return kJFalse;
		}
}

/******************************************************************************
 AdjustStylesBeforeRecalc (virtual protected)

	Draw the invalid portion of the path in red.

 ******************************************************************************/

#ifdef _J_UNIX
static const JCharacter* kThisDirSuffix = "/.";
#elif defined WIN32
static const JCharacter* kThisDirSuffix = "\\.";
#endif

void
JXFileInput::AdjustStylesBeforeRecalc
	(
	const JString&		buffer,
	JRunArray<Font>*	styles,
	JIndexRange*		recalcRange,
	JIndexRange*		redrawRange,
	const JBoolean		deletion
	)
{
	const JColormap* colormap = GetColormap();
	const JSize totalLength   = buffer.GetLength();

	JString fullName = buffer;
	if ((JIsRelativePath(buffer) && !HasBasePath()) ||
		!JExpandHomeDirShortcut(buffer, &fullName))
		{
		fullName.Clear();
		}

	// Last clause because if JConvertToAbsolutePath() succeeds, we don't
	// want to further modify fullName.

	else if (JIsRelativePath(buffer) &&
			 !JConvertToAbsolutePath(buffer, itsBasePath, &fullName))
		{
		if (HasBasePath())
			{
			fullName = JCombinePathAndName(itsBasePath, buffer);
			}
		else
			{
			fullName.Clear();
			}
		}

	JSize errLength;
	if (fullName.IsEmpty())
		{
		errLength = totalLength;
		}
	else if (!JFileExists(fullName) ||
			 (itsRequireReadFlag  && !JFileReadable(fullName)) ||
			 (itsRequireWriteFlag && !JFileWritable(fullName)) ||
			 (itsRequireExecFlag  && !JFileExecutable(fullName)))
		{
		const JString closestDir = JGetClosestDirectory(fullName, kJFalse);
		if (fullName.BeginsWith(closestDir))
			{
			errLength = fullName.GetLength() - closestDir.GetLength();
			}
		else
			{
			errLength = totalLength;
			}
		}
	else
		{
		errLength = 0;
		}

	if (errLength > 0 && buffer.EndsWith(kThisDirSuffix))
		{
		errLength++;	// trailing . is trimmed
		}

	Font f = styles->GetFirstElement();

	styles->RemoveAll();
	if (errLength >= totalLength)
		{
		f.style.color = colormap->GetRedColor();
		styles->AppendElements(f, totalLength);
		}
	else
		{
		f.style.color = colormap->GetBlackColor();
		styles->AppendElements(f, totalLength - errLength);

		if (errLength > 0)
			{
			f.style.color = colormap->GetRedColor();
			styles->AppendElements(f, errLength);
			}
		}

	*redrawRange += JIndexRange(1, totalLength);
}

/******************************************************************************
 GetTextColor (static)

	Draw the entire text red if the file is invalid.  This is provided
	so tables can draw the text the same way as the input field.

	base can be NULL.  If you use JXFileInput for relative paths, basePath
	should be the path passed to SetBasePath().

 ******************************************************************************/

JColorIndex
JXFileInput::GetTextColor
	(
	const JCharacter*	fileName,
	const JCharacter*	basePath,
	const JBoolean		requireRead,
	const JBoolean		requireWrite,
	const JBoolean		requireExec,
	const JColormap*	colormap
	)
{
	if (JStringEmpty(fileName))
		{
		return colormap->GetBlackColor();
		}

	JString fullName;
	if ((!JIsRelativePath(fileName) || !JStringEmpty(basePath)) &&
		JConvertToAbsolutePath(fileName, basePath, &fullName) &&
		(!requireRead  || JFileReadable(fullName)) &&
		(!requireWrite || JFileWritable(fullName)) &&
		(!requireExec  || JFileExecutable(fullName)))
		{
		return colormap->GetBlackColor();
		}
	else
		{
		return colormap->GetRedColor();
		}
}

/******************************************************************************
 WillAcceptDrop (virtual protected)

	Accept text/uri-list.

 ******************************************************************************/

JBoolean
JXFileInput::WillAcceptDrop
	(
	const JArray<Atom>&	typeList,
	Atom*				action,
	const Time			time,
	const JXWidget*		source
	)
{
	itsExpectURLDropFlag = kJFalse;

	const Atom urlXAtom = (GetSelectionManager())->GetURLXAtom();

	JString fileName;
	const JSize typeCount = typeList.GetElementCount();
	for (JIndex i=1; i<=typeCount; i++)
		{
		if (typeList.GetElement(i) == urlXAtom &&
			GetDroppedFileName(time, kJFalse, &fileName))
			{
			*action = (GetDNDManager())->GetDNDActionPrivateXAtom();
			itsExpectURLDropFlag = kJTrue;
			return kJTrue;
			}
		}

	return JXInputField::WillAcceptDrop(typeList, action, time, source);
}

/******************************************************************************
 HandleDNDEnter (virtual protected)

	This is called when the mouse enters the widget.

 ******************************************************************************/

void
JXFileInput::HandleDNDEnter()
{
	if (!itsExpectURLDropFlag)
		{
		JXInputField::HandleDNDEnter();
		}
}

/******************************************************************************
 HandleDNDHere (virtual protected)

	This is called while the mouse is inside the widget.

 ******************************************************************************/

void
JXFileInput::HandleDNDHere
	(
	const JPoint&	pt,
	const JXWidget*	source
	)
{
	if (!itsExpectURLDropFlag)
		{
		JXInputField::HandleDNDHere(pt, source);
		}
}

/******************************************************************************
 HandleDNDLeave (virtual protected)

	This is called when the mouse leaves the widget without dropping data.

 ******************************************************************************/

void
JXFileInput::HandleDNDLeave()
{
	if (!itsExpectURLDropFlag)
		{
		JXInputField::HandleDNDLeave();
		}
}

/******************************************************************************
 HandleDNDDrop (virtual protected)

	This is called when the data is dropped.  The data is accessed via
	the selection manager, just like Paste.

	Since we only accept text/uri-list, we don't bother to check typeList.

 ******************************************************************************/

void
JXFileInput::HandleDNDDrop
	(
	const JPoint&		pt,
	const JArray<Atom>&	typeList,
	const Atom			action,
	const Time			time,
	const JXWidget*		source
	)
{
	JString fileName;
	if (!itsExpectURLDropFlag)
		{
		JXInputField::HandleDNDDrop(pt, typeList, action, time, source);
		}
	else if (Focus() && GetDroppedFileName(time, kJTrue, &fileName))
		{
		SetText(fileName);
		}
}

/******************************************************************************
 GetDroppedFileName (private)

 ******************************************************************************/

JBoolean
JXFileInput::GetDroppedFileName
	(
	const Time		time,
	const JBoolean	reportErrors,
	JString*		fileName
	)
{
	fileName->Clear();

	JXSelectionManager* selMgr = GetSelectionManager();

	Atom returnType;
	unsigned char* data;
	JSize dataLength;
	JXSelectionManager::DeleteMethod delMethod;
	if (selMgr->GetData((GetDNDManager())->GetDNDSelectionName(),
						time, selMgr->GetURLXAtom(),
						&returnType, &data, &dataLength, &delMethod))
		{
		if (returnType == selMgr->GetURLXAtom())
			{
			JPtrArray<JString> fileNameList(JPtrArrayT::kDeleteAll),
							   urlList(JPtrArrayT::kDeleteAll);
			JXUnpackFileNames((char*) data, dataLength, &fileNameList, &urlList);
			if (fileNameList.GetElementCount() == 1 &&
				(JFileExists(*(fileNameList.FirstElement())) ||
				 JDirectoryExists(*(fileNameList.FirstElement()))))
				{
				*fileName = *(fileNameList.FirstElement());
				}
			JXReportUnreachableHosts(urlList);
			}

		selMgr->DeleteData(&data, delMethod);
		}

	return JNegate( fileName->IsEmpty() );
}

/******************************************************************************
 GetTextForChooseFile (virtual)

	Returns a string that can safely be passed to JChooseSaveFile::ChooseFile().

 ******************************************************************************/

JString
JXFileInput::GetTextForChooseFile()
	const
{
	JString text = GetText();
	if (text.IsEmpty() && HasBasePath())
		{
		text = itsBasePath;
		JAppendDirSeparator(&text);
		}
	if (text.EndsWith(ACE_DIRECTORY_SEPARATOR_STR))
		{
		text.AppendCharacter('*');
		}
	if (!text.IsEmpty() && JIsRelativePath(text) && HasBasePath())
		{
		text = JCombinePathAndName(itsBasePath, text);
		}
	return text;
}

/******************************************************************************
 ChooseFile

	instr can be NULL, just like in JChooseSaveFile::ChooseFile().

 ******************************************************************************/

JBoolean
JXFileInput::ChooseFile
	(
	const JCharacter* prompt,
	const JCharacter* instr
	)
{
	const JString origName = GetTextForChooseFile();
	JString newName;
	if ((JGetChooseSaveFile())->ChooseFile(prompt, instr, origName, &newName))
		{
		SetText(newName);
		return kJTrue;
		}
	else
		{
		return kJFalse;
		}
}

/******************************************************************************
 SaveFile

	instr can be NULL, just like in JChooseSaveFile::SaveFile().

 ******************************************************************************/

JBoolean
JXFileInput::SaveFile
	(
	const JCharacter* prompt,
	const JCharacter* instr
	)
{
	const JString origName = GetTextForChooseFile();
	JString newName;
	if ((JGetChooseSaveFile())->SaveFile(prompt, instr, origName, &newName))
		{
		SetText(newName);
		return kJTrue;
		}
	else
		{
		return kJFalse;
		}
}

/******************************************************************************
 SIsCharacterInWord (static)

 ******************************************************************************/

JBoolean
JXFileInput::SIsCharacterInWord
	(
	const JString&	text,
	const JIndex	charIndex
	)
{
	return JI2B( text.GetCharacter(charIndex) != ACE_DIRECTORY_SEPARATOR_CHAR );
}

/******************************************************************************
 HandleKeyPress (virtual)

 ******************************************************************************/

void
JXFileInput::HandleKeyPress
	(
	const int				key,
	const JXKeyModifiers&	modifiers
	)
{
	if (key == '\t')
		{
		if (itsCompleter == NULL)
			{
			if (!JDirInfo::Create(JGetRootDirectory(), &itsCompleter))
				{
				return;
				}
			itsCompleter->ShowHidden(kJTrue);
			itsCompleter->ShowDirs(kJTrue);
			itsCompleter->ShowFiles(kJTrue);
			itsCompleter->ShouldApplyWildcardFilterToDirs();
			}

		JXPathInput::Complete(this, itsBasePath, itsCompleter, &itsCompletionMenu);
		}
	else
		{
		if (itsCompletionMenu != NULL)
			{
			itsCompletionMenu->ClearRequestCount();
			}
		JXInputField::HandleKeyPress(key, modifiers);
		}
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
JXFileInput::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsCompletionMenu != NULL)
		{
		itsCompletionMenu->ClearRequestCount();
		}
	JXInputField::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}
