/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2015 Chris Roberts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Debug.h>
#include <Entry.h>
#include <Message.h>
#include <Path.h>
#include <String.h>
#include <File.h>
#include <NodeInfo.h>
#include <Messenger.h>
#include <Directory.h>
#include <Menu.h>
#include <MenuItem.h>

#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <fstream>

#include "RarArchiver.h"
#include "RarStrings.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"



Archiver* load_archiver()
{
    return new RarArchiver();
}



RarArchiver::RarArchiver()
{
    // The list of supported mimetypes by this add-on, note the first (index 0) mime-type
    // in the list will be the one that will be used while creating files using this add-on
    m_passwordRequired = false;
    m_mimeList.AddItem(strdup("application/x-rar"));
    m_mimeList.AddItem(strdup("application/x-rar-compressed"));
    m_mimeList.AddItem(strdup("application/rar"));
    SetArchiveType("rar");
    SetArchiveExtension(".rar");

    m_error = BZR_DONE;

    // Detect rar binary
    if (IsBinaryFound(m_unrarPath, BZR_UNARK) == false)
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}



status_t RarArchiver::ReadOpen(FILE* fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
         sizeStr[25], packedStr[25], ratioStr[15], dayStr[5], permStr[50],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], dateStr[90], crcStr[25], versionStr[25],
         pathStr[B_PATH_NAME_LENGTH + 1];

    do
    {
        fgets(lineString, len, fp);
    }
    while (!feof(fp) && (strstr(lineString, "--------") == NULL));

    fgets(lineString, len, fp);

    while (!feof(fp) && (strstr(lineString, "--------") == NULL))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString,
               " %[^ ] %[0-9] %[0-9] %[0-9%] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9] %[^ ] %[^ ] %[^\n]",
               permStr, sizeStr, packedStr, ratioStr, dayStr, monthStr, yearStr, hourStr, minuteStr,
               crcStr, pathStr);

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");
        FormatDate(dateStr, 90, &timeStruct);

        BString pathString = pathStr;
        if (permStr[0] == '*')
            m_passwordRequired = true;

        // Check to see if pathStr is as folder, else add it as a file
        if (strstr(permStr, "D") != NULL || permStr[0] == 'd')
        {
            // Critical we add '/' for empty folders as rar doesn't report folder names with '/'
            pathString << '/';
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, packedStr, dateStr,
                                                   timeValue, "-", crcStr));
        }
        else
        {
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, packedStr, dateStr,
                                                   timeValue, "-", crcStr));
        }

        fgets(lineString, len, fp);
    }

    return BZR_DONE;
}



status_t RarArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "v" << "-c-" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle rar unloadable error here

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    exitCode = ReadOpen(out);

    close(outdes[0]);
    fclose(out);

    err = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(err, "- the file header is corrupt");
    close(errdes[0]);
    fclose(err);

    return exitCode;
}



status_t RarArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                              volatile bool* cancel)
{
    BEntry dirEntry;
    entry_ref dirRef;

    dirEntry.SetTo(refToDir);
    status_t exitCode = BZR_DONE;
    if (progress)        // Perform output directory checking only when a messenger is passed
    {
        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false)
            return BZR_EXTRACT_DIR_INIT_ERROR;
    }

    BPath dirPath(refToDir);
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
        return BZR_ARCHIVE_PATH_INIT_ERROR;

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo(kPath, &type, &count);
        if (type != B_STRING_TYPE)
            return BZR_UNKNOWN;
    }

    // Setup argv, fill with selection names if needed
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "x" << "-idp" << "-c-";

    if (progress)        // Only enable extract options when user is NOT viewing
    {
        if (m_settingsMenu->FindItem(kNoOverwrite)->IsMarked())
            m_pipeMgr << "-o-";
        else if (m_settingsMenu->FindItem(kUpdate)->IsMarked())
            m_pipeMgr << "-u";
        else if (m_settingsMenu->FindItem(kFreshen)->IsMarked())
            m_pipeMgr << "-f";
        else
            m_pipeMgr << "-o+";
    }
    else
        m_pipeMgr << "-o+";

    if (m_settingsMenu->FindItem(kKeepBroken)->IsMarked())
        m_pipeMgr << "-kb";

    if (m_settingsMenu->FindItem(kProcessAttributes)->IsMarked() == false)
        m_pipeMgr << "-ee";

    if (Password().Length() > 0)
        m_pipeMgr << ((BString)"-p" << Password());
    else
        m_pipeMgr << "-p-";

    m_pipeMgr << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcards(pathString);
    }

    BString dirStr = dirPath.Path();
    dirStr << '/';
    m_pipeMgr << dirStr.String();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;           // Handle rar unloadable error here

    if (progress)
        resume_thread(tid);
    else
    {
        status_t exitCode;
        wait_for_thread(tid, &exitCode);
    }

    close(errdes[1]);
    close(outdes[1]);

    if (progress)
    {
        out = fdopen(outdes[0], "r");
        exitCode = ReadExtract(out, progress, cancel);
        fclose(out);
    }

    err = fdopen(errdes[0], "r");
    BString errStreamOutput;
    Archiver::ReadStream(err, errStreamOutput);
    if (errStreamOutput.FindFirst("Encrypted file:  CRC failed in ") > 0)
        exitCode = BZR_PASSWORD_ERROR;

    fclose(err);

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}



status_t RarArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    // Reads output of rar while extracting files and updates progress window (thru messenger)
    char lineString[728];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 727, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        if (strncmp(lineString, "Extracting  ", 12) == 0)
        {
            BString lineStr = lineString;
            int32 found = lineStr.FindLast("OK");
            lineStr.Remove(found, lineStr.Length() - found);

            const char* fileName = LeafFromPath(lineStr.String());
            if (fileName && strlen(fileName) > 0)
            {
                updateMessage.RemoveName("text");
                updateMessage.AddString("text", fileName);

                progress->SendMessage(&updateMessage, &reply);
            }
        }
    }

    return BZR_DONE;
}



status_t RarArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "t";;

    if (Password().Length() > 0)
        m_pipeMgr << ((BString)"-p" << Password());
    else
        m_pipeMgr << "-p-";

    m_pipeMgr << "-idp" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle lha unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(outdes[1]);
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadTest(out, outputStr, progress, cancel);
    close(errdes[1]);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen(errdes[0], "r");
        BString errStreamContent;
        Archiver::ReadStream(err, errStreamContent);
        if (errStreamContent.Length() > 0)
            exitCode = BZR_ERRSTREAM_FOUND;

        close(errdes[0]);
        close(outdes[0]);
        fclose(out);
        fclose(err);

        if (exitCode == BZR_ERRSTREAM_FOUND)
        {
            BString outStr = outputStr;
            delete[] outputStr;
            outStr << "\n" << errStreamContent << "\n";
            outputStr = new char[strlen(outStr.String()) + 1];
            strcpy(outputStr, outStr.String());
        }
    }

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    return exitCode;
}



status_t RarArchiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of arj -t
    status_t exitCode = BZR_DONE;
    char lineString[999];
    int32 lineCount = -1;
    BString fullOutputStr;

    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        lineString[strlen(lineString) - 1] = '\0';
        fullOutputStr << lineString << "\n";
        lineCount++;

        // Skip first 4 line which contains Archive: <path of archive> | We don't need this here
        if (lineCount > 3)
        {
            char* testingStr = lineString;
            testingStr += CountCharsInFront(testingStr, ' ');

            if (strncmp(testingStr, "Testing     ", 12) == 0)
            {
                // The format of testingStr is now "Testing   path-here    OK"
                // Number of spaces between path and OK is not constant
                BString pathStr = testingStr;
                if (pathStr.FindLast("OK") < 0)
                    exitCode = BZR_ERRSTREAM_FOUND;

                pathStr.RemoveLast("OK");
                while (pathStr[pathStr.Length() - 1] == ' ')     // Trim right hand side spaces
                    pathStr.RemoveLast(" ");

                if (pathStr.ByteAt(pathStr.Length() - 1) != '/')
                {
                    updateMessage.RemoveName("text");
                    updateMessage.AddString("text", FinalPathComponent(pathStr.String() + 12));
                    progress->SendMessage(&updateMessage, &reply);
                }
            }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());

    return exitCode;
}



bool RarArchiver::SupportsComment() const
{
    //TODO Rar does support comments but it's going to take some work to parse it out of the unrar listing
    return false;
}



bool RarArchiver::CanPartiallyOpen() const
{
    // This is because we cannot determine the EXACT added-path because rar reports it with trailing spaces
    // ans we have no way of finding what spaces are added by rar and what belong to the end of a filename
    return false;
}



status_t RarArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t RarArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
                             volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t RarArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                             BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



void RarArchiver::BuildDefaultMenu()
{
    BMenu *extractMenu, *othersMenu;
    BMenuItem* item;

    m_settingsMenu = new BMenu(m_typeStr);

    // Build the "While Extracting" sub-menu
    extractMenu = new BMenu(kExtracting);
    extractMenu->SetRadioMode(true);

    item = new BMenuItem(kAlwaysOverwrite, NULL);
    item->SetMarked(true);
    extractMenu->AddItem(item);

    item = new BMenuItem(kNoOverwrite, NULL);
    item->SetMarked(false);
    extractMenu->AddItem(item);

    item = new BMenuItem(kUpdate, NULL);
    item->SetMarked(false);
    extractMenu->AddItem(item);

    item = new BMenuItem(kFreshen, NULL);
    item->SetMarked(false);
    extractMenu->AddItem(item);

    // Build the "Other Options" sub-menu
    othersMenu = new BMenu(kOtherOptions);
    othersMenu->SetRadioMode(false);

    item = new BMenuItem(kProcessAttributes, new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    othersMenu->AddItem(item);

    item = new BMenuItem(kKeepBroken, new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    othersMenu->AddItem(item);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(extractMenu);
    m_settingsMenu->AddItem(othersMenu);
}



void RarArchiver::SetMimeType()
{
    // It seems the lha binary resets the mime-type of archives to "application/octet-stream", after
    // operations such as delete, add, create - this function sets it back to the correct type
    BNode node(m_archivePath.Path());
    if (node.InitCheck() == B_OK)
    {
        BNodeInfo nodeInfo(&node);
        nodeInfo.SetType((const char*)m_mimeList.ItemAtFast(0L));
    }
}



bool RarArchiver::NeedsTempDirectory() const
{
    // The comments need temp directory
    return true;
}



bool RarArchiver::SupportsPassword() const
{
    return true;
}



bool RarArchiver::CanDeleteFiles() const
{
    return false;
}


bool RarArchiver::CanAddFiles() const
{
    return false;
}
