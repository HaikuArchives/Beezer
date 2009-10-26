/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <Debug.h>
#include <Autolock.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <List.h>
#include <Alert.h>

#include <image.h>

#include "ArchiverMgr.h"
#include "MsgConstants.h"
#include "LangStrings.h"

BLocker _ark_locker ("_ark_mgr_lock", true);

//=============================================================================================================//

Beezer *_bzr ()
{
    return (Beezer*)be_app;
}

//=============================================================================================================//

Archiver* ArchiverForMime (const char *mimeType)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker (_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;
    
    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory *archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();
    
    BEntry binDirEntry;
    BPath binDirPath;
    (_bzr()->m_binDir).GetEntry (&binDirEntry);
    binDirEntry.GetPath (&binDirPath);
    const char *binPath = binDirPath.Path();
    
    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry (&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath (&path);

        image_id addonID = load_add_on (path.Path());
        if (addonID > 0L)
        {
           // Archiver loaded successfully, now check if it supports the mimetype
           Archiver *(*load_archiver)(const char*);
           if (get_image_symbol (addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
           {
               Archiver *ark = (*load_archiver)(binPath);

               BList *mimeList = ark->MimeTypeList();
               int32 supportedMimeCount = mimeList->CountItems();

               for (int32 i = 0; i < supportedMimeCount; i++)
               {
                  const char *mimeString = reinterpret_cast<const char*>(mimeList->ItemAtFast(i));
                  if (strcmp (mimeString, mimeType) == 0)
                      return ark;
               }
           }

           unload_add_on (addonID);
        }
    }
    
    return NULL;
}

//=============================================================================================================//

BList ArchiversInstalled (BList *extensionStrings)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BList installedArkList;
    BAutolock autoLocker (_ark_locker);
    if (autoLocker.IsLocked() == false)
        return installedArkList;
    
    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory *archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();
    
    BEntry binDirEntry;
    BPath binDirPath;
    (_bzr()->m_binDir).GetEntry (&binDirEntry);
    binDirEntry.GetPath (&binDirPath);
    const char *binPath = binDirPath.Path();
    
    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry (&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath (&path);

        image_id addonID = load_add_on (path.Path());
        if (addonID > 0L)
        {
           // Archiver loaded successfully, now check if it supports the mimetype
           Archiver *(*load_archiver)(const char*);
           if (get_image_symbol (addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
           {
               Archiver *ark = (*load_archiver)(binPath);
               installedArkList.AddItem ((void*)strdup (ark->ArchiveType()));
               if (extensionStrings)
                  extensionStrings->AddItem ((void*)strdup (ark->ArchiveExtension()));
           }

           unload_add_on (addonID);
        }
    }
    
    return installedArkList;
}

//=============================================================================================================//

Archiver* ArchiverForType (const char *archiverType)
{
    // Finds an archiver given its name (archiverType and name is the same, eg: zip, tar etc)
    
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker (_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;
    
    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory *archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();
    
    BEntry binDirEntry;
    BPath binDirPath;
    (_bzr()->m_binDir).GetEntry (&binDirEntry);
    binDirEntry.GetPath (&binDirPath);
    const char *binPath = binDirPath.Path();
    
    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry (&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath (&path);

        image_id addonID = load_add_on (path.Path());
        if (addonID > 0L)
        {
           // Archiver loaded successfully, now check if it supports the mimetype
           Archiver *(*load_archiver)(const char*);
           if (get_image_symbol (addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
           {
               Archiver *ark = (*load_archiver)(binPath);
               if (strcmp (ark->ArchiveType(), archiverType) == 0)
                  return ark;
           }

           unload_add_on (addonID);
        }
    }
    return NULL;
}

//=============================================================================================================//

BPopUpMenu* BuildArchiveTypesMenu (BHandler *targetHandler, BList *arkExtensions)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker (_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    // targetHandler is where the message will be sent when an archive type is selected from the menu
    BPopUpMenu *arkTypePopUp = new BPopUpMenu ("");
    BList arkTypes = ArchiversInstalled (arkExtensions);
    for (int32 i = 0; i < arkTypes.CountItems(); i++)
    {
        BMessage *clickMsg = new BMessage (M_ARK_TYPE_SELECTED);
        clickMsg->AddString (kText, (char*)arkExtensions->ItemAtFast (i));
        BMenuItem *arkTypeItem = new BMenuItem ((char*)arkTypes.ItemAtFast (i), clickMsg);
        arkTypeItem->SetTarget (targetHandler);
        arkTypePopUp->AddItem (arkTypeItem);
    }

    return arkTypePopUp;
}

//=============================================================================================================//

Archiver* NewArchiver (const char *name, bool popupErrors, status_t *returnCode)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker (_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    Archiver *ark = ArchiverForType (name);
    if (!ark)
    {
        *returnCode = B_ERROR;
        return ark;
    }
    
    status_t result = ark->InitCheck();
    switch (result)
    {
        case BZR_BINARY_MISSING:
        {
           if (popupErrors)
           {
               (new BAlert ("error", str (S_BINARY_MISSING), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
                  B_EVEN_SPACING, B_STOP_ALERT))->Go();
           }
           break;
        }
        
        case BZR_OPTIONAL_BINARY_MISSING:
        {
           if (popupErrors)
           {
               (new BAlert ("error", str (S_OPTIONAL_BINARY_MISSING), str (S_OK), NULL, NULL,
                  B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT))->Go();
           }
           break;
        }
    }
    
    *returnCode = result;
    return ark;    
}

//=============================================================================================================//
