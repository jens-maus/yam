#ifndef YAM_FIND_H
#define YAM_FIND_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <dos/datetime.h>

#ifndef FORMAT_DEF
#define FORMAT_DEF 4
#endif

// forward declarations
struct BoyerMooreContext;

struct FI_GUIData
{
  Object *WI;
  Object *LV_FOLDERS;
  Object *GR_SEARCH;
  Object *LV_MAILS;
  Object *GR_PAGE;
  Object *GA_PROGRESS;
  Object *BT_SEARCH;
  Object *BT_SELECTACTIVE;
  Object *BT_SELECT;
  Object *BT_READ;
};

// find window
struct FI_ClassData
{
  struct FI_GUIData GUI;
  ULONG             Abort;
  BOOL              SearchActive;
  BOOL              ClearOnEnd;
};

enum ApplyFilterMode  { APPLY_USER, APPLY_AUTO, APPLY_SENT, APPLY_REMOTE, APPLY_RX_ALL, APPLY_RX, APPLY_SPAM };
enum FastSearch       { FS_NONE=0, FS_FROM, FS_TO, FS_CC, FS_REPLYTO, FS_SUBJECT, FS_DATE, FS_SIZE };
enum SearchMode       { SM_FROM=0, SM_TO, SM_CC, SM_REPLYTO, SM_SUBJECT, SM_DATE, SM_HEADLINE,
                        SM_SIZE, SM_HEADER, SM_BODY, SM_WHOLE, SM_STATUS, SM_SPAM };
enum CombineMode      { CB_NONE=0, CB_OR, CB_AND, CB_XOR };
enum SubSearchMode    { SSM_ADDRESS=0, SSM_NAME };
enum Comparison       { CP_EQUAL=0, CP_NOTEQUAL, CP_LOWER, CP_GREATER, CP_INPUT };

// lets define all the filter->actions flags and
// define some flag macros for them
#define FA_BOUNCE       (1<<0)
#define FA_FORWARD      (1<<1)
#define FA_REPLY        (1<<2)
#define FA_EXECUTE      (1<<3)
#define FA_PLAYSOUND    (1<<4)
#define FA_MOVE         (1<<5)
#define FA_DELETE       (1<<6)
#define FA_SKIPMSG      (1<<7)
#define hasBounceAction(filter)     (isFlagSet((filter)->actions, FA_BOUNCE))
#define hasForwardAction(filter)    (isFlagSet((filter)->actions, FA_FORWARD))
#define hasReplyAction(filter)      (isFlagSet((filter)->actions, FA_REPLY))
#define hasExecuteAction(filter)    (isFlagSet((filter)->actions, FA_EXECUTE))
#define hasPlaySoundAction(filter)  (isFlagSet((filter)->actions, FA_PLAYSOUND))
#define hasMoveAction(filter)       (isFlagSet((filter)->actions, FA_MOVE))
#define hasDeleteAction(filter)     (isFlagSet((filter)->actions, FA_DELETE))
#define hasSkipMsgAction(filter)    (isFlagSet((filter)->actions, FA_SKIPMSG))

struct SearchPatternNode
{
  struct MinNode node;                // for storing it into the patternList
  char pattern[(SIZE_PATTERN+4)*2+2]; // for storing the parsed pattern, plus 4 for possible "#?str#?" expansions
};

// flags for a search
#define SEARCHF_CASE_SENSITIVE      (1<<0) // perform a case sensitive seatch
#define SEARCHF_SUBSTRING           (1<<1) // search for a substring instead of a complete string
#define SEARCHF_DOS_PATTERN         (1<<2) // use AmigaDOS pattern matching

struct Search
{
  struct FilterNode  * filter;  // backchain pointer to the filter which this search belongs to
  long                 Size;
  enum SearchMode      Mode;
  int                  PersMode;
  int                  Compare;
  char                 Status;  // mail status flags
  enum FastSearch      Fast;
  int                  flags;   // search flags, see above
  char                 Match[SIZE_PATTERN+4];     // the string to search, plus 4 for possible "#?str#?" expansions
  char                 Field[SIZE_DEFAULT];
  struct DateTime      dateTime;
  struct MinList       patternList;               // for storing search patterns, including the embedded singlePattern
  struct BoyerMooreContext *bmContext;
};

// A rule structure which is used to be placed
// into the ruleList of a filter
struct RuleNode
{
  struct MinNode      node;                       // required for placing it into struct FilterNode;
  struct Search     * search;                     // ptr to our search structure or NULL if not ready yet
  enum CombineMode    combine;                    // combine value defining which combine operation is used (i.e. AND/OR/XOR)
  enum SearchMode     searchMode;                 // the destination of the search (i.e. FROM/TO/CC/REPLYTO etc..)
  enum SubSearchMode  subSearchMode;              // the sub mode for the search (i.e. Adress/Name for email adresses etc.)
  enum Comparison     comparison;                 // comparison mode to use for our query (i.e. >, <, <>, IN)
  int                 flags;                      // search flags, see above
  char                matchPattern[SIZE_PATTERN]; // user defined pattern for search/filter
  char                customField[SIZE_DEFAULT];  // user definable string to query some more information
};

// A filter is represented as a single filter node
// containing actions and stuff to apply
struct FilterNode
{
  struct MinNode  node;                     // required for placing it into struct Config
  int             actions;                  // actions to execute if filter/search matches
  BOOL            remote;                   // filter is a remote filter
  BOOL            applyToNew;               // apply filter automatically to new mail
  BOOL            applyOnReq;               // apply filter on user request
  BOOL            applyToSent;              // apply filter automatically on sent mail
  char            name[SIZE_NAME];          // user definable filter name
  char            bounceTo[SIZE_ADDRESS];   // bounce action: address to bounce the mail to
  char            forwardTo[SIZE_ADDRESS];  // forward action: address to forward the mail to
  char            replyFile[SIZE_PATHFILE]; // path to a file to use as the reply text
  char            executeCmd[SIZE_COMMAND]; // command string for execute action
  char            playSound[SIZE_PATHFILE]; // path to sound file for sound notification action
  char            moveTo[SIZE_NAME];        // folder name for move mail action
  struct MinList  ruleList;                 // list of all rules that filter evaluates.
};

// external hooks
extern struct Hook FI_OpenHook;
extern struct Hook ApplyFiltersHook;

extern const char mailStatusCycleMap[11];

BOOL FI_PrepareSearch(struct Search *search, enum SearchMode mode, int persmode,
                      int compar, char stat, const char *match, const char *field, const int flags);
BOOL FI_DoSearch(struct Search *search, const struct Mail *mail);
BOOL FI_FilterSingleMail(const struct MinList *filterList, struct Mail *mail, int *matches);

void FreeSearchData(struct Search *search);
void DeleteRuleNode(struct RuleNode *rule);
void DeleteFilterNode(struct FilterNode *filter);
void DeleteFilterList(struct MinList *filterList);
struct MinList *CloneFilterList(enum ApplyFilterMode mode);
void FreeFilterList(struct MinList *filterList);
BOOL ExecuteFilterAction(const struct FilterNode *filter, struct Mail *mail);
BOOL CopyFilterData(struct FilterNode *dstFilter, struct FilterNode *srcFilter);
void FreeFilterRuleList(struct FilterNode *filter);
struct FilterNode *CreateNewFilter(void);
void FreeFilterList(struct MinList *filterList);
struct RuleNode *CreateNewRule(struct FilterNode *filter, const BOOL dosPattern);
struct RuleNode *GetFilterRule(struct FilterNode *filter, int pos);
BOOL DoFilterSearch(const struct FilterNode *filter, const struct Mail *mail);
BOOL CompareFilterLists(const struct MinList *fl1, const struct MinList *fl2);
void FilterMails(struct Folder *folder, struct MailList *mlist, int mode);
BOOL FolderIsUsedByFilters(const char *folder);
void RenameFolderInFilters(const char *oldFolder, const char *newFolder);
void RemoveFolderFromFilters(const char *folder);

#endif /* YAM_FIND_H */
