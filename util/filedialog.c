/*
 * Copyright 2019 Olaf Wintermann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include "filedialog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>
#include <inttypes.h>

#include "icons.h"

#include <X11/xpm.h>

#include <Xm/PrimitiveP.h>
#include <X11/CoreP.h>

#include"../Microline/XmL/Grid.h"

/* nedit utils */
#include "nedit_malloc.h"
#include "utils.h"
#include "fileUtils.h"
#include "getfiles.h"
#include "misc.h"

#include "../source/preferences.h"

#include "DialogF.h"

#define WIDGET_SPACING 5
#define WINDOW_SPACING 8

#define BUTTON_EXTRA_SPACE 4

#define DATE_FORMAT_SAME_YEAR  "%b %d %H:%M"
#define DATE_FORMAT_OTHER_YEAR "%b %d  %Y"

#define KB_SUFFIX "KiB"
#define MB_SUFFIX "MiB"
#define GB_SUFFIX "GiB"
#define TB_SUFFIX "TiB"

static int pixmaps_initialized = 0;
static int pixmaps_error = 0;
static Pixmap folderIcon;
static Pixmap fileIcon;
static Pixmap folderShape;
static Pixmap fileShape;

static Pixmap newFolderIcon16;
static Pixmap newFolderIcon24;
static Pixmap newFolderIcon32;

static XColor bgColor;

static int LastView = -1; // 0: icon   1: list   2: grid

#define FSB_ENABLE_DETAIL

const char *newFolder16Data = "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377>>>\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377ZZZ\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\064\064\064\377\177\177\177\377\177\177\177\377\177"
  "\177\177\377www\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\000"
  "\000\000\377\363\267/\377\363\267/\377\000\000\000\377\064\064\064\377\064\064\064\377\343"
  "\343\343\377\064\064\064\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377"
  "ooo\377\000\000\000\377\363\267/\377\363\267/\377\000\000\000\377ooo\377^^^\377;;;\377"
  "\064\064\064\377ooo\377ooo\377ooo\377ooo\377ooo\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\061\061\061\377///\377---\377---\377---\377---\377\000\000\000\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\000\000\000\377\061\061\061\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\064\064\064\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064"
  "\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377"
  "\363\267/\377\363\267/\377\000\000\000\377MMM\377MMM\377\064\064\064\377\064\064\064"
  "\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\363"
  "\267/\377\363\267/\377\000\000\000\377MMM\377MMM\377\064\064\064\377\064\064\064\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377MMM\377MMM\377\064\064\064\377\064\064\064\377MMM\377M"
  "MM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM"
  "\377MMM\377MMM\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\064"
  "\064\064\377\064\064\064\377MMM\377MMM\377MMM\377MMM\377KKK\377KKK\377KKK\377"
  "KKK\377KKK\377JJJ\377JJJ\377GGG\377GGG\377GGG\377\064\064\064\377ZZZ\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377ZZZ\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377";

const char *newFolder24Data = "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377EEE\377\064\064\064\377\064\064"
  "\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\065\065\065\377"
  "uuu\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\064"
  "\064\064\377eee\377\202\202\202\377\202\202\202\377\202\202\202\377\202\202"
  "\202\377xxx\377fff\377AAA\377uuu\377\354\354\354\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\363\267/\377\363\267"
  "/\377\363\267/\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\374\374\374\377\064\064\064\377[[[\377ttt\377ttt\377ttt\377ttt\377ttt\377"
  "rrr\377eee\377EEE\377\071\071\071\377\071\071\071\377\071\071\071\377\071\071\071\377"
  "\070\070\070\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377"
  "\070\070\070\377\064\064\064\377;;;\377\264\264\264\377\064\064\064\377XXX\377ooo"
  "\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377kkk\377hhh\377hhh\377hhh\377"
  "hhh\377hhh\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377"
  "hhh\377hhh\377SSS\377\070\070\070\377\064\064\064\377XXX\377ooo\377ooo\377ooo\377"
  "ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377HHH\377VVV\377UUU\377UU"
  "U\377UUU\377UUU\377UUU\377UUU\377UUU\377UUU\377\000\000\000\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377///\377///"
  "\377...\377...\377...\377...\377...\377...\377...\377...\377...\377\000\000\000"
  "\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\000\000\000\377///\377@@@\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\363\267/"
  "\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\000\000\000\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377BBB\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377MMM\377MMM\377"
  "BBB\377\064\064\064\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\363\267"
  "/\377\363\267/\377\363\267/\377\000\000\000\377MMM\377MMM\377BBB\377\064\064\064\377"
  "\064\064\064\377BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM"
  "\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\363\267/\377\363\267/\377"
  "\363\267/\377\000\000\000\377MMM\377MMM\377BBB\377\064\064\064\377\064\064\064\377BB"
  "B\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377M"
  "MM\377MMM\377BBB\377\064\064\064\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377BBB\377\064\064\064\377\064\064"
  "\064\377BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377BBB\377\064\064\064\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377BBB\377\064\064\064\377\064\064\064\377"
  "BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "BBB\377\064\064\064\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377BBB\377\064\064\064\377\064\064\064\377BBB\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377KKK\377KKK\377KKK\377KKK\377@@@\377"
  "\064\064\064\377\064\064\064\377BBB\377MMM\377MMM\377MMM\377MMM\377MMM\377KKK\377"
  "III\377III\377III\377III\377III\377III\377III\377GGG\377GGG\377GGG\377FF"
  "F\377FFF\377FFF\377FFF\377>>>\377\064\064\064\377EEE\377\064\064\064\377\064\064"
  "\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377EEE\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377";

const char *newFolder32Data = "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377UUU\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377>>>\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377>>>"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\064\064\064\377"
  "\064\064\064\377\204\204\204\377\204\204\204\377\204\204\204\377\204\204\204"
  "\377\204\204\204\377\204\204\204\377yyy\377ooo\377UUU\377\064\064\064\377>>"
  ">\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\064\064\064\377\064\064\064"
  "\377yyy\377yyy\377yyy\377yyy\377yyy\377yyy\377yyy\377yyy\377ooo\377UUU\377"
  "\064\064\064\377>>>\377>>>\377>>>\377>>>\377>>>\377\000\000\000\377\000\000\000\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377;;;\377"
  ";;;\377\064\064\064\377\064\064\064\377MMM\377\377\377\377\377\064\064\064\377\064"
  "\064\064\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377"
  "ooo\377UUU\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064"
  "\064\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\363\267"
  "/\377\000\000\000\377\000\000\000\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377MMM\377\064\064\064\377\064\064\064\377ooo\377ooo\377ooo\377o"
  "oo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo"
  "\377ooo\377ooo\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267"
  "/\377\363\267/\377\000\000\000\377\000\000\000\377ooo\377ooo\377ooo\377ooo\377\064\064"
  "\064\377\064\064\064\377\064\064\064\377\064\064\064\377ooo\377ooo\377ooo\377ooo\377"
  "ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/"
  "\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377\064\064\064\377ooo\377o"
  "oo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377ooo\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377+++\377+++\377+++\377+++\377+++\377+++\377+++\377"
  "+++\377+++\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267"
  "/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\000\000\000\377\000\000\000\377...\377...\377...\377...\377.."
  ".\377...\377...\377...\377...\377...\377...\377...\377\000\000\000\377\000\000\000\377"
  "\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267"
  "/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000"
  "\000\000\377...\377...\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377M"
  "MM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267"
  "/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\064\064\064\377\064\064\064\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000"
  "\000\000\377\000\000\000\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267/\377\363\267"
  "/\377\000\000\000\377\000\000\000\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\363\267/\377\363"
  "\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377\064\064\064\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\064\064\064\377\064"
  "\064\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\363"
  "\267/\377\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377MMM\377"
  "MMM\377MMM\377MMM\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\363\267/\377"
  "\363\267/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377MMM\377MMM\377"
  "MMM\377MMM\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\363\267/\377\363\267"
  "/\377\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377MMM\377MMM\377MMM\377"
  "MMM\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377\000\000\000\377\000\000\000\377\363\267/\377\363\267/\377"
  "\363\267/\377\363\267/\377\000\000\000\377\000\000\000\377MMM\377MMM\377MMM\377MMM\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377MMM\377MMM\377MMM\377MMM\377\064\064\064\377\064\064"
  "\064\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377MMM\377MMM\377MMM\377MMM\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM"
  "\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377MMM\377M"
  "MM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM"
  "\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377MMM\377MMM\377MMM\377"
  "MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
  "M\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377HHH\377HHH\377"
  "HHH\377HHH\377HHH\377HHH\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064"
  "\064\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377HHH\377HH"
  "H\377HHH\377HHH\377HHH\377HHH\377HHH\377HHH\377HHH\377HHH\377FFF\377FFF\377"
  "FFF\377FFF\377FFF\377FFF\377FFF\377FFF\377FFF\377FFF\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377UUU\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064"
  "\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064"
  "\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377\064\064\064\377"
  "\064\064\064\377UUU\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377";

/*
 * get number of shifts for specific color mask
 */
static int get_shift(unsigned long mask) {
    if(mask == 0) {
        return 0;
    }
    
    int shift = 0;
    while((mask & 1L) == 0) {
        shift++;
        mask >>= 1;
    }
    return shift;
}

/*
 * get number of bits from color mask
 */
static int get_mask_len(unsigned long mask) {
    if(mask == 0) {
        return 0;
    }
    
    while((mask & 1L) == 0) {
        mask >>= 1;
    }
    int len = 0;
    while((mask & 1L) == 1) {
        len++;
        mask >>= 1;
    }
    return len;
}

/*
 * create an image from data and copy image to the specific pixmap
 */
static void create_image(Display *dp, Visual *visual, int depth, Pixmap pix, const char *data, int wh) {
    // wh is width and height
    size_t imglen = wh*wh*4;
    char *imgdata = malloc(imglen); // will be freed with XDestroyImage
    
    //get number of shifts required for each color bit mask
    int red_shift = get_shift(visual->red_mask);
    int green_shift = get_shift(visual->green_mask);
    int blue_shift = get_shift(visual->blue_mask);
    
    uint32_t *src = (uint32_t*)data;
    uint32_t *dst = (uint32_t*)imgdata;
    // init all bits that are not in any mask
    uint32_t pixel_init = UINT32_MAX ^ (visual->red_mask ^ visual->green_mask ^ visual->blue_mask);
    
    // convert pixels to visual-specific format
    size_t len = wh*wh;
    for(int i=0;i<len;i++) {
        uint32_t pixel = src[i];
        uint8_t red = pixel & 0xFF;
        uint8_t green = (pixel & 0xFF00) >> 8;
        uint8_t blue = (pixel & 0xFF0000) >> 16;
        // TODO: work with alpha value
        if(pixel == UINT32_MAX) {
            red = bgColor.red;
            green = bgColor.green;
            blue = bgColor.blue;
        }
        
        uint32_t out = pixel_init;
        out ^= (red << red_shift);
        out ^= (green << green_shift);
        out ^= (blue << blue_shift);
        dst[i] = out;
    }
    
    // create rgb image (32 bit alignment)
    XImage *img = XCreateImage(dp, visual, depth, ZPixmap, 0, imgdata, wh, wh, 32, 0);
    
    XGCValues gcval;
    gcval.graphics_exposures = 0;
    unsigned long valuemask = GCGraphicsExposures;	
    GC gc = XCreateGC(dp, newFolderIcon16, valuemask, &gcval);
    
    XPutImage(dp, pix, gc, img, 0, 0, 0, 0, wh, wh);
    
    XDestroyImage(img);
    XFreeGC(dp, gc);
}

static void initPixmaps(Display *dp, Drawable d, Screen *screen, int depth)
{
    // TODO: remove Xpm dependency and use create_image
    if(XpmCreatePixmapFromData(dp, d, DtdirB_m_pm, &folderIcon, &folderShape, NULL)) {
        fprintf(stderr, "failed to create folder pixmap\n");
    }
    if(XpmCreatePixmapFromData(dp, d, Dtdata_m_pm, &fileIcon, &fileShape, NULL)) {
        fprintf(stderr, "failed to create file pixmap\n");
    }
      
    // get the correct visual for current screen/depth
    Visual *visual = NULL;
    for(int i=0;i<screen->ndepths;i++) {
        Depth d = screen->depths[i];
        if(d.depth == depth) {
            for(int v=0;v<d.nvisuals;v++) {
                Visual *vs = &d.visuals[v];
                if(get_mask_len(vs->red_mask) == 8) {
                    visual = vs;
                    break;
                }
            }
        }
    }
    
    if(!visual) {
        // no visual found for using rgb images
        fprintf(stderr, "can't use images with this visual\n");
        pixmaps_initialized = 1;
        pixmaps_error = 1;
        return;
    }
    
    // create pixmaps and load images
    newFolderIcon16 = XCreatePixmap(dp, d, 16, 16, depth);
    if(newFolderIcon16 == BadValue) {
        fprintf(stderr, "failed to create newFolderIcon16 pixmap\n");
        pixmaps_error = 1;
    } else {
        create_image(dp, visual, depth, newFolderIcon16, newFolder16Data, 16);
    }
    
    newFolderIcon24 = XCreatePixmap(dp, d, 24, 24, depth);
    if(newFolderIcon24 == BadValue) {
        fprintf(stderr, "failed to create newFolderIcon24 pixmap\n");
        pixmaps_error = 1;
    } else {
        create_image(dp, visual, depth, newFolderIcon24, newFolder24Data, 24);
    }
    
    newFolderIcon32 = XCreatePixmap(dp, d, 32, 32, depth);
    if(newFolderIcon32 == BadValue) {
        fprintf(stderr, "failed to create newFolderIcon32 pixmap\n");
        pixmaps_error = 1;
    } else {
        create_image(dp, visual, depth, newFolderIcon32, newFolder32Data, 32);
    }
    
    pixmaps_initialized = 1;
}


/* -------------------- path utils -------------------- */

char* ConcatPath(const char *parent, const char *name)
{ 
    size_t parentlen = strlen(parent);
    size_t namelen = strlen(name);
    
    size_t pathlen = parentlen + namelen + 2;
    char *path = NEditMalloc(pathlen);
    
    memcpy(path, parent, parentlen);
    if(parentlen > 0 && parent[parentlen-1] != '/') {
        path[parentlen] = '/';
        parentlen++;
    }
    if(name[0] == '/') {
        name++;
        namelen--;
    }
    memcpy(path+parentlen, name, namelen);
    path[parentlen+namelen] = '\0';
    return path;
}

char* FileName(char *path) {
    int si = 0;
    int osi = 0;
    int i = 0;
    int p = 0;
    char c;
    while((c = path[i]) != 0) {
        if(c == '/') {
            osi = si;
            si = i;
            p = 1;
        }
        i++;
    }
    
    char *name = path + si + p;
    if(name[0] == 0) {
        name = path + osi + p;
        if(name[0] == 0) {
            return path;
        }
    }
    
    return name;
}

char* ParentPath(char *path) {
    char *name = FileName(path);
    size_t namelen = strlen(name);
    size_t pathlen = strlen(path);
    size_t parentlen = pathlen - namelen;
    if(parentlen == 0) {
        parentlen++;
    }
    char *parent = NEditMalloc(parentlen + 1);
    memcpy(parent, path, parentlen);
    parent[parentlen] = '\0';
    return parent;
}

/* -------------------- path bar -------------------- */

typedef void(*updatedir_callback)(void*,char*);

typedef struct PathBar {  
    Widget widget;
    Widget textfield;
    
    Widget left;
    Widget right;
    Dimension lw;
    Dimension rw;
    
    int shift;
    
    Widget *pathSegments;
    size_t numSegments;
    size_t segmentAlloc;
    
    char *path;
    int selection;
    Boolean input;
    
    updatedir_callback updateDir;
    void *updateDirData;
} PathBar;

void PathBarSetPath(PathBar *bar, char *path);

void pathbar_resize(Widget w, PathBar *p, XtPointer d)
{
    Dimension width, height;
    XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
    
    Dimension *segW = NEditCalloc(p->numSegments, sizeof(Dimension));
    
    Dimension maxHeight = 0;
    
    /* get width/height from all widgets */
    Dimension pathWidth = 0;
    for(int i=0;i<p->numSegments;i++) {
        Dimension segWidth;
        Dimension segHeight;
        XtVaGetValues(p->pathSegments[i], XmNwidth, &segWidth, XmNheight, &segHeight, NULL);
        segW[i] = segWidth;
        pathWidth += segWidth;
        if(segHeight > maxHeight) {
            maxHeight = segHeight;
        }
    }
    Dimension tfHeight;
    XtVaGetValues(p->textfield, XmNheight, &tfHeight, NULL);
    if(tfHeight > maxHeight) {
        maxHeight = tfHeight;
    }
    
    Boolean arrows = False;
    if(pathWidth + 10 > width) {
        arrows = True;
        pathWidth += p->lw + p->rw;
    }
    
    /* calc max visible widgets */
    int start = 0;
    if(arrows) {
        Dimension vis = p->lw+p->rw;
        for(int i=p->numSegments;i>0;i--) {
            Dimension segWidth = segW[i-1];
            if(vis + segWidth + 10 > width) {
                start = i;
                arrows = True;
                break;
            }
            vis += segWidth;
        }
    } else {
        p->shift = 0;
    }
    
    int leftShift = 0;
    if(p->shift < 0) {
        if(start + p->shift < 0) {
            leftShift = start;
            start = 0;
            p->shift = -leftShift;
        } else {
            leftShift = -p->shift; /* negative shift */
            start += p->shift;
        }
    }
    
    int x = 0;
    if(arrows) {
        XtManageChild(p->left);
        XtManageChild(p->right);
        x = p->lw;
    } else {
        XtUnmanageChild(p->left);
        XtUnmanageChild(p->right);
    }
    
    for(int i=0;i<p->numSegments;i++) {
        if(i >= start && i < p->numSegments - leftShift && !p->input) {
            XtVaSetValues(p->pathSegments[i], XmNx, x, XmNy, 0, XmNheight, maxHeight, NULL);
            x += segW[i];
            XtManageChild(p->pathSegments[i]);
        } else {
            XtUnmanageChild(p->pathSegments[i]);
        }
    }
    
    if(arrows) {
        XtVaSetValues(p->left, XmNx, 0, XmNy, 0, XmNheight, maxHeight, NULL);
        XtVaSetValues(p->right, XmNx, x, XmNy, 0, XmNheight, maxHeight, NULL);
    }
    
    NEditFree(segW);
    
    Dimension rw, rh;
    XtMakeResizeRequest(w, width, maxHeight, &rw, &rh);
    
    XtVaSetValues(p->textfield, XmNwidth, rw, XmNheight, rh, NULL);
}

void pathbar_input(Widget w, PathBar *p, XtPointer c)
{
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*)c;
    XEvent *xevent = cbs->event;
    
    if (cbs->reason == XmCR_INPUT) {
        if (xevent->xany.type == ButtonPress) {
            XtUnmanageChild(p->left);
            XtUnmanageChild(p->right);
            
            XtManageChild(p->textfield);
            p->input = 1;
            
            XmProcessTraversal(p->textfield, XmTRAVERSE_CURRENT);
            
            pathbar_resize(p->widget, p, NULL);
        }
    }
}

void pathbar_losingfocus(Widget w, PathBar *p, XtPointer c)
{
    p->input = False;
    XtUnmanageChild(p->textfield);
}

void pathbar_pathinput(Widget w, PathBar *p, XtPointer d)
{
    char *newpath = XmTextFieldGetString(p->textfield);
    if(newpath) {
        if(newpath[0] == '~') {
            char *p = newpath+1;
            char *cp = ConcatPath(GetHomeDir(), p);
            XtFree(newpath);
            newpath = cp;
        } else if(newpath[0] != '/') {
            char *cp = ConcatPath(GetCurrentDir(), newpath);
            XtFree(newpath);
            newpath = cp;
        }
        
        /* update path */
        PathBarSetPath(p, newpath);
        if(p->updateDir) {
            p->updateDir(p->updateDirData, newpath);
        }
        XtFree(newpath);
        
        /* hide textfield and show path as buttons */
        XtUnmanageChild(p->textfield);
        pathbar_resize(p->widget, p, NULL);
    }
}

void pathbar_shift_left(Widget w, PathBar *p, XtPointer d)
{
    p->shift--;
    pathbar_resize(p->widget, p, NULL);
}

void pathbar_shift_right(Widget w, PathBar *p, XtPointer d)
{
    if(p->shift < 0) {
        p->shift++;
    }
    pathbar_resize(p->widget, p, NULL);
}

PathBar* CreatePathBar(Widget parent, ArgList args, int n)
{
    PathBar *bar = NEditMalloc(sizeof(PathBar));
    bar->path = NULL;
    bar->updateDir = NULL;
    bar->updateDirData = NULL;
    
    bar->shift = 0;
    
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    bar->widget = XmCreateDrawingArea(parent, "pathbar", args, n);
    XtAddCallback(
            bar->widget,
            XmNresizeCallback,
            (XtCallbackProc)pathbar_resize,
            bar);
    XtAddCallback(
            bar->widget,
            XmNinputCallback,
            (XtCallbackProc)pathbar_input,
            bar);
    
    Arg a[4];
    XtSetArg(a[0], XmNshadowThickness, 0);
    XtSetArg(a[1], XmNx, 0);
    XtSetArg(a[2], XmNy, 0);
    bar->textfield = XmCreateTextField(bar->widget, "pbtext", a, 3);
    bar->input = 0;
    XtAddCallback(
            bar->textfield,
            XmNlosingFocusCallback,
            (XtCallbackProc)pathbar_losingfocus,
            bar);
    XtAddCallback(bar->textfield, XmNactivateCallback,
                 (XtCallbackProc)pathbar_pathinput, bar);
    
    XtSetArg(a[0], XmNarrowDirection, XmARROW_LEFT);
    bar->left = XmCreateArrowButton(bar->widget, "pbbutton", a, 1);
    XtSetArg(a[0], XmNarrowDirection, XmARROW_RIGHT);
    bar->right = XmCreateArrowButton(bar->widget, "pbbutton", a, 1);
    XtAddCallback(
                bar->left,
                XmNactivateCallback,
                (XtCallbackProc)pathbar_shift_left,
                bar);
    XtAddCallback(
                bar->right,
                XmNactivateCallback,
                (XtCallbackProc)pathbar_shift_right,
                bar);
    
    Pixel bg;
    XtVaGetValues(bar->textfield, XmNbackground, &bg, NULL);
    XtVaSetValues(bar->widget, XmNbackground, bg, NULL);
    
    XtManageChild(bar->left);
    XtManageChild(bar->right);
    
    XtVaGetValues(bar->left, XmNwidth, &bar->lw, NULL);
    XtVaGetValues(bar->right, XmNwidth, &bar->rw, NULL);
    
    bar->segmentAlloc = 16;
    bar->numSegments = 0;
    bar->pathSegments = calloc(16, sizeof(Widget));
    
    bar->selection = 0;
    
    return bar;
}

void PathBarChangeDir(Widget w, PathBar *bar, XtPointer c)
{
    XmToggleButtonSetState(bar->pathSegments[bar->selection], False, False);
    
    for(int i=0;i<bar->numSegments;i++) {  
        if(bar->pathSegments[i] == w) {
            bar->selection = i;
            XmToggleButtonSetState(w, True, False);
            break;
        }
    }
    
    int plen = strlen(bar->path);
    int countSeg = 0;
    for(int i=0;i<=plen;i++) {
        char c = bar->path[i];
        if(c == '/' || c == '\0') {
            if(countSeg == bar->selection) {
                char *dir = NEditMalloc(i+2);
                memcpy(dir, bar->path, i+1);
                dir[i+1] = '\0';
                if(bar->updateDir) {
                    bar->updateDir(bar->updateDirData, dir);
                }
                NEditFree(dir);
            }
            countSeg++;
        }
    }
}

void PathBarSetPath(PathBar *bar, char *path)
{
    if(bar->path) {
        NEditFree(bar->path);
    }
    bar->path = NEditStrdup(path);
    
    for(int i=0;i<bar->numSegments;i++) {
        XtDestroyWidget(bar->pathSegments[i]);
    }
    XtUnmanageChild(bar->textfield);
    XtManageChild(bar->left);
    XtManageChild(bar->right);
    bar->input = False;
    
    Arg args[4];
    int n;
    XmString str;
    
    bar->numSegments = 0;
    
    int i=0;
    if(path[0] == '/') {
        n = 0;
        str = XmStringCreateLocalized("/");
        XtSetArg(args[0], XmNlabelString, str);
        XtSetArg(args[1], XmNfillOnSelect, True);
        XtSetArg(args[2], XmNindicatorOn, False);
        bar->pathSegments[0] = XmCreateToggleButton(
                bar->widget, "pbbutton", args, 3);
        XtAddCallback(
                bar->pathSegments[0],
                XmNvalueChangedCallback,
                (XtCallbackProc)PathBarChangeDir,
                bar);
        XmStringFree(str);
        bar->numSegments++;
        i++;
    }
    
    int len = strlen(path);
    int begin = i;
    for(;i<=len;i++) {
        char c = path[i];
        if((c == '/' || c == '\0') && i > begin+1) {
            char *segStr = NEditMalloc(i - begin + 1);
            memcpy(segStr, path+begin, i-begin);
            segStr[i-begin] = '\0';
            begin = i+1;
            
            str = XmStringCreateLocalized(segStr);
            NEditFree(segStr);
            XtSetArg(args[0], XmNlabelString, str);
            XtSetArg(args[1], XmNfillOnSelect, True);
            XtSetArg(args[2], XmNindicatorOn, False);
            Widget button = XmCreateToggleButton(bar->widget, "pbbutton", args, 3);
            XtAddCallback(
                    button,
                    XmNvalueChangedCallback,
                    (XtCallbackProc)PathBarChangeDir,
                    bar);
            XmStringFree(str);
            
            if(bar->numSegments >= bar->segmentAlloc) {
                bar->segmentAlloc += 8;
                bar->pathSegments = realloc(bar->pathSegments, bar->segmentAlloc * sizeof(Widget));
            }
            
            bar->pathSegments[bar->numSegments++] = button;
        }
    }
    
    bar->selection = bar->numSegments-1;
    XmToggleButtonSetState(bar->pathSegments[bar->selection], True, False);
    
    XmTextFieldSetString(bar->textfield, path);
    XmTextFieldSetInsertionPosition(bar->textfield, XmTextFieldGetLastPosition(bar->textfield));
    
    pathbar_resize(bar->widget, bar, NULL);
}




/* -------------------- file dialog -------------------- */

typedef struct FileElm FileElm;
struct FileElm {
    char *path;
    int isDirectory;
    uint64_t size;
    time_t lastModified;
};

typedef struct FileDialogData {
    Widget shell;
    
    Widget path;
    PathBar *pathBar;
    Widget filter;
    
    int selectedview;
    
    // icon view
    Widget scrollw;
    Widget container;
    
    // dir/file list view
    Widget listform;
    Widget dirlist;
    Widget filelist;
    Widget filelistcontainer;
    
    // detail view
    Widget grid;
    Widget gridcontainer;
    
    Widget name;
    Widget wrap;
    Widget unixFormat;
    Widget dosFormat;
    Widget macFormat;
    Widget encoding;
    Widget bom;
    Widget xattr;
    
    FileElm *dirs;
    FileElm *files;
    int dircount;
    int filecount;
    int maxnamelen;
    
    WidgetList gadgets;
    int numGadgets;
    
    char *currentPath;
    char *selectedPath;
    int selIsDir;
    int showHidden;
      
    int type;
    
    int end;
    int status;
} FileDialogData;

static void filedialog_cancel(Widget w, FileDialogData *data, XtPointer d)
{
    data->end = 1;
    data->status = FILEDIALOG_CANCEL;
}

static int cleanupFileView(FileDialogData *data)
{
    if(!data->gadgets) {
        return 0;
    }
    
    XtUnmanageChildren(data->gadgets, data->numGadgets);
    for(int i=0;i<data->numGadgets;i++) {
        XtDestroyWidget(data->gadgets[i]);
    }
    
    int ret = data->numGadgets;
    NEditFree(data->gadgets);
    data->gadgets = NULL;
    data->numGadgets = 0;
    return ret;
}

static void cleanupLists(FileDialogData *data)
{
    XmListDeleteAllItems(data->dirlist);
    XmListDeleteAllItems(data->filelist);
}

static void filedialog_cleanup(FileDialogData *data)
{
    cleanupFileView(data);
    if(data->selectedPath) {
        NEditFree(data->selectedPath);
    }
    if(data->currentPath) {
        NEditFree(data->currentPath);
    }
}

/*
 * file_cmp_field
 * 0: compare path
 * 1: compare size
 * 2: compare mtime
 */
static int file_cmp_field = 0;

/*
 * 1 or -1
 */
static int file_cmp_order = 1;

static int filecmp(const void *f1, const void *f2)
{
    const FileElm *file1 = f1;
    const FileElm *file2 = f2;
    if(file1->isDirectory != file2->isDirectory) {
        return file1->isDirectory < file2->isDirectory;
    }
    
    int cmp_field = file_cmp_field;
    int cmp_order = file_cmp_order;
    if(file1->isDirectory) {
        cmp_field = 0;
        cmp_order = 1;
    }
    
    int ret = 0;
    switch(cmp_field) {
        case 0: {
            ret = strcmp(FileName(file1->path), FileName(file2->path));
            break;
        }
        case 1: {
            if(file1->size < file2->size) {
                ret = -1;
            } else if(file1->size == file2->size) {
                ret = 0;
            } else {
                ret = 1;
            }
            break;
        }
        case 2: {
            if(file1->lastModified < file2->lastModified) {
                ret = -1;
            } else if(file1->lastModified == file2->lastModified) {
                ret = 0;
            } else {
                ret = 1;
            }
            break;
        }
    }
    
    return ret * cmp_order;
}

static void resize_container(Widget w, FileDialogData *data, XtPointer d) 
{
    Dimension width, height;
    Dimension cw, ch;
    
    XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, NULL);
    XtVaGetValues(data->container, XtNwidth, &cw, XtNheight, &ch, NULL);
    
    if(ch < height) {
        XtVaSetValues(data->container, XtNwidth, width, XtNheight, height, NULL);
    } else {
        XtVaSetValues(data->container, XtNwidth, width, NULL);
    }
}

static void init_container_size(FileDialogData *data)
{
    Widget parent = XtParent(data->container);
    Dimension width;
    
    XtVaGetValues(parent, XtNwidth, &width, NULL);
    XtVaSetValues(data->container, XtNwidth, width, XtNheight, 100, NULL);
}

typedef void(*ViewUpdateFunc)(FileDialogData*,FileElm*,FileElm*,int,int,int);

static void filedialog_update_iconview(
        FileDialogData *data,
        FileElm *dirs,
        FileElm *files,
        int dircount,
        int filecount,
        int maxnamelen)
{
    Arg args[16];
    XmString str;
    int n;
    WidgetList gadgets = NEditCalloc(dircount+filecount, sizeof(Widget));
    
    // TODO: better width calculation
    // FIXME: for some reason setting XmNlargeCellWidth on Solaris doesn't work
#ifndef __sun
    Dimension cellwidth = maxnamelen * 8;
    XtVaSetValues(data->container, XmNlargeCellWidth, cellwidth, NULL);
#endif
    
    char *filter = XmTextFieldGetString(data->filter);
    char *filterStr = filter;
    if(!filter || strlen(filter) == 0) {
        filterStr = "*";
    }
    
    int numgadgets = 0;
    FileElm *ls = dirs;
    int count = dircount;
    int pos = 0;
    for(int i=0;i<2;i++) {
        for(int j=0;j<count;j++) {
            FileElm *e = &ls[j];
            
            char *name = FileName(e->path);
            if((!data->showHidden && name[0] == '.') || (!e->isDirectory && fnmatch(filterStr, name, 0))) {
                continue;
            }
            
            n = 0;
            str = XmStringCreateLocalized(name);
            XtSetArg(args[n], XmNuserData, e); n++;
            XtSetArg(args[n], XmNlabelString, str); n++;
            XtSetArg(args[n], XmNshadowThickness, 0); n++;
            XtSetArg(args[n], XmNpositionIndex, pos); n++;
            XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
            if(e->isDirectory) {
                XtSetArg(args[n], XmNlargeIconPixmap, folderIcon); n++;
                XtSetArg(args[n], XmNlargeIconMask, folderShape); n++;
            } else {
                XtSetArg(args[n], XmNlargeIconPixmap, fileIcon); n++;
                XtSetArg(args[n], XmNlargeIconMask, fileShape); n++;
            }
            Widget item = XmCreateIconGadget(data->container, "table", args, n);
            XtManageChild(item);
            numgadgets++;

            gadgets[pos] = item;
            XmStringFree(str);
            pos++;
        }
        ls = files;
        count = filecount;
    }
    
    if(filter) {
        XtFree(filter);
    }
    
    data->gadgets = gadgets;
    data->numGadgets = numgadgets;
    
    //XmContainerRelayout(data->container);   
    resize_container(XtParent(data->container), data, NULL);
}

static void filelistwidget_add(Widget w, int showHidden, char *filter, FileElm *ls, int count)
{   
    if(count > 0) {
        XmStringTable items = NEditCalloc(count, sizeof(XmString));
        int i = 0;
        
        for(int j=0;j<count;j++) {
            FileElm *e = &ls[j];
            
            char *name = FileName(e->path);
            if((!showHidden && name[0] == '.') || fnmatch(filter, name, 0)) {
                continue;
            }
            
            items[i] = XmStringCreateLocalized(name);
            i++;
        }
        XmListAddItems(w, items, i, 0);
        for(int i=0;i<count;i++) {
            XmStringFree(items[i]);
        }
        NEditFree(items);
    }
}


static void filedialog_update_lists(
        FileDialogData *data,
        FileElm *dirs,
        FileElm *files,
        int dircount,
        int filecount,
        int maxnamelen)
{
    char *filter = XmTextFieldGetString(data->filter);
    char *filterStr = filter;
    if(!filter || strlen(filter) == 0) {
        filterStr = "*";
    }
    
    filelistwidget_add(data->dirlist, data->showHidden, "*", dirs, dircount);
    filelistwidget_add(data->filelist, data->showHidden, filterStr, files, filecount);
    
    if(filter) {
        XtFree(filter);
    }
}

/*
 * create file size string with kb/mb/gb/tb suffix
 */
static char* size_str(FileElm *f) {
    char *str = malloc(16);
    uint64_t size = f->size;
    
    if(f->isDirectory) {
        str[0] = '\0';
    } else if(size < 0x400) {
        snprintf(str, 16, "%d bytes", (int)size);
    } else if(size < 0x100000) {
        float s = (float)size/0x400;
        int diff = (s*100 - (int)s*100);
        if(diff > 90) {
            diff = 0;
            s += 0.10f;
        }
        if(size < 0x2800 && diff != 0) {
            // size < 10 KiB
            snprintf(str, 16, "%.1f " KB_SUFFIX, s);
        } else {
            snprintf(str, 16, "%.0f " KB_SUFFIX, s);
        }
    } else if(size < 0x40000000) {
        float s = (float)size/0x100000;
        int diff = (s*100 - (int)s*100);
        if(diff > 90) {
            diff = 0;
            s += 0.10f;
        }
        if(size < 0xa00000 && diff != 0) {
            // size < 10 MiB
            snprintf(str, 16, "%.1f " MB_SUFFIX, s);
        } else {
            size /= 0x100000;
            snprintf(str, 16, "%.0f " MB_SUFFIX, s);
        }
    } else if(size < 0x1000000000ULL) {
        float s = (float)size/0x40000000;
        int diff = (s*100 - (int)s*100);
        if(diff > 90) {
            diff = 0;
            s += 0.10f;
        }
        if(size < 0x280000000 && diff != 0) {
            // size < 10 GiB
            snprintf(str, 16, "%.1f " GB_SUFFIX, s);
        } else {
            size /= 0x40000000;
            snprintf(str, 16, "%.0f " GB_SUFFIX, s);
        }
    } else {
        size /= 1024;
        float s = (float)size/0x40000000;
        int diff = (s*100 - (int)s*100);
        if(diff > 90) {
            diff = 0;
            s += 0.10f;
        }
        if(size < 0x280000000 && diff != 0) {
            // size < 10 TiB
            snprintf(str, 16, "%.1f " TB_SUFFIX, s);
        } else {
            size /= 0x40000000;
            snprintf(str, 16, "%.0f " TB_SUFFIX, s);
        }
    }
    return str;
}

static char* date_str(time_t tm) {
    struct tm t;
    struct tm n;
    time_t now = time(NULL);
    
    localtime_r(&tm, &t);
    localtime_r(&now, &n);
    
    char *str = malloc(16);
    if(t.tm_year == n.tm_year) {
        strftime(str, 16, DATE_FORMAT_SAME_YEAR, &t);
    } else {
        strftime(str, 16, DATE_FORMAT_OTHER_YEAR, &t);
    }
    return str;
}

static void filegridwidget_add(Widget grid, int showHidden, char *filter, FileElm *ls, int count, int maxWidth)
{
    XmLGridAddRows(grid, XmCONTENT, 1, count);
    
    int row = 0;
    for(int i=0;i<count;i++) {
        FileElm *e = &ls[i];
        
        char *name = FileName(e->path);
        if((!showHidden && name[0] == '.') || (!e->isDirectory && fnmatch(filter, name, 0))) {
            continue;
        }
        
        // name
        XmString str = XmStringCreateLocalized(name);
        XtVaSetValues(grid,
                XmNcolumn, 0, 
                XmNrow, row,
                XmNcellString, str, NULL);
        XmStringFree(str);
        // size
        char *szbuf = size_str(e);
        str = XmStringCreateLocalized(szbuf);
        XtVaSetValues(grid,
                XmNcolumn, 1, 
                XmNrow, row,
                XmNcellString, str, NULL);
        free(szbuf);
        XmStringFree(str);
        // date
        char *datebuf = date_str(e->lastModified);
        str = XmStringCreateLocalized(datebuf);
        XtVaSetValues(grid,
                XmNcolumn, 2, 
                XmNrow, row,
                XmNcellString, str, NULL);
        free(datebuf);
        XmStringFree(str);
        
        XtVaSetValues(grid, XmNrow, row, XmNrowUserData, e, NULL);
        row++;
    }
    
    // remove unused rows
    if(count > row) {
        XmLGridDeleteRows(grid, XmCONTENT, row, count-row);
    }
    
    if(maxWidth < 16) {
        maxWidth = 16;
    }
    
    XtVaSetValues(grid,
        XmNcolumnRangeStart, 0,
        XmNcolumnRangeEnd, 0,
        XmNcolumnWidth, maxWidth,
        XmNcellAlignment, XmALIGNMENT_LEFT,
        XmNcolumnSizePolicy, XmVARIABLE,
        NULL);
    XtVaSetValues(grid,
        XmNcolumnRangeStart, 1,
        XmNcolumnRangeEnd, 1,
        XmNcolumnWidth, 9,
        XmNcellAlignment, XmALIGNMENT_LEFT,
        XmNcolumnSizePolicy, XmVARIABLE,
        NULL);
    XtVaSetValues(grid,
        XmNcolumnRangeStart, 2,
        XmNcolumnRangeEnd, 2,
        XmNcolumnWidth, 16,
        XmNcellAlignment, XmALIGNMENT_RIGHT,
        XmNcolumnSizePolicy, XmVARIABLE,
        NULL);
    
    XmLGridColumn column0 = XmLGridGetColumn(grid, XmCONTENT, 1);
    XmLGridColumn column1 = XmLGridGetColumn(grid, XmCONTENT, 1);
    XmLGridColumn column2 = XmLGridGetColumn(grid, XmCONTENT, 2);
    
    Dimension col0Width = XmLGridColumnWidthInPixels(column1);
    Dimension col1Width = XmLGridColumnWidthInPixels(column1);
    Dimension col2Width = XmLGridColumnWidthInPixels(column2);
    col0Width = 0;
    
    Dimension totalWidth = col0Width + col1Width + col2Width;
    
    Dimension gridWidth = 0;
    Dimension gridShadow = 0;
    XtVaGetValues(grid, XmNwidth, &gridWidth, XmNshadowThickness, &gridShadow, NULL);
    
    Dimension widthDiff = gridWidth - totalWidth - gridShadow - gridShadow;
    
    if(gridWidth > totalWidth) {
            XtVaSetValues(grid,
            XmNcolumnRangeStart, 0,
            XmNcolumnRangeEnd, 0,
            XmNcolumnWidth, col0Width + widthDiff - XmLGridVSBWidth(grid) - 2,
            XmNcolumnSizePolicy, XmCONSTANT,
            NULL);
    }
}

static void filedialog_update_grid(
        FileDialogData *data,
        FileElm *dirs,
        FileElm *files,
        int dircount,
        int filecount,
        int maxnamelen)
{
    char *filter = XmTextFieldGetString(data->filter);
    char *filterStr = filter;
    if(!filter || strlen(filter) == 0) {
        filterStr = "*";
    }
    
    // update dir list
    filelistwidget_add(data->dirlist, data->showHidden, "*", dirs, dircount);
    // update file detail grid
    filegridwidget_add(data->grid, data->showHidden, filterStr, files, filecount, maxnamelen);
    
    if(filter) {
        XtFree(filter);
    }
}

static void cleanupGrid(FileDialogData *data)
{
    // cleanup dir list widget
    XmListDeleteAllItems(data->dirlist);
    
    // cleanup grid
    Cardinal rows = 0;
    XtVaGetValues(data->grid, XmNrows, &rows, NULL);
    XmLGridDeleteRows(data->grid, XmCONTENT, 0, rows);
}

static void free_files(FileElm *ls, int count)
{
    for(int i=0;i<count;i++) {
        if(ls[i].path) {
            free(ls[i].path);
        }
    }
    free(ls);
}

static void filedialog_cleanup_filedata(FileDialogData *data)
{
    free_files(data->dirs, data->dircount);
    free_files(data->files, data->filecount);
    data->dirs = NULL;
    data->files = NULL;
    data->dircount = 0;
    data->filecount = 0;
    data->maxnamelen = 0;
}

#define FILEDIALOG_FALLBACK_PATH "/"

#define FILE_ARRAY_SIZE 1024

void file_array_add(FileElm **files, int *alloc, int *count, FileElm elm) {
    int c = *count;
    int a = *alloc;
    if(c >= a) {
        a *= 2;
        FileElm *newarray = realloc(*files, sizeof(FileElm) * a);
        
        *files = newarray;
        *alloc = a;
    }
    
    (*files)[c] = elm;
    c++;
    *count = c;
}

static void filedialog_update_dir(FileDialogData *data, char *path)
{
    Arg args[16];
    int n;
    XmString str;
    
    ViewUpdateFunc update_view = NULL;
    switch(data->selectedview) {
        case 0: {
            if(cleanupFileView(data)) {
                init_container_size(data);
            }
            update_view = filedialog_update_iconview;
            break;
        }
        case 1: {
            cleanupLists(data);
            update_view = filedialog_update_lists;
            break;
        }
        case 2: {
            cleanupGrid(data);
            update_view = filedialog_update_grid;
            break;
        }
    }
    
    /* read dir and insert items */
    if(path) {
        FileElm *dirs = calloc(sizeof(FileElm), FILE_ARRAY_SIZE);
        FileElm *files = calloc(sizeof(FileElm), FILE_ARRAY_SIZE);
        int dirs_alloc = FILE_ARRAY_SIZE;
        int files_alloc = FILE_ARRAY_SIZE;
        
        int dircount = 0; 
        int filecount = 0;
        size_t maxNameLen = 0;
        DIR *dir = opendir(path);
        if(!dir) {
            if(path == FILEDIALOG_FALLBACK_PATH) {
                // TODO: ERROR
                fprintf(stderr, "Cannot open directory: %s\n", path);
                perror("opendir");
            } else {
                filedialog_update_dir(data, FILEDIALOG_FALLBACK_PATH);
            }
            return;
        }
        filedialog_cleanup_filedata(data);
    
        /* dir reading complete - set the path textfield */  
        XmTextFieldSetString(data->path, path);
        char *oldPath = data->currentPath;
        data->currentPath = NEditStrdup(path);
        if(oldPath) {
            NEditFree(oldPath);
        }
        path = data->currentPath;

        struct dirent *ent;
        while((ent = readdir(dir)) != NULL) {
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            char *entpath = ConcatPath(path, ent->d_name);

            struct stat s;
            if(stat(entpath, &s)) {
                NEditFree(entpath);
                continue;
            }

            FileElm new_entry;
            new_entry.path = entpath;
            new_entry.isDirectory = S_ISDIR(s.st_mode);
            new_entry.size = (uint64_t)s.st_size;
            new_entry.lastModified = s.st_mtime;

            size_t nameLen = strlen(ent->d_name);
            if(nameLen > maxNameLen) {
                maxNameLen = nameLen;
            }

            if(new_entry.isDirectory) {
                file_array_add(&dirs, &dirs_alloc, &dircount, new_entry);
            } else {
                file_array_add(&files, &files_alloc, &filecount, new_entry);
            }
        }
        closedir(dir);
        
        data->dirs = dirs;
        data->files = files;
        data->dircount = dircount;
        data->filecount = filecount;
        data->maxnamelen = maxNameLen;
        
        // sort file arrays
        qsort(dirs, dircount, sizeof(FileElm), filecmp);
        qsort(files, filecount, sizeof(FileElm), filecmp);
    }
    
    update_view(data, data->dirs, data->files,
            data->dircount, data->filecount, data->maxnamelen);
}

static void filedialog_goup(Widget w, FileDialogData *data, XtPointer d)
{
    char *newPath = ParentPath(data->currentPath);
    filedialog_update_dir(data, newPath);
    PathBarSetPath(data->pathBar, newPath);
    NEditFree(newPath);
}

static void filedialog_setselection(
        FileDialogData *data,
        XmContainerSelectCallbackStruct *sel)
{
    if(sel->selected_item_count > 0) {
        FileElm *file = NULL;
        XtVaGetValues(sel->selected_items[0], XmNuserData, &file, NULL);
        if(file) {
            if(data->selectedPath) {
                NEditFree(data->selectedPath);
            }
            data->selectedPath = NEditStrdup(file->path);
            data->selIsDir = file->isDirectory;
            
            if(!file->isDirectory) {
                if(data->name) {
                    XmTextFieldSetString(data->name, FileName(file->path));
                }
            }
        }
    }
}

static void filedialog_select(
        Widget w,
        FileDialogData *data,
        XmContainerSelectCallbackStruct *sel)
{
    filedialog_setselection(data, sel);
}

static void filedialog_action(
        Widget w,
        FileDialogData *data,
        XmContainerSelectCallbackStruct *sel)
{
    filedialog_setselection(data, sel);
    
    if(data->selIsDir) {
        filedialog_update_dir(data, data->selectedPath);
        PathBarSetPath(data->pathBar, data->selectedPath);
    } else {
        data->end = True;
        data->status = FILEDIALOG_OK;
    }
}

char* set_selected_path(FileDialogData *data, XmString item)
{
    char *name = NULL;
    XmStringGetLtoR(item, XmFONTLIST_DEFAULT_TAG, &name);
    if(!name) {
        return NULL;
    }
    char *path = ConcatPath(data->currentPath, name);
    XtFree(name);
    
    if(data->selectedPath) {
        NEditFree(data->selectedPath);
    }
    data->selectedPath = path;
    
    return path;
}

void set_path_from_row(FileDialogData *data, int row, Boolean update) {
    FileElm *elm = NULL;
    XmLGridRow rowPtr = XmLGridGetRow(data->grid, XmCONTENT, row);
    XtVaGetValues(data->grid, XmNrowPtr, rowPtr, XmNrowUserData, &elm, NULL);
    if(!elm) {
        fprintf(stderr, "error: no row data\n");
        return;
    }
    
    char *path = NEditStrdup(elm->path);
        
    if(update) {
        data->end = True;
        data->status = FILEDIALOG_OK;
        data->selIsDir = False;
    }
    if(data->type == FILEDIALOG_SAVE) {
        XmTextFieldSetString(data->name, FileName(path));
    }
    
    if(data->selectedPath) {
        NEditFree(data->selectedPath);
    }
    data->selectedPath = path;
}

void grid_select(Widget w, FileDialogData *data, XmLGridCallbackStruct *cb) {
    set_path_from_row(data, cb->row, False);
}

void grid_activate(Widget w, FileDialogData *data, XmLGridCallbackStruct *cb) {
    set_path_from_row(data, cb->row, True);
}

void grid_header_clicked(Widget w, FileDialogData *data, XmLGridCallbackStruct *cb) { 
    int new_cmp_field = 0;
    switch(cb->column) {
        case 0: {
            new_cmp_field = 0;            
            break;
        }
        case 1: {
            new_cmp_field = 1;
            break;
        }
        case 2: {
            new_cmp_field = 2;
            break;
        }
    }
    
    if(new_cmp_field == file_cmp_field) {
        file_cmp_order = -file_cmp_order; // revert sort order
    } else {
        file_cmp_field = new_cmp_field; // change file cmp order to new field
        file_cmp_order = 1;
    }
    
    int sort_type = file_cmp_order == 1 ? XmSORT_ASCENDING : XmSORT_DESCENDING;
    XmLGridSetSort(data->grid, file_cmp_field, sort_type);
    
    qsort(data->files, data->filecount, sizeof(FileElm), filecmp);
    
    // refresh widget
    filedialog_update_dir(data, NULL);
} 

void dirlist_activate(Widget w, FileDialogData *data, XmListCallbackStruct *cb)
{
    char *path = set_selected_path(data, cb->item);
    if(path) {
        filedialog_update_dir(data, path);
        PathBarSetPath(data->pathBar, path);
        data->selIsDir = TRUE;
    }    
}

void dirlist_select(Widget w, FileDialogData *data, XmListCallbackStruct *cb)
{
    char *path = set_selected_path(data, cb->item);
    if(path) {
        data->selIsDir = TRUE;
    }
}

void filelist_activate(Widget w, FileDialogData *data, XmListCallbackStruct *cb)
{
    char *path = set_selected_path(data, cb->item);
    if(path) {
        data->end = True;
        data->status = FILEDIALOG_OK;
        data->selIsDir = False;
    }
}

void filelist_select(Widget w, FileDialogData *data, XmListCallbackStruct *cb)
{
    if(data->type == FILEDIALOG_SAVE) {
        char *name = NULL;
        XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG, &name);
        XmTextFieldSetString(data->name, name);
        XtFree(name);
    } else {
        char *path = set_selected_path(data, cb->item);
        if(path) {
            data->selIsDir = False;
        }
    }
}
 
static void filedialog_setshowhidden(
        Widget w,
        FileDialogData *data,
        XmToggleButtonCallbackStruct *tb)
{
    data->showHidden = tb->set;
    filedialog_update_dir(data, NULL);
}

static void filedialog_ok(Widget w, FileDialogData *data, XtPointer d)
{
    if(data->selectedPath) {
        if(!data->selIsDir) {
            data->status = FILEDIALOG_OK;
            data->end = True;
            return;
        }
    }
    
    if(data->type == FILEDIALOG_SAVE) {
        char *newName = XmTextFieldGetString(data->name);
        if(newName) {
            if(strlen(newName) > 0) {
                data->selectedPath = ConcatPath(data->currentPath, newName);
                data->status = FILEDIALOG_OK;
                data->end = True;
                data->selIsDir = 0;
            }
            XtFree(newName);
        }
    }
}

#define DETECT_ENCODING "detect"
static char *default_encodings[] = {
    DETECT_ENCODING,
    "UTF-8",
    "UTF-16",
    "UTF-16BE",
    "UTF-16LE",
    "UTF-32",
    "UTF-32BE",
    "UTF-32LE",
    "ISO8859-1",
    "ISO8859-2",
    "ISO8859-3",
    "ISO8859-4",
    "ISO8859-5",
    "ISO8859-6",
    "ISO8859-7",
    "ISO8859-8",
    "ISO8859-9",
    "ISO8859-10",
    "ISO8859-13",
    "ISO8859-14",
    "ISO8859-15",
    "ISO8859-16",
    NULL
};

static void adjust_enc_settings(FileDialogData *data){
    /* this should never happen, but make sure it will not do anything */
    if(data->type != FILEDIALOG_SAVE) return;
    
    int encPos;
    XtVaGetValues(data->encoding, XmNselectedPosition, &encPos, NULL);
    
    /*
     * the save file dialog doesn't has the "detect" item
     * the default_encodings index is encPos + 1
     */
    if(encPos > 6) {
        /* no unicode no bom */
        XtSetSensitive(data->bom, False);
        if(GetAutoEnableXattr()) {
            XtVaSetValues(data->xattr, XmNset, 1, NULL);
        }
    } else {
        XtSetSensitive(data->bom, True);
        if(encPos > 0) {
            /* enable bom for all non-UTF-8 unicode encodings */
            XtVaSetValues(data->bom, XmNset, 1, NULL);
        }
    }
}

static void filedialog_select_encoding(
        Widget w,
        FileDialogData *data,
        XmComboBoxCallbackStruct *cb)
{
    if(cb->reason == XmCR_SELECT) {
        adjust_enc_settings(data);
    }
}

static void filedialog_filter(Widget w, FileDialogData *data, XtPointer c)
{
    filedialog_update_dir(data, NULL);
}

static void unselect_view(FileDialogData *data)
{
    switch(data->selectedview) {
        case 0: {
            XtUnmanageChild(data->scrollw);
            cleanupFileView(data);
            break;
        }
        case 1: {
            XtUnmanageChild(data->listform);
            XtUnmanageChild(data->filelistcontainer);
            cleanupLists(data);
            break;
        }
        case 2: {
            XtUnmanageChild(data->listform);
            XtUnmanageChild(data->gridcontainer);
            cleanupGrid(data);
            
            // reset sort options and resort files
            file_cmp_field = 0;
            file_cmp_order = 1;
            qsort(data->files, data->filecount, sizeof(FileElm), filecmp);
            
            break;
        }
    }
}

static void select_iconview(Widget w, FileDialogData *data, XtPointer u)
{
    unselect_view(data);
    data->selectedview = 0;
    XtManageChild(data->scrollw);
    filedialog_update_dir(data, NULL);
    filedialog_update_dir(data, NULL); // workaround for what I do not know
}

static void select_listview(Widget w, FileDialogData *data, XtPointer u)
{
    unselect_view(data);
    data->selectedview = 1;
    XtManageChild(data->listform);
    XtManageChild(data->filelistcontainer);
    filedialog_update_dir(data, NULL);
}

static void select_detailview(Widget w, FileDialogData *data, XtPointer u)
{
    XmLGridSetSort(data->grid, 0, XmSORT_ASCENDING);
    
    unselect_view(data);
    data->selectedview = 2;
    XtManageChild(data->listform);
    XtManageChild(data->gridcontainer);
    filedialog_update_dir(data, NULL);
}

static void new_folder(Widget w, FileDialogData *data, XtPointer u)
{
    char fileName[DF_MAX_PROMPT_LENGTH];
    fileName[0] = 0;
    
    int response = DialogF(
            DF_PROMPT,
            data->shell,
            2,
            "Create new directory", "Directory name:",
            fileName,
            "OK",
            "Cancel");
    
    if(response == 2 || strlen(fileName) == 0) {
        return;
    }
    
    char *newFolder = ConcatPath(data->currentPath ? data->currentPath : "", fileName);
    if(mkdir(newFolder, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
        DialogF(
                DF_ERR,
                data->shell,
                1,
                "Error creating Directory",
                "Can't create %s:\n%s", "OK",
                newFolder,
                strerror(errno));
    } else {
        char *p = strdup(data->currentPath);
        filedialog_update_dir(data, p);
        free(p);
    }
    free(newFolder);
}

int FileDialog(Widget parent, char *promptString, FileSelection *file, int type)
{
    Arg args[32];
    int n = 0;
    XmString str;
    
    int currentEncItem = 0;
    
    if(LastView == -1) {
        LastView = GetFsbView();
        if(LastView < 0 || LastView > 2) {
            LastView = 1;
        }
    }
#ifndef FSB_ENABLE_DETAIL
    if(LastView == 2) {
        LastView = 1;
    }
#endif
    Boolean showHiddenValue = GetFsbShowHidden();
    
    FileDialogData data;
    memset(&data, 0, sizeof(FileDialogData));
    data.type = type;
    
    file->addwrap = FALSE;
    file->setxattr = FALSE;
    
    Widget dialog = CreateDialogShell(parent, promptString, args, 0);
    AddMotifCloseCallback(dialog, (XtCallbackProc)filedialog_cancel, &data);
    data.shell = dialog;
    
    n = 0;
    XtSetArg(args[n],  XmNautoUnmanage, False); n++;
    Widget form = XmCreateForm(dialog, "form", args, n);
    
    /* upper part of the gui */
     
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNresizable, True); n++;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_UP); n++;
    Widget goUp = XmCreateArrowButton(form, "button", args, n);
    //XtManageChild(goUp);
    XtAddCallback(goUp, XmNactivateCallback,
                 (XtCallbackProc)filedialog_goup, &data);
    
    // View Option Menu
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    Widget viewframe = XmCreateForm(form, "vframe", args, n);
    XtManageChild(viewframe);
    
    XmString v0 = XmStringCreateLocalized("Icons");
    XmString v1 = XmStringCreateLocalized("List");
    XmString v2 = XmStringCreateLocalized("Detail");
    
    Widget menu = XmCreatePulldownMenu(viewframe, "menu", NULL, 0);
    
    XtSetArg(args[0], XmNlabelString, v0);
    XtSetArg(args[1], XmNpositionIndex, LastView == 0 ? 0 : 1);
    Widget mitem0 = XmCreatePushButton(menu, "menuitem", args, 2);
    XtSetArg(args[0], XmNlabelString, v1);
    XtSetArg(args[1], XmNpositionIndex, LastView == 1 ? 0 : 1);
    Widget mitem1 = XmCreatePushButton(menu, "menuitem", args, 2);
    XtSetArg(args[0], XmNlabelString, v2);
    XtSetArg(args[1], XmNpositionIndex, LastView == 2 ? 0 : 2);
    Widget mitem2 = XmCreatePushButton(menu, "menuitem", args, 2);
    XtManageChild(mitem0);
    XtManageChild(mitem1);
#ifdef FSB_ENABLE_DETAIL
    XtManageChild(mitem2);
#endif
    XmStringFree(v0);
    XmStringFree(v1);
    XmStringFree(v2);
    XtAddCallback(
            mitem0,
            XmNactivateCallback,
            (XtCallbackProc)select_iconview,
            &data);
    XtAddCallback(
            mitem1,
            XmNactivateCallback,
            (XtCallbackProc)select_listview,
            &data);
    XtAddCallback(
            mitem2,
            XmNactivateCallback,
            (XtCallbackProc)select_detailview,
            &data);
     
    n = 0;
    XtSetArg(args[n], XmNsubMenuId, menu); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    Widget view = XmCreateOptionMenu(viewframe, "option_menu", args, n);
    XtManageChild(view);
    
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightWidget, view); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    Widget newFolder = XmCreatePushButton(viewframe, "newFolder", args, n);
    XtManageChild(newFolder);
    XtAddCallback(
            newFolder,
            XmNactivateCallback,
            (XtCallbackProc)new_folder,
            &data);
    
    Dimension xh;
    Pixel buttonFg;
    Pixel buttonBg;
    XtVaGetValues(newFolder, XmNheight, &xh, XmNforeground, &buttonFg, XmNbackground, &buttonBg, NULL);
    
    // get rgb value of buttonBg
    memset(&bgColor, 0, sizeof(XColor));
    bgColor.pixel = buttonBg;
    XQueryColor(XtDisplay(newFolder), newFolder->core.colormap, &bgColor);
    
    // init pixmaps after we got the background color
    if(!pixmaps_initialized) {
        initPixmaps(XtDisplay(parent), XtWindow(parent), newFolder->core.screen, newFolder->core.depth);
    }
    
    if(pixmaps_error) {
        XtVaSetValues(newFolder, XmNlabelType, XmSTRING, NULL);
    } else if(xh > 32+BUTTON_EXTRA_SPACE) {
        XtVaSetValues(newFolder, XmNlabelPixmap, newFolderIcon32, NULL);
    } else if(xh > 24+BUTTON_EXTRA_SPACE) {
        XtVaSetValues(newFolder, XmNlabelPixmap, newFolderIcon24, NULL);
    } else {
        XtVaSetValues(newFolder, XmNlabelPixmap, newFolderIcon16, NULL);
    }
    
    // pathbar
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, goUp); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, viewframe); n++;
    XtSetArg(args[n], XmNrightOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_IN); n++;
    Widget pathBarFrame = XmCreateFrame(form, "pathbar_frame", args, n);
    XtManageChild(pathBarFrame);
    data.pathBar = CreatePathBar(pathBarFrame, args, 0);
    data.pathBar->updateDir = (updatedir_callback)filedialog_update_dir;
    data.pathBar->updateDirData = &data;
    XtManageChild(data.pathBar->widget);
    data.path = XmCreateTextField(form, "textfield", args, 0);
    
    n = 0;
    str = XmStringCreateLocalized("Filter");
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, pathBarFrame); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    Widget filterLabel = XmCreateLabel(form, "label", args, n);
    XtManageChild(filterLabel);
    XmStringFree(str);
    
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, filterLabel); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, WINDOW_SPACING); n++;
    Widget filterform = XmCreateForm(form, "filterform", args, n);
    XtManageChild(filterform);
    
    n = 0;
    str = XmStringCreateSimple("Show hidden files");
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    XtSetArg(args[n], XmNset, showHiddenValue); n++;
    Widget showHidden = XmCreateToggleButton(filterform, "showHidden", args, n);
    XtManageChild(showHidden);
    XmStringFree(str);
    XtAddCallback(showHidden, XmNvalueChangedCallback,
                 (XtCallbackProc)filedialog_setshowhidden, &data);
    data.showHidden = showHiddenValue;
    
    n = 0;
    str = XmStringCreateLocalized("Filter");
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, showHidden); n++;
    Widget filterButton = XmCreatePushButton(filterform, "filedialog_filter", args, n);
    XtManageChild(filterButton);
    XmStringFree(str);
    XtAddCallback(filterButton, XmNactivateCallback,
                 (XtCallbackProc)filedialog_filter, &data);
    
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, filterButton); n++;
    XtSetArg(args[n], XmNrightOffset, WIDGET_SPACING); n++;
    data.filter = XmCreateTextField(filterform, "filedialog_filter_textfield", args, n);
    XtManageChild(data.filter);
    XmTextFieldSetString(data.filter, "*");
    XtAddCallback(data.filter, XmNactivateCallback,
                 (XtCallbackProc)filedialog_filter, &data);
    
    /* lower part */
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING * 2); n++;
    Widget buttons = XmCreateForm(form, "buttons", args, n);
    XtManageChild(buttons);
    
    n = 0;
    str = XmStringCreateLocalized(type == FILEDIALOG_OPEN ? "Open" : "Save");
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    Widget okBtn = XmCreatePushButton(buttons, "filedialog_open", args, n);
    XtManageChild(okBtn);
    XmStringFree(str);
    XtAddCallback(okBtn, XmNactivateCallback,
                 (XtCallbackProc)filedialog_ok, &data);
    
    n = 0;
    str = XmStringCreateLocalized("Cancel");
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    Widget cancelBtn = XmCreatePushButton(buttons, "filedialog_cancel", args, n);
    XtManageChild(cancelBtn);
    XmStringFree(str);
    XtAddCallback(cancelBtn, XmNactivateCallback,
                 (XtCallbackProc)filedialog_cancel, &data);
    
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, buttons); n++;
    XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 1); n++;
    Widget separator = XmCreateSeparator(form, "ofd_separator", args, n);
    XtManageChild(separator);
    
    Widget bottomWidget = separator;
    
    if(type == FILEDIALOG_SAVE) {
        n = 0;
        XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNbottomWidget, separator); n++;
        XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
        XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
        XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNrightOffset, WINDOW_SPACING); n++;
        data.name = XmCreateTextField(form, "textfield", args, n);
        XtManageChild(data.name);
        XtAddCallback(data.name, XmNactivateCallback,
                 (XtCallbackProc)filedialog_ok, &data);

        n = 0;
        str = XmStringCreateSimple("New File Name");
        XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNbottomWidget, data.name); n++;
        XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
        XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
        XtSetArg(args[n], XmNlabelString, str); n++;
        Widget nameLabel = XmCreateLabel(form, "label", args, n);
        XtManageChild(nameLabel);

        n = 0;
        str = XmStringCreateSimple("Add line breaks where wrapped");
        XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNbottomWidget, nameLabel); n++;
        XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
        XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
        XtSetArg(args[n], XmNmnemonic, 'A'); n++;
        XtSetArg(args[n], XmNlabelString, str); n++;
        data.wrap = XmCreateToggleButton(form, "addWrap", args, n);
        XtManageChild(data.wrap);
        XmStringFree(str);

        Widget formatBtns = CreateFormatButtons(
                form,
                data.wrap,
                file->format,
                &data.unixFormat,
                &data.dosFormat,
                &data.macFormat);
        
        bottomWidget = formatBtns;
    }
    
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    Widget enc = XmCreateRowColumn(form, "enc", args, n);
    XtManageChild(enc);
    
    if(file->setenc) {
        n = 0;
        str = XmStringCreateSimple("Encoding:");
        XtSetArg(args[n], XmNlabelString, str); n++;
        Widget encLabel = XmCreateLabel(enc, "label", args, n);
        XtManageChild(encLabel);
        XmStringFree(str);

        n = 0;
        int arraylen = 22;
        
        // TODO: code dup of encoding list generation (window.c)
        
        char *encStr;
        XmStringTable encodings = NEditCalloc(arraylen, sizeof(XmString));
        /* skip the "detect" item on type == save */
        int skip = type == FILEDIALOG_OPEN ? 0 : 1;
        char *defEncoding = type == FILEDIALOG_OPEN ? NULL : file->encoding;
        int hasDef = 0;
        int i;
        for(i=skip;(encStr=default_encodings[i]);i++) {
            if(i >= arraylen) {
                arraylen *= 2;
                encodings = NEditRealloc(encodings, arraylen * sizeof(XmString));
            }
            encodings[i] = XmStringCreateSimple(encStr);
            if(defEncoding) {
                if(!strcasecmp(defEncoding, encStr)) {
                    hasDef = 1;
                    defEncoding = NULL;
                }
            }
        }
        if(skip == 1 && !hasDef && file->encoding) {
            /* Current encoding is not in the list of
             * default encodings
             * Add an extra item at pos 0 for the current encoding
             */
            encodings[0] = XmStringCreateSimple(file->encoding);
            currentEncItem = 1;
            skip = 0;
        }
        XtSetArg(args[n], XmNcolumns, 11); n++;
        XtSetArg(args[n], XmNitemCount, i-skip); n++;
        XtSetArg(args[n], XmNitems, encodings+skip); n++;
        data.encoding = XmCreateDropDownList(enc, "combobox", args, n);
        XtManageChild(data.encoding);
        for(int j=0;j<i;j++) {
            XmStringFree(encodings[j]);
        }
        NEditFree(encodings);
        
        if(file->encoding) {
            char *encStr = NEditStrdup(file->encoding);
            size_t encLen = strlen(encStr);
            for(int i=0;i<encLen;i++) {
                encStr[i] = toupper(encStr[i]);
            }
            str = XmStringCreateSimple(encStr);
            XmComboBoxSelectItem(data.encoding, str);
            XmStringFree(str);
            NEditFree(encStr);
        }
        
        /* bom and xattr option */
        if(type == FILEDIALOG_SAVE) {
            /* only the save file dialog needs an encoding select callback */
            XtAddCallback(
                    data.encoding,
                    XmNselectionCallback,
                    (XtCallbackProc)filedialog_select_encoding,
                    &data);
            
            n = 0;
            str = XmStringCreateSimple("Write BOM");
            XtSetArg(args[n], XmNlabelString, str); n++;
            XtSetArg(args[n], XmNset, file->writebom); n++;
            data.bom = XmCreateToggleButton(enc, "togglebutton", args, n);
            XtManageChild(data.bom);
            XmStringFree(str);
            
            n = 0;
            str = XmStringCreateSimple("Store encoding in extended attribute");
            XtSetArg(args[n], XmNlabelString, str); n++;
            data.xattr = XmCreateToggleButton(enc, "togglebutton", args, n);
            XtManageChild(data.xattr);
            XmStringFree(str);
        }
        
    }
    
    /* middle */
    data.selectedview = LastView;
    
    // form for dir/file lists
    int layout = 2;
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, filterform); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, enc); n++;
    XtSetArg(args[n], XmNleftOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNrightOffset, WINDOW_SPACING); n++;
    XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNwidth, 580); n++;
    XtSetArg(args[n], XmNheight, 400); n++;
    layout = n;
    data.listform = XmCreateForm(form, "fds_listform", args, n); 
    
    // icon view
    n = layout;
    XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC); n++;
    //XtSetArg(args[n], XmNwidth, 580); n++;
    //XtSetArg(args[n], XmNheight, 400); n++;
    Widget scrollw = XmCreateScrolledWindow(form, "scroll_win", args, n);
    data.scrollw = scrollw;
    
    n = 0;
    XtSetArg(args[n], XmNlayoutType,  XmSPATIAL); n++;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNentryViewType, XmLARGE_ICON); n++;
    XtSetArg(args[n], XmNspatialStyle, XmGRID); n++;
    XtSetArg(args[n], XmNspatialIncludeModel, XmAPPEND); n++;
    XtSetArg(args[n], XmNspatialResizeModel, XmGROW_MINOR); n++;
    XtSetArg(args[n], XmNlargeCellWidth, 150); n++;
    data.container = XmCreateContainer(scrollw, "table", args, n);
    XtManageChild(data.container);
    XtAddCallback(XtParent(data.container), XmNresizeCallback,
		(XtCallbackProc)resize_container, &data);
    XmContainerAddMouseWheelSupport(data.container);
    
    XtAddCallback(
            data.container,
            XmNselectionCallback,
            (XtCallbackProc)filedialog_select,
            &data);
    XtAddCallback(
            data.container,
            XmNdefaultActionCallback,
            (XtCallbackProc)filedialog_action,
            &data);
    
    // dir/file lists
    n = 0;
    str = XmStringCreateLocalized("Directories");
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    Widget lsDirLabel = XmCreateLabel(data.listform, "label", args, n);
    XtManageChild(lsDirLabel);
    XmStringFree(str);
    
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, lsDirLabel); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 35); n++;
    data.dirlist = XmCreateScrolledList(data.listform, "dirlist", args, n);
    Dimension w, h;
    XtMakeResizeRequest(data.dirlist, 150, 200, &w, &h);
    XtManageChild(data.dirlist);
    XtAddCallback(
            data.dirlist,
            XmNdefaultActionCallback,
            (XtCallbackProc)dirlist_activate,
            &data); 
    XtAddCallback(
            data.dirlist,
            XmNbrowseSelectionCallback,
            (XtCallbackProc)dirlist_select,
            &data);
    
    n = 0;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, lsDirLabel); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, data.dirlist); n++;
    XtSetArg(args[n], XmNleftOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    data.filelistcontainer = XmCreateFrame(data.listform, "filelistframe", args, n);
    //XtManageChild(data.filelistcontainer);
    
    data.filelist = XmCreateScrolledList(data.filelistcontainer, "filelist", NULL, 0);
    XtManageChild(data.filelist);
    XtAddCallback(
            data.filelist,
            XmNdefaultActionCallback,
            (XtCallbackProc)filelist_activate,
            &data); 
    XtAddCallback(
            data.filelist,
            XmNbrowseSelectionCallback,
            (XtCallbackProc)filelist_select,
            &data);
    
    // Detail FileList
    // the detail view shares widgets with the list view
    // switching between list and detail view only changes
    // filelistcontainer <-> gridcontainer
    n = 0;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, lsDirLabel); n++;
    XtSetArg(args[n], XmNtopOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, data.dirlist); n++;
    XtSetArg(args[n], XmNleftOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    data.gridcontainer = XmCreateFrame(data.listform, "gridcontainer", args, n);
    //XtManageChild(data.gridcontainer);
    
    n = 0;
    XtSetArg(args[n], XmNcolumns, 3); n++;
    XtSetArg(args[n], XmNheadingColumns, 0); n++;
    XtSetArg(args[n], XmNheadingRows, 1); n++;
    XtSetArg(args[n], XmNallowColumnResize, 1); n++;
    XtSetArg(args[n], XmNsimpleHeadings, "Name|Size|Last Modified"); n++;
    XtSetArg(args[n], XmNhorizontalSizePolicy, XmCONSTANT); n++;
    
    data.grid = XmLCreateGrid(data.gridcontainer, "grid", args, n);
    XtManageChild(data.grid);
    
    XtVaSetValues(
            data.grid,
            XmNcellDefaults, True,
            XtVaTypedArg, XmNblankBackground, XmRString, "white", 6,
            XtVaTypedArg, XmNcellBackground, XmRString, "white", 6,
            NULL);
    
    //XmLGridSetStrings(data.grid, "Name|Size|Last Modified");
    XtAddCallback(data.grid, XmNselectCallback, (XtCallbackProc)grid_select, &data);
    XtAddCallback(data.grid, XmNactivateCallback, (XtCallbackProc)grid_activate, &data);
    XtAddCallback(data.grid, XmNheaderClickCallback, (XtCallbackProc)grid_header_clicked, &data);
    
    
    n = 0;
    str = XmStringCreateLocalized("Files");
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, data.dirlist); n++;
    XtSetArg(args[n], XmNleftOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, data.filelist); n++;
    XtSetArg(args[n], XmNbottomOffset, WIDGET_SPACING); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNlabelString, str); n++;
    Widget lsFileLabel = XmCreateLabel(data.listform, "label", args, n);
    XtManageChild(lsFileLabel);
    XmStringFree(str);
    
    Widget focus = NULL;
    switch(data.selectedview) {
        case 0: XtManageChild(scrollw); focus = data.container; break;
        case 1: XtManageChild(data.listform); XtManageChild(data.filelistcontainer); focus = data.filelist; break;
        case 2: XtManageChild(data.listform); XtManageChild(data.gridcontainer); focus = data.grid; break;
    }
    
    if(file->path) {
        char *defDir = ParentPath(file->path);
        filedialog_update_dir(&data, defDir);
        PathBarSetPath(data.pathBar, defDir);
        NEditFree(defDir);
        
        XmTextFieldSetString(data.name, FileName(file->path));
    } else {
        char *defDirStr = GetDefaultDirectoryStr();
        char *defDir = defDirStr ? defDirStr : getenv("HOME");
        
        filedialog_update_dir(&data, defDir);
        PathBarSetPath(data.pathBar, defDir);
    }
    
    //init_container_size(&data);
    
    /* event loop */
    ManageDialogCenteredOnPointer(form);
    
    XmProcessTraversal(focus, XmTRAVERSE_CURRENT);
    
    XtAppContext app = XtWidgetToApplicationContext(dialog);
    while(!data.end && !XtAppGetExitFlag(app)) {
        XEvent event;
        XtAppNextEvent(app, &event);
        XtDispatchEvent(&event);
    }
    
    LastView = data.selectedview;
    
    if(data.selectedPath && !data.selIsDir && data.status == FILEDIALOG_OK) {
        file->path = data.selectedPath;
        data.selectedPath = NULL;
        
        if(file->setenc) {
            int encPos;
            XtVaGetValues(data.encoding, XmNselectedPosition, &encPos, NULL);
            if(type == FILEDIALOG_OPEN) {
                if(encPos > 0) {
                    /* index 0 is the "detect" item which is not a valid 
                       encoding string that can be used later */
                    file->encoding = default_encodings[encPos];
                }
            } else {
                if(currentEncItem) {
                    /* first item is the current encoding that is not 
                     * in default_encodings */
                    if(encPos > 0) {
                        file->encoding = default_encodings[encPos];
                    }
                } else {
                    /* first item is "UTF-8" */
                    file->encoding = default_encodings[encPos+1];
                }
            }
        }
        
        if(type == FILEDIALOG_SAVE) {
            int bomVal = 0;
            int xattrVal = 0;
            int wrapVal = 0;
            XtVaGetValues(data.bom, XmNset, &bomVal, NULL);
            XtVaGetValues(data.xattr, XmNset, &xattrVal, NULL);
            XtVaGetValues(data.wrap, XmNset, &wrapVal, NULL);
            file->writebom = bomVal;
            file->setxattr = xattrVal;
            file->addwrap = wrapVal;
            
            int formatVal = 0;
            XtVaGetValues(data.unixFormat, XmNset, &formatVal, NULL);
            if(formatVal) {
                file->format = UNIX_FILE_FORMAT;
            } else {
                XtVaGetValues(data.dosFormat, XmNset, &formatVal, NULL);
                if(formatVal) {
                    file->format = DOS_FILE_FORMAT;
                } else {
                    XtVaGetValues(data.macFormat, XmNset, &formatVal, NULL);
                    if(formatVal) {
                        file->format = MAC_FILE_FORMAT;
                    }
                }
            }
            
        }
    } else {
        data.status = FILEDIALOG_CANCEL;
    }
   
    XtUnmapWidget(dialog);
    XtDestroyWidget(dialog);
    return data.status;
}
