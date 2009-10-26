/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _SELECT_DIR_PANEL_H
#define _SELECT_DIR_PANEL_H

#include <FilePanel.h>

class SelectDirPanel : public BFilePanel
{
	public:
		SelectDirPanel (file_panel_mode mode = B_OPEN_PANEL, BMessenger *target = 0,
					const entry_ref *start_directory = 0, uint32 node_flavors = 0,
					bool allow_multiple_selection = true, BMessage *message = 0, BRefFilter * = 0,
					bool modal = false, bool hide_when_done = true);
		~SelectDirPanel ();
		
		// Inherited hooks
		virtual void		SelectionChanged();
		void				Refresh();
		void				SetMessage (BMessage *msg);

		// Additional hooks
		void				SetCurrentDirButton (const char *label);
		void				UpdateButton ();

	protected:
		// Protected hooks
		void				SendMessage (const BMessenger *target, BMessage *message);
		
	private:
		// Private members
		BString				m_buttonLabel,
							m_buttonName;
		BButton				*m_curDirBtn;
};

#endif /* _CURRENT_DIR_FILTER_H */
