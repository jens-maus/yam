#ifndef YAM_FIND_H
#define YAM_FIND_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <dos/datetime.h>

#ifndef FORMAT_DEF
#define FORMAT_DEF 4
#endif

// forward declarations
struct BoyerMooreContext;

enum ApplyFilterMode
{
  APPLY_USER=0,
  APPLY_AUTO,
  APPLY_SENT,
  APPLY_REMOTE,
  APPLY_RX_ALL,
  APPLY_RX,
  APPLY_SPAM
};

enum FastSearch
{
  FS_NONE=0,
  FS_FROM,
  FS_TO,
  FS_CC,
  FS_REPLYTO,
  FS_SUBJECT,
  FS_DATE,
  FS_SIZE
};

enum SearchMode
{
  SM_FROM=0,
  SM_TO,
  SM_CC,
  SM_REPLYTO,
  SM_SUBJECT,
  SM_DATE,
  SM_HEADLINE,
  SM_SIZE,
  SM_HEADER,
  SM_BODY,
  SM_WHOLE,
  SM_STATUS,
  SM_SPAM
};

enum CombineMode
{
  CB_ALL=0,
  CB_AT_LEAST_ONE,
  CB_EXACTLY_ONE
};

enum SubSearchMode
{
  SSM_ADDRESS=0,
  SSM_NAME
};

enum Comparison
{
  CP_EQUAL=0,
  CP_NOTEQUAL,
  CP_LOWER,
  CP_GREATER,
  CP_INPUT
};

// lets define all the filter->actions flags and
// define some flag macros for them
#define FA_REDIRECT         (1<<0)
#define FA_FORWARD          (1<<1)
#define FA_REPLY            (1<<2)
#define FA_EXECUTE          (1<<3)
#define FA_PLAYSOUND        (1<<4)
#define FA_MOVE             (1<<5)
#define FA_DELETE           (1<<6)
#define FA_SKIPMSG          (1<<7)
#define FA_STATUSTOMARKED   (1<<8)
#define FA_STATUSTOUNMARKED (1<<9)
#define FA_STATUSTOREAD     (1<<10)
#define FA_STATUSTOUNREAD   (1<<11)
#define FA_STATUSTOSPAM     (1<<12)
#define FA_STATUSTOHAM      (1<<13)
#define FA_TERMINATE        (1<<14)
#define hasRedirectAction(filter)         (isFlagSet((filter)->actions, FA_REDIRECT))
#define hasForwardAction(filter)          (isFlagSet((filter)->actions, FA_FORWARD))
#define hasReplyAction(filter)            (isFlagSet((filter)->actions, FA_REPLY))
#define hasExecuteAction(filter)          (isFlagSet((filter)->actions, FA_EXECUTE))
#define hasPlaySoundAction(filter)        (isFlagSet((filter)->actions, FA_PLAYSOUND))
#define hasMoveAction(filter)             (isFlagSet((filter)->actions, FA_MOVE))
#define hasDeleteAction(filter)           (isFlagSet((filter)->actions, FA_DELETE))
#define hasSkipMsgAction(filter)          (isFlagSet((filter)->actions, FA_SKIPMSG))
#define hasStatusToMarkedAction(filter)   (isFlagSet((filter)->actions, FA_STATUSTOMARKED))
#define hasStatusToUnmarkedAction(filter) (isFlagSet((filter)->actions, FA_STATUSTOUNMARKED))
#define hasStatusToReadAction(filter)     (isFlagSet((filter)->actions, FA_STATUSTOREAD))
#define hasStatusToUnreadAction(filter)   (isFlagSet((filter)->actions, FA_STATUSTOUNREAD))
#define hasStatusToSpamAction(filter)     (isFlagSet((filter)->actions, FA_STATUSTOSPAM))
#define hasStatusToHamAction(filter)      (isFlagSet((filter)->actions, FA_STATUSTOHAM))
#define hasTerminateAction(filter)        (isFlagSet((filter)->actions, FA_TERMINATE))

struct SearchPatternNode
{
  struct MinNode node;                // for storing it into the patternList
  char pattern[(SIZE_PATTERN+4)*2+2]; // for storing the parsed pattern, plus 4 for possible "#?str#?" expansions
};

// flags for a search
#define SEARCHF_CASE_SENSITIVE      (1<<0) // perform a case sensitive seatch
#define SEARCHF_SUBSTRING           (1<<1) // search for a substring instead of a complete string
#define SEARCHF_DOS_PATTERN         (1<<2) // use AmigaDOS pattern matching
#define SEARCHF_SKIP_ENCRYPTED      (1<<3) // skip encrypted mails (i.e. PGP)

struct Search
{
  struct FilterNode  * filter;  // backchain pointer to the filter which this search belongs to
  long                 Size;
  enum SearchMode      Mode;
  int                  PersMode;
  enum Comparison      Compare;
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
  struct MinNode   node;                     // required for placing it into struct Config
  enum CombineMode combine;                  // how are the rules to be met?
  int              actions;                  // actions to execute if conditions are met
  BOOL             isVolatile;               // filter is a volatile filter inserted by YAM
  BOOL             remote;                   // filter is a remote filter
  BOOL             applyToNew;               // apply filter automatically to new mail
  BOOL             applyOnReq;               // apply filter on user request
  BOOL             applyToSent;              // apply filter automatically on sent mail
  char             name[SIZE_NAME];          // user definable filter name
  char             redirectTo[SIZE_ADDRESS]; // redirect action: address to redirect the mail to
  char             forwardTo[SIZE_ADDRESS];  // forward action: address to forward the mail to
  char             replyFile[SIZE_PATHFILE]; // path to a file to use as the reply text
  char             executeCmd[SIZE_COMMAND]; // command string for execute action
  char             playSound[SIZE_PATHFILE]; // path to sound file for sound notification action
  char             moveToName[SIZE_NAME];    // folder name for move mail action
  int              moveToID;                 // folder ID for move mail action
  struct MinList   ruleList;                 // list of all rules that filter evaluates.
};

struct FilterResult
{
  long Checked;
  long Redirected;
  long Forwarded;
  long Replied;
  long Executed;
  long Moved;
  long Deleted;
  long Spam;
};

extern const char mailStatusCycleMap[11];

BOOL FI_PrepareSearch(struct Search *search, const enum SearchMode mode, const int persmode,
                      const enum Comparison compare, const char stat, const char *match, const char *field, const int flags);
BOOL FI_DoSearch(struct Search *search, const struct Mail *mail);
BOOL FI_FilterSingleMail(const struct MinList *filterList, struct Mail *mail, int *matches, struct FilterResult *result);

void FreeSearchData(struct Search *search);
void DeleteRuleNode(struct RuleNode *rule);
void DeleteFilterNode(struct FilterNode *filter);
void DeleteFilterList(struct MinList *filterList);
struct MinList *CloneFilterList(enum ApplyFilterMode mode);
void FreeFilterList(struct MinList *filterList);
BOOL ExecuteFilterAction(const struct FilterNode *filter, struct Mail *mail, struct FilterResult *result);
BOOL CopyFilterData(struct FilterNode *dstFilter, struct FilterNode *srcFilter);
void FreeFilterRuleList(struct FilterNode *filter);
struct FilterNode *CreateNewFilter(const int actions, const int ruleFlags);
void FreeFilterList(struct MinList *filterList);
struct RuleNode *CreateNewRule(struct FilterNode *filter, const int flags);
struct RuleNode *GetFilterRule(struct FilterNode *filter, int pos);
BOOL DoFilterSearch(const struct FilterNode *filter, const struct Mail *mail);
BOOL CompareFilterLists(const struct MinList *fl1, const struct MinList *fl2);
void FilterMails(const struct MailList *mlist, const int mode, struct FilterResult *result);
BOOL FolderIsUsedByFilters(const struct Folder *folder);
void RenameFolderInFilters(const struct Folder *oldFolder, const struct Folder *newFolder);
void RemoveFolderFromFilters(const struct Folder *folder);
BOOL ImportFilter(const char *fileName, const BOOL isVolatile, struct MinList *filterList);
void CheckFilterRules(struct FilterNode *filter);

#endif /* YAM_FIND_H */
