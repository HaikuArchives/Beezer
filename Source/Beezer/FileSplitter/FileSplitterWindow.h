/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _FILE_SPLITTER_WINDOW_H
#define _FILE_SPLITTER_WINDOW_H

#include <Window.h>
#include <String.h>

class BStringView;            // Zeta needs such pre-declares to compiler
class BCheckBox;
class BTextControl;
class BStatusBar;
class BString;

class BevelView;
class SelectDirPanel;
class RecentMgr;

#define M_SPLIT_NOW                        'splt'
#define M_SELECT_SPLIT_FILE                'file'
#define M_SELECT_SPLIT_FOLDER              'fold'
#define M_SPLIT_FILE_SELECTED              'fise'
#define M_SPLIT_FOLDER_SELECTED            'flse'
#define M_CUSTOM_SIZE                      'cuss'
#define M_PREDEFINED_SIZE                  'pres'
#define M_UPDATE_DATA                      'updd'
#define M_OPERATION_COMPLETE               'opcc'
#define M_SEPARATOR_CHANGED                'spch'

#define K_WINDOW                           "split_wnd"

class FileSplitterWindow : public BWindow
{
    public:
        FileSplitterWindow (RecentMgr *files, RecentMgr *dirs);
        virtual ~FileSplitterWindow ();
        
        // Inherited hooks
        virtual void         MessageReceived (BMessage *message);
        virtual void         Quit ();
        virtual bool         QuitRequested ();
        
    private:
        // Private hooks
        void                 ToggleWindowHeight (bool expand);
        void                 UpdateData ();
        void                 UpdateRecentMenus ();
        void                 CreateSelfJoiner ();
        
        // Thread functions
        static int32         _splitter (void *data);

        // Private members
        BevelView           *m_backView,
                            *m_innerView;
        BMenu               *m_fileMenu,
                            *m_folderMenu;
        BMenuField          *m_fileField,
                            *m_folderField,
                            *m_sizeField,
                            *m_prefixField,
                            *m_separatorField;
        BPopUpMenu          *m_sizePopUp,
                            *m_prefixPopUp,
                            *m_separatorPopUp;
        BTextControl        *m_filePathView,
                            *m_folderPathView,
                            *m_customSizeView,
                            *m_separatorView;
        BButton             *m_selectFileBtn,
                            *m_selectFolderBtn,
                            *m_splitBtn;
        BStringView         *m_descStr,
                            *m_descStr2,
                            *m_piecesStr,
                            *m_sizeStr;
        BCheckBox           *m_createChk,
                            *m_openDirChk,
                            *m_closeChk;
        BStatusBar          *m_statusBar;
        
        SelectDirPanel      *m_dirPanel;
        BFilePanel          *m_filePanel;
        
        float                m_hideProgress,
                             m_showProgress;

        BEntry               m_fileEntry;
        BDirectory           m_destDir;
        BString              m_firstChunkName;
        
        uint64               m_fragmentSize;
        uint16               m_fragmentCount;
        char                *m_sepString;
        volatile bool        m_cancel;
        bool                 m_splitInProgress,
                             m_quitNow;
        BMessenger          *m_messenger;
        thread_id            m_thread;
        
        RecentMgr           *m_recentSplitFiles,
                            *m_recentSplitDirs;
};

#endif /* _FILE_SPLITTER_WINDOW_H */
