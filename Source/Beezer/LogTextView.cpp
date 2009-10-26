/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <String.h>
#include <PopUpMenu.h>
#include <Window.h>
#include <Message.h>
#include <MenuItem.h>
#include <Clipboard.h>

#include <ctype.h>

#include "LogTextView.h"
#include "MsgConstants.h"

//=============================================================================================================//

LogTextView::LogTextView (BRect frame, const char *name, uint32 resizeMask, uint32 flags = B_WILL_DRAW)
	: BTextView (frame, name, kInternalTextRect, resizeMask, flags)
{
	InitSelf();
}

//=============================================================================================================//

LogTextView::LogTextView (BRect frame, const char *name, const BFont *initialFont,
				const rgb_color *initialColor, uint32 resizeMask, uint32 flags)
	: BTextView (frame, name, kInternalTextRect, initialFont, initialColor, resizeMask, flags)
{
	InitSelf();
}

//=============================================================================================================//

void LogTextView::InitSelf ()
{
	// Initialise members
	m_contextMenu = NULL;
	
	// Initialise methods
	MakeEditable (false);
	SetWordWrap (false);
}

//=============================================================================================================//

void LogTextView::SetContextMenu (BPopUpMenu *contextMenu)
{
	m_contextMenu = contextMenu;
}

//=============================================================================================================//

void LogTextView::AddText (const char *text, bool newLine, bool capitalizeFirstLetter, bool trimLeadingSpaces)
{
	int32 textLen = TextLength() - 1;

	if (newLine == true)
	{
		Insert (textLen, "\n", 1);
		textLen++;
	}
	
	// Count trailing newline character if any
	int32 byteLenOfText = strlen (text);
	if (text[byteLenOfText - 1] == '\n')
		byteLenOfText--;
	
	// Skip all leading spaces
	if (trimLeadingSpaces == true)
		while (*text == ' ')
			text++;
	
	BString outText = text;
	outText.RemoveAll ("\n");

	#ifdef B_ZETA_VERSION
		if (capitalizeFirstLetter == true)
			outText.ReplaceFirst (outText[0], toupper(outText[0]));
	#else
		if (capitalizeFirstLetter == true)
			outText[0] = toupper(outText[0]);
	#endif
	
	Insert (textLen, outText.String(), byteLenOfText);
	ScrollToSelection();
}
	
//=============================================================================================================//

void LogTextView::MouseDown (BPoint point)
{
	BMessage *msg = Window()->CurrentMessage();
	int32 button = msg->FindInt32 ("buttons");

	// Detect right click for context menu popup
	if (button == B_SECONDARY_MOUSE_BUTTON && m_contextMenu != NULL && TextLength() > 0)
	{
		BPoint screenPt = point;
		BRect openRect (point.x - 2, point.y - 2, point.x + 2, point.y + 2);
		
		ConvertToScreen (&screenPt);
		ConvertToScreen (&openRect);

		m_contextMenu->SetAsyncAutoDestruct (true);
		BMenuItem *selectedItem = m_contextMenu->Go (screenPt, false, true, openRect, false);
		if (selectedItem && Window())
			Window()->PostMessage (selectedItem->Message());
	}
	else			// Bug Fix -- else added, otherwise selection will take place after RIGHT-click
		BTextView::MouseDown (point);
}

//=============================================================================================================//

void LogTextView::Copy ()
{
	// If no selection, copy all the text, else copy only the selected text.
	int32 selStart, selEnd;
	GetSelection (&selStart, &selEnd);
	if (selStart == selEnd)
	{
		const char *text = Text();
		uint32 len = strlen (text);
		
		if (len > 0L)
		{
			// Now copy the buffer to the clipboard
			BMessage *clip = NULL;
			if (be_clipboard->Lock())
			{
				be_clipboard->Clear();
				if ((clip = be_clipboard->Data()))
				{
					clip->AddData ("text/plain", B_MIME_TYPE, text, len);
					be_clipboard->Commit();
				}
				be_clipboard->Unlock();
			}
		}
	}
	else
	{
		BTextView::Copy (be_clipboard);

		// Now remove the selection (if there was any)
		if (selStart != selEnd)
			Select (selEnd, selEnd);
	}	
}

//=============================================================================================================//

void LogTextView::MakeFocus (bool focused)
{
	Window()->PostMessage (M_GOT_FOCUS);
	return BTextView::MakeFocus (focused);
}

//=============================================================================================================//
