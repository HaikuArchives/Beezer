/*
 *	Beezer
 *	Copyright (c) 2005 Ramshankar (aka Teknomancer)
 *	Copyright (c) 2005 Marcin Konicki (aka ahwayakchih)
 *	See "License.txt" for licensing info.
*/

#ifndef _7Z_ARCHIVER_H
#define _7Z_ARCHIVER_H

#include <stdio.h>

#include "Archiver.h"

#define BZR_ARK							"7zip"

class BMessenger;

class z7Archiver : public Archiver
{
	public:
		z7Archiver (const char *binPath);
		
		// Overridables
		void				BuildDefaultMenu ();
		
		// Abstract Implementations & overridables
		status_t			Open (entry_ref *ref, BMessage *fileList);
		status_t			ReadOpen (FILE *fp);

		status_t			ReadExtract (FILE *fp, BMessenger *progress, volatile bool *cancel);
		status_t			Extract (entry_ref *dir, BMessage *list, BMessenger *progress, volatile bool *cancel);
		
		status_t			Test (char *&outputStr, BMessenger *progress, volatile bool *cancel);
		status_t			ReadTest (FILE *fp, char *&outputStr, BMessenger *progress, volatile bool *cancel);
		
		status_t			ReadAdd (FILE *fp, BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
		status_t			Add (bool createMode, const char *relPath, BMessage *list, BMessage *addedPaths,
								BMessenger *progress, volatile bool *cancel);
	
		status_t			Create (BPath *archivePath, const char *relPath, BMessage *fileList,
								BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
	
		status_t			Delete (char *&outputStr, BMessage *list, BMessenger *progress, volatile bool *cancel);
		status_t			ReadDelete (FILE *fp, char *&outputStr,	BMessenger *progress, volatile bool *cancel);

		bool				SupportsFolderEntity () const;
		bool				CanReplaceFiles () const;
		bool				CanPartiallyOpen () const;
		bool				SupportsPassword () const;

	private:
		void				SetMimeType ();

		char				m_7zPath[B_PATH_NAME_LENGTH];
};

#endif /* _7Z_ARCHIVER_H */
