/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _JOINER_STRINGS_H
#define _JOINER_STRINGS_H

#include <InterfaceDefs.h>

#ifdef B_ZETA_VERSION
	#include <Locale.h>
	extern const char* _X (const char *key, int keyindex);
	#define str(X) _X (# X, X)
#else
	#define str(X) strR5(X)
#endif

extern const char *strR5 (int whichString, int whichLanguage = -1);
extern void SetLanguage (int whichLanguage);
extern const char *GetLanguageName (int whichLanguage, bool inNativeTongue);
extern int GetLanguage ();

enum LanguageNames
{
	LANG_ENGLISH = 0,
	NUM_LANGUAGES
};


// NOTE:: 
// If you add any new language strings please add which version of the code has it been added
// in, notice that 0.06 upwards these labels are present as 0.04, 0.05 had same labels and Zeta 0.04
// language file is as per that order

enum LanguageStrings
{
	S_ENGLISH = 0,
	S_JOINER_WINDOW_TITLE,
	S_JOINING_FILE,
	S_CANCEL,
	S_OK,
	S_JOIN_ERROR,
	S_COMPUTING,
	S_STUB_ERROR,
	NUM_STRINGS
};

#endif /* _JOINER_STRINGS_H */
