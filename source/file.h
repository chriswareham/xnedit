/*******************************************************************************
*                                                                              *
* file.h -- Nirvana Editor File Header File                                    *
*                                                                              *
* Copyright 2004 The NEdit Developers                                          *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version. In addition, you may distribute versions of this program linked to  *
* Motif or Open Motif. See README for details.                                 *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for    *
* more details.                                                                *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* July 31, 2001                                                                *
*                                                                              *
*******************************************************************************/

#ifndef NEDIT_FILE_H_INCLUDED
#define NEDIT_FILE_H_INCLUDED

#include "nedit.h"

#include "../util/getfiles.h"

#include <X11/Intrinsic.h>

/* flags for EditExistingFile */
#define CREATE 1
#define SUPPRESS_CREATE_WARN 2
#define PREF_READ_ONLY 4

#define PROMPT_SBC_DIALOG_RESPONSE 0
#define YES_SBC_DIALOG_RESPONSE 1
#define NO_SBC_DIALOG_RESPONSE 2

typedef struct DefaultEncoding {
    char *locale;
    char *encoding;
} DefaultEncoding;

typedef struct FileContent {
    struct stat statbuf;
    char      *content;
    size_t    length;
    int       readonly;
    int       hasBOM;
    int       fileFormat;
    int       isdir;
    int       isblk;
    int       allocerror;
    int       iconverror;
    int       closeerror;
    int       skipped;
    int       err;
    EncError  *enc_errors;
    size_t    num_enc_errors;
    char      encoding[MAX_ENCODING_LENGTH];
} FileContent;


WindowInfo *EditNewFile(WindowInfo *inWindow, char *geometry, int iconic,
        const char *languageMode, const char *defaultPath);
WindowInfo *EditExistingFile(WindowInfo *inWindow, const char *name,
        const char *path, const char *encoding, const char *filter, int flags,
        char *geometry, int iconic, const char *languageMode, int tabbed,
        int bgOpen);
void RevertToSaved(WindowInfo *window, char *newEncoding);
int SaveWindow(WindowInfo *window);
int SaveWindowAs(WindowInfo *window, FileSelection *file);
int CloseAllFilesAndWindows(void);
int CloseFileAndWindow(WindowInfo *window, int preResponse);
void PrintWindow(WindowInfo *window, int selectedOnly);
void PrintString(const char *string, int length, Widget parent, const char *jobName);
int WriteBackupFile(WindowInfo *window);
int IncludeFile(WindowInfo *window, const char *name, const char *encoding, const char *filter_name);
int PromptForExistingFile(WindowInfo *window, char *prompt, FileSelection *file);
int PromptForNewFile(WindowInfo *window, char *prompt, FileSelection *file,
    	int *fileFormat);
int CheckReadOnly(WindowInfo *window);
void RemoveBackupFile(WindowInfo *window);
void UniqueUntitledName(char *name);
void CheckForChangesToFile(WindowInfo *window);

const char * DetectEncoding(const char *buf, size_t len, const char *def);

int GetFileContent(Widget shell, const char *path, const char *encoding, const char *filter_name, FileContent *content);

#endif /* NEDIT_FILE_H_INCLUDED */
