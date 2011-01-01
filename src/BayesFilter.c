/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_read.h"
#include "YAM_addressbook.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "extrasrc.h"

#include "BayesFilter.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MethodStack.h"

#include "Debug.h"

#define BAYES_TOKEN_DELIMITERS  " \t\n\r\f.,"
#define BAYES_MIN_TOKEN_LENGTH  3
#define BAYES_MAX_TOKEN_LENGTH  12

#define SPAMDATAFILE            ".spamdata"

// some compilers (vbcc) don't define this, so lets do it ourself
#ifndef M_LN2
#define M_LN2                   0.69314718055994530942
#endif

/*** Structure definitions ***/
struct Token
{
  struct HashEntryHeader hash;
  const char *word;
  ULONG length;
  ULONG count;
  double probability;
  double distance;
};

struct TokenEnumeration
{
  ULONG entrySize;
  ULONG entryCount;
  ULONG entryOffset;
  char *entryAddr;
  char *entryLimit;
};

// the magic data we expect upon reading the data file
static const unsigned char magicCookie[] = { '\xFE', '\xED', '\xFA', '\xCE' };

/*** Static functions ***/
/// tokenizerInit
// initalize a token table
static BOOL tokenizerInit(struct Tokenizer *t)
{
  BOOL result;

  ENTER();

  result = HashTableInit(&t->tokenTable, HashTableGetDefaultStringOps(), NULL, sizeof(struct Token), 4096);

  RETURN(result);
  return result;
}

///
/// tokenizerCleanup
// cleanup a token table
static void tokenizerCleanup(struct Tokenizer *t)
{
  ENTER();

  HashTableCleanup(&t->tokenTable);

  LEAVE();
}

///
/// tokenizerClearTokens
// reinitialize a token table
static BOOL tokenizerClearTokens(struct Tokenizer *t)
{
  BOOL ok = TRUE;

  ENTER();

  if(t->tokenTable.entryStore != NULL)
  {
    tokenizerCleanup(t);
    ok = tokenizerInit(t);
  }

  RETURN(ok);
  return ok;
}

///
/// optimizeToken
// optimize a token table
static enum HashTableOperator optimizeToken(UNUSED struct HashTable *table,
                                            struct HashEntryHeader *entry,
                                            UNUSED ULONG number,
                                            void *arg)
{
  enum HashTableOperator result;
  struct Token *token = (struct Token *)entry;

  ENTER();

  // Check whether the token's count value is less or equal to the
  // threshold. If yes, then this token should be removed.
  if(token->count <= (ULONG)arg)
    result = htoNext|htoRemove;
  else
    result = htoNext;

  RETURN(result);
  return result;
}

///
/// tokenizerOptimizeTokens
// Optimize a token table and return the number of removed words
static ULONG tokenizerOptimizeTokens(struct Tokenizer *t,
                                     const ULONG maxCount)
{
  ULONG num;

  ENTER();

  num = HashTableEnumerate(&t->tokenTable, optimizeToken, (void *)maxCount);

  RETURN(num);
  return num;
}

///
/// tokenizerGet
// look up a word in the token table
static struct Token *tokenizerGet(struct Tokenizer *t,
                                  const char *word)
{
  struct HashEntryHeader *entry;

  ENTER();

  entry = HashTableOperate(&t->tokenTable, word, htoLookup);
  if(HASH_ENTRY_IS_FREE(entry))
  {
    // we didn't find the entry we were looking for
    entry = NULL;
  }

  RETURN(entry);
  return (struct Token *)entry;
}

///
/// tokenizerAdd
// add a word to the token table with an arbitrary prefix (maybe NULL) and count
static struct Token *tokenizerAdd(struct Tokenizer *t,
                                  const char *word,
                                  const char *prefix,
                                  const ULONG count)
{
  struct Token *token = NULL;
  ULONG len;
  char *tmpWord;

  ENTER();

  len = strlen(word) + 1;
  if(prefix != NULL)
    len += strlen(prefix) + 1;

  if((tmpWord = (STRPTR)malloc(len)) != NULL)
  {
    if(prefix != NULL)
      snprintf(tmpWord, len, "%s:%s", prefix, word);
    else
      strlcpy(tmpWord, word, len);

    if((token = (struct Token *)HashTableOperate(&t->tokenTable, tmpWord, htoAdd)) != NULL)
    {
      if(token->word == NULL)
      {
        token->word = tmpWord;
        token->length = len-1;
        token->count = count;
        token->probability = 0.0;
        // make sure this one isn't free()'d
        tmpWord = NULL;
      }
      else
        token->count += count;
    }

    free(tmpWord);
  }

  RETURN(token);
  return token;
}

///
/// tokenizerRemove
// remove <count> occurences of word from the token table
static void tokenizerRemove(struct Tokenizer *t,
                            const char *word,
                            const ULONG count)
{
  struct Token *token;

  ENTER();

  if((token = tokenizerGet(t, word)) != NULL)
  {
    if(token->count >= count)
    {
      token->count -= count;

      if(token->count == 0)
        HashTableRawRemove(&t->tokenTable, (struct HashEntryHeader *)token);
    }
  }

  LEAVE();
}

///
/// isDecimalNumber
// check if <word> is a decimal number
static BOOL isDecimalNumber(const char *word)
{
  BOOL isDecimal = TRUE;
  const char *p = word;
  char c;

  ENTER();

  if(*p == '-')
    p++;

  while((c = *p++) != '\0')
  {
    if(!isdigit((unsigned char)c))
    {
      isDecimal = FALSE;
      break;
    }
  }

  RETURN(isDecimal);
  return isDecimal;
}

///
/// isASCII
// check if <word> is an ASCII word
static BOOL isASCII(const char *word)
{
  BOOL isAsc = TRUE;
  const unsigned char *p = (const unsigned char *)word;
  unsigned char c;

  ENTER();

  while((c = *p++) != '\0')
  {
    if(c > 127)
    {
      isAsc = FALSE;
      break;
    }
  }

  RETURN(isAsc);
  return isAsc;
}

///
/// tokenizerAddTokenForHeader
// add a token for a mail header line, prefix may be NULL
// the concatenated string can be chosen to be tokenized or be treated as one word
static void tokenizerAddTokenForHeader(struct Tokenizer *t,
                                       const char *prefix,
                                       char *value,
                                       const BOOL tokenizeValue)
{
  ENTER();

  if(value != NULL && strlen(value) > 0)
  {
    ToLowerCase(value);
    if(tokenizeValue == FALSE)
      tokenizerAdd(t, value, prefix, 1);
    else
    {
      char *word = value;
      char *next;

      do
      {
        // split the line into separate words
        if((next = strpbrk(word, BAYES_TOKEN_DELIMITERS)) != NULL)
          *next++ = '\0';

        // add all non-empty and non-number words to the tokenizer
        if(word[0] != '\0' && isDecimalNumber(word) == FALSE && isASCII(word) == TRUE)
          tokenizerAdd(t, word, prefix, 1);

        word = next;
      }
      while(word != NULL);
    }
  }

  LEAVE();
}

///
/// tokenizerTokenizeAttachment
// tokenize a mail attachment
static void tokenizerTokenizeAttachment(struct Tokenizer *t,
                                        const char *contentType,
                                        const char *fileName)
{
  char *tmpContentType;
  char *tmpFileName;

  ENTER();

  if((tmpContentType = strdup(contentType)) != NULL)
  {
    if((tmpFileName = strdup(fileName)) != NULL)
    {
      tokenizerAddTokenForHeader(t, "attachment/filename", tmpFileName, FALSE);
      tokenizerAddTokenForHeader(t, "attachment/content-type", tmpContentType, FALSE);

      free(tmpFileName);
    }

    free(tmpContentType);
  }

  LEAVE();
}

///
/// tokenizerTokenizeHeader
// tokenize all headers of a mail
static void tokenizerTokenizeHeaders(struct Tokenizer *t,
                                     const struct Part *part)
{
  struct Node *curNode;
  char *contentType;
  char *charSet;

  ENTER();

  contentType = (part->ContentType != NULL) ? strdup(part->ContentType) : NULL;
  charSet = (part->CParCSet != NULL) ? strdup(part->CParCSet) : NULL;

  IterateList(part->headerList, curNode)
  {
    struct HeaderNode *hdr = (struct HeaderNode *)curNode;
    char *name;

    if((name = strdup(hdr->name)) != NULL)
    {
      char *content;

      if((content = strdup(hdr->content)) != NULL)
      {
        ToLowerCase(name);
        ToLowerCase(content);

        SHOWSTRING(DBF_SPAM, name);
        SHOWSTRING(DBF_SPAM, content);

        switch(name[0])
        {
          case 'c':
          {
            if(strcmp(name, "content-type") == 0)
            {
              tokenizerAddTokenForHeader(t, "content-type", contentType, FALSE);
              tokenizerAddTokenForHeader(t, "charset", charSet, FALSE);
            }
          }
          break;

          case 'r':
          {
            if(strcmp(name, "received") == 0 &&
               strstr(content, "may be forged") != NULL)
            {
              char tmpForged[16];

              // copy the constant string to a variable which may be modified in tokenizerAddTokenForHeader(),
              // otherwise we risk crashes on OS4 because we would modify a read-only string
              strlcpy(tmpForged, "may be forged", sizeof(tmpForged));
              tokenizerAddTokenForHeader(t, name, tmpForged, FALSE);
            }

            // leave out reply-to
          }
          break;

          case 's':
          {
            // we want to tokenize the subject
            if(strcmp(name, "subject") == 0)
              tokenizerAddTokenForHeader(t, name, content, TRUE);

            // leave out sender field, too strong of an indicator
          }
          break;

          case 'u':
          case 'x':
          {
            // X-Mailer/User-Agent works best if it is untokenized
            // just fold the case and any leading/trailing white space
            tokenizerAddTokenForHeader(t, name, content, FALSE);
          }
          break;

          default:
          {
            tokenizerAddTokenForHeader(t, name, content, FALSE);
          }
          break;
        }

        free(content);
      }

      free(name);
    }
  }

  free(contentType);
  free(charSet);

  LEAVE();
}

///
/// countChars
// count occurences of <c> in string <str>
static ULONG countChars(const char *str, const char c)
{
  ULONG count = 0;
  char cc;

  ENTER();

  while((cc = *str++) != '\0')
  {
    if(cc == c)
      count++;
  }

  RETURN(count);
  return count;
}

///
/// tokenizerTokenizeASCIIWord
// tokenize an ASCII word
static void tokenizerTokenizeASCIIWord(struct Tokenizer *t,
                                       char *word)
{
  size_t length;

  ENTER();

  ToLowerCase(word);
  length = strlen(word);

  // if the word fits in our length restrictions then we add it
  if(length >= BAYES_MIN_TOKEN_LENGTH && length <= BAYES_MAX_TOKEN_LENGTH)
    tokenizerAdd(t, word, NULL, 1);
  else
  {
    BOOL skipped = TRUE;

    // don't skip over the word if it looks like an email address
    if(length > BAYES_MAX_TOKEN_LENGTH)
    {
      if(length < 40 && strchr(word, '.') != NULL && countChars(word, '@') == 1)
      {
        struct Person pe;

        // split the john@foo.com into john and foo.com, treat them as separate tokens
        ExtractAddress(word, &pe);

        if(pe.Address[0] != '\0' && pe.RealName[0] != '\0')
        {
          SHOWSTRING(DBF_SPAM, pe.Address);
          SHOWSTRING(DBF_SPAM, pe.RealName);

          tokenizerAdd(t, pe.Address, "email-addr", 1);
          tokenizerAdd(t, pe.RealName, "email-name", 1);
          skipped = FALSE;
        }
      }
    }

    // there is value in generating a token indicating the number of characters we are skipping
    // we'll round to the nearest of 10
    if(skipped == TRUE)
    {
      char buffer[40];

      snprintf(buffer, sizeof(buffer), "%c %d", word[0], (length / 10) * 10);
      tokenizerAdd(t, buffer, "skip", 1);
    }
  }

  LEAVE();
}

///
/// tokenizeTokenize
// tokenize an arbitrary text
static void tokenizerTokenize(struct Tokenizer *t,
                              char *text)
{
  char *word = text;
  char *next;

  ENTER();

  do
  {
    if((next = strpbrk(word, BAYES_TOKEN_DELIMITERS)) != NULL)
      *next++ = '\0';

    if(word[0] != '\0' && isDecimalNumber(word) == FALSE)
    {
      if(isASCII(word) == TRUE)
        tokenizerTokenizeASCIIWord(t, word);
      else
        tokenizerAdd(t, word, NULL, 1);
    }

    word = next;
  }
  while(word != NULL);

  LEAVE();
}

///
/// tokenEnumerationInit
// initialize a token enumeration
static void tokenEnumerationInit(struct TokenEnumeration *te,
                                 const struct Tokenizer *t)
{
  ENTER();

  te->entrySize = t->tokenTable.entrySize;
  te->entryCount = t->tokenTable.entryCount;
  te->entryOffset = 0;
  te->entryAddr = t->tokenTable.entryStore;
  te->entryLimit = te->entryAddr + HASH_TABLE_SIZE(&t->tokenTable) * te->entrySize;

  LEAVE();
}

///
/// tokenEnumerationNext
// advance one step in the enumeration
static struct Token *tokenEnumerationNext(struct TokenEnumeration *te)
{
  struct Token *token = NULL;

  ENTER();

  if(te->entryOffset < te->entryCount)
  {
    ULONG entrySize = te->entrySize;
    char *entryAddr = te->entryAddr;
    char *entryLimit = te->entryLimit;

    while(entryAddr < entryLimit)
    {
      struct HashEntryHeader *entry = (struct HashEntryHeader *)entryAddr;

      entryAddr += entrySize;
      if(HASH_ENTRY_IS_LIVE(entry))
      {
        token = (struct Token *)entry;
        te->entryOffset++;
        break;
      }
    }

    te->entryAddr = entryAddr;
  }

  RETURN(token);
  return token;
}

///
/// tokenizerCopyTokens
// build a copy of the token table
static struct Token *tokenizerCopyTokens(const struct Tokenizer *t)
{
  struct Token *tokens = NULL;
  ULONG count = t->tokenTable.entryCount;

  ENTER();

  if(count > 0)
  {
    if((tokens = (struct Token *)malloc(count * sizeof(*tokens))) != NULL)
    {
      struct TokenEnumeration te;
      struct Token *tp = tokens, *token;

      tokenEnumerationInit(&te, t);
      while((token = tokenEnumerationNext(&te)) != NULL)
        *tp++ = *token;
    }
  }

  RETURN(tokens);
  return tokens;
}

///
/// tokenizerForgetTokens
// remove all words of the enumeration from the token table
static void tokenizerForgetTokens(struct Tokenizer *t,
                                  struct TokenEnumeration *te)
{
  struct Token *token;

  ENTER();

  // if we are forgetting the tokens for a message, should only substract 1 from the occurence
  // count for that token in the training set, because we assume we only bumped the training
  // set count once per message containing the token
  while((token = tokenEnumerationNext(te)) != NULL)
    tokenizerRemove(t, token->word, 1);

  LEAVE();
}

///
/// tokenizerRememberTokens
// put all words of the enumeration back into the token table
static void tokenizerRememberTokens(struct Tokenizer *t,
                                    struct TokenEnumeration *te)
{
  struct Token *token;

  ENTER();

  while((token = tokenEnumerationNext(te)) != NULL)
    tokenizerAdd(t, token->word, NULL, 1);

  LEAVE();
}

///
/// tokenAnalyzerInit
// initialize the analyzer
static BOOL tokenAnalyzerInit(void)
{
  BOOL result = FALSE;

  ENTER();

  // initialize the counters
  G->spamFilter.goodCount = 0;
  G->spamFilter.badCount = 0;
  G->spamFilter.numDirtyingMessages = 0;

  memset(&G->spamFilter.lockSema, 0, sizeof(G->spamFilter.lockSema));
  InitSemaphore(&G->spamFilter.lockSema);

  if(tokenizerInit(&G->spamFilter.goodTokens) == TRUE && tokenizerInit(&G->spamFilter.badTokens) == TRUE)
    result = TRUE;

  RETURN(result);
  return result;
}

///
/// tokenAnalyzerCleanup
// clean up the analyzer
static void tokenAnalyzerCleanup(void)
{
  ENTER();

  ObtainSemaphore(&G->spamFilter.lockSema);

  tokenizerCleanup(&G->spamFilter.goodTokens);
  tokenizerCleanup(&G->spamFilter.badTokens);

  ReleaseSemaphore(&G->spamFilter.lockSema);

  LEAVE();
}

///
/// writeTokens
// write the tokens of a token table to a stream
static BOOL writeTokens(FILE *stream,
                        const struct Tokenizer *t)
{
  ULONG tokenCount = t->tokenTable.entryCount;

  ENTER();

  if(WriteUInt32(stream, tokenCount) != 1)
  {
    RETURN(FALSE);
    return FALSE;
  }

  if(tokenCount > 0)
  {
    struct TokenEnumeration te;
    ULONG i;

    tokenEnumerationInit(&te, t);
    for(i = 0; i < tokenCount; i++)
    {
      struct Token *token = tokenEnumerationNext(&te);
      ULONG length = token->length;

      if(WriteUInt32(stream, token->count) != 1)
        break;

      if(WriteUInt32(stream, length) != 1)
        break;

      if(fwrite(token->word, length, 1, stream) != 1)
        break;
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// readTokens
// read tokens from a stream into the token table
static BOOL readTokens(FILE *stream,
                       struct Tokenizer *t,
                       const LONG fileSize)
{
  ULONG tokenCount;
  LONG filePos;
  ULONG bufferSize = 4096;
  char *buffer;

  ENTER();

  if(ReadUInt32(stream, &tokenCount) != 1)
  {
    RETURN(FALSE);
    return FALSE;
  }
  if((filePos = ftell(stream)) < 0)
  {
    RETURN(FALSE);
    return FALSE;
  }

  if((buffer = malloc(bufferSize)) != NULL)
  {
    ULONG i;

    for(i = 0; i < tokenCount; i++)
    {
      ULONG count;
      ULONG size;

      if(ReadUInt32(stream, &count) != 1)
        break;

      if(ReadUInt32(stream, &size) != 1)
        break;

      filePos += 8;

      if(size >= bufferSize)
      {
        free(buffer);

        if((LONG)(filePos + size) > fileSize)
        {
          RETURN(FALSE);
          return FALSE;
        }

        while(size >= bufferSize)
        {
          bufferSize *= 2;
          if(bufferSize == 0)
          {
            // overrun
            RETURN(FALSE);
            return FALSE;
          }
        }

        if((buffer = malloc(bufferSize)) == NULL)
        {
          RETURN(FALSE);
          return FALSE;
        }
      }

      if(fread(buffer, size, 1, stream) != 1)
        break;

      filePos += size;

      buffer[size] = '\0';

      tokenizerAdd(t, buffer, NULL, count);
    }

    free(buffer);
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// tokenAnalyzerResetTrainingData
// reset the training data. The makes the spam filter stupid again
static void tokenAnalyzerResetTrainingData(void)
{
  char fname[SIZE_PATHFILE];

  ENTER();

  ObtainSemaphore(&G->spamFilter.lockSema);

  if(G->spamFilter.goodCount != 0 || G->spamFilter.goodTokens.tokenTable.entryCount != 0)
  {
    tokenizerClearTokens(&G->spamFilter.goodTokens);
    G->spamFilter.goodCount = 0;
  }

  if(G->spamFilter.badCount != 0 || G->spamFilter.badTokens.tokenTable.entryCount != 0)
  {
    tokenizerClearTokens(&G->spamFilter.badTokens);
    G->spamFilter.badCount = 0;
  }

  // prepare the filename for analysis
  AddPath(fname, G->MA_MailDir, SPAMDATAFILE, sizeof(fname));

  if(FileExists(fname) == TRUE)
    DeleteFile(fname);

  ReleaseSemaphore(&G->spamFilter.lockSema);

  LEAVE();
}

///
/// tokenAnalyzerOptimizeTrainingData
// Optimize the training data by filtering out words which occured only once so far
static void tokenAnalyzerOptimizeTrainingData(void)
{
  ENTER();

  ObtainSemaphore(&G->spamFilter.lockSema);

  if(G->spamFilter.goodTokens.tokenTable.entryCount != 0)
    G->spamFilter.numDirtyingMessages += tokenizerOptimizeTokens(&G->spamFilter.goodTokens, 1);

  if(G->spamFilter.badTokens.tokenTable.entryCount != 0)
    G->spamFilter.numDirtyingMessages += tokenizerOptimizeTokens(&G->spamFilter.badTokens, 1);

  ReleaseSemaphore(&G->spamFilter.lockSema);

  LEAVE();
}

///
/// tokenAnalyzerWriteTraningData
// write the accumulated training data to disk
static void tokenAnalyzerWriteTrainingData(void)
{
  char fname[SIZE_PATHFILE];
  FILE *stream;

  ENTER();

  // prepare the filename for saving
  AddPath(fname, G->MA_MailDir, SPAMDATAFILE, sizeof(fname));

  // open the .spamdata file for binary write
  if((stream = fopen(fname, "wb")) != NULL)
  {
    setvbuf(stream, NULL, _IOFBF, SIZE_FILEBUF);

    if(fwrite(magicCookie, sizeof(magicCookie), 1, stream) == 1 &&
       WriteUInt32(stream, G->spamFilter.goodCount) == 1 &&
       WriteUInt32(stream, G->spamFilter.badCount) == 1 &&
       writeTokens(stream, &G->spamFilter.goodTokens) == 1 &&
       writeTokens(stream, &G->spamFilter.badTokens) == 1)
    {
      // everything is ok
      fclose(stream);
      G->spamFilter.numDirtyingMessages = 0;
    }
    else
    {
      // anything went wrong, so delete the training data file
      fclose(stream);

      DeleteFile(fname);
    }
  }

  LEAVE();
}

///
/// tokenAnalyzerReadTrainingData
// read the training data from disk
static void tokenAnalyzerReadTrainingData(void)
{
  char fname[SIZE_PATHFILE];
  LONG fileSize;

  ENTER();

  // prepare the filename for loading
  AddPath(fname, G->MA_MailDir, SPAMDATAFILE, sizeof(fname));

  if(ObtainFileInfo(fname, FI_SIZE, &fileSize) == TRUE && fileSize > 0)
  {
    FILE *stream;

    // open the .spamdata file for binary read
    if((stream = fopen(fname, "rb")) != NULL)
    {
      unsigned char cookie[4];
      BOOL success = FALSE;

      setvbuf(stream, NULL, _IOFBF, SIZE_FILEBUF);

      fread(cookie, sizeof(cookie), 1, stream);

      if(memcmp(cookie, magicCookie, sizeof(cookie)) == 0)
      {
        if(ReadUInt32(stream, &G->spamFilter.goodCount) == 1 &&
           ReadUInt32(stream, &G->spamFilter.badCount) == 1)
        {
          SHOWVALUE(DBF_SPAM, G->spamFilter.goodCount);
          SHOWVALUE(DBF_SPAM, G->spamFilter.badCount);

          if(readTokens(stream, &G->spamFilter.goodTokens, fileSize) == TRUE &&
             readTokens(stream, &G->spamFilter.badTokens, fileSize) == TRUE)
          {
            success = TRUE;
          }
        }
      }

      fclose(stream);

      if(success == FALSE)
      {
        // something went wrong during the read process, reset everything
        tokenAnalyzerResetTrainingData();
      }
    }
  }

  LEAVE();
}

///
/// tokenAnalyzerSetClassification
// set a new classification for a token table, substract the data from the old class
// and add them to the new class, if possible
static void tokenAnalyzerSetClassification(const struct Tokenizer *t,
                                           const enum BayesClassification oldClass,
                                           const enum BayesClassification newClass)
{
  struct TokenEnumeration te;

  ENTER();

  tokenEnumerationInit(&te, t);

  ObtainSemaphore(&G->spamFilter.lockSema);

  if(oldClass != newClass)
  {
    switch(oldClass)
    {
      case BC_SPAM:
      {
        // remove tokens from spam corpus
        if(G->spamFilter.badCount > 0)
        {
          G->spamFilter.badCount--;
          G->spamFilter.numDirtyingMessages++;
          tokenizerForgetTokens(&G->spamFilter.badTokens, &te);
        }
      }
      break;

      case BC_HAM:
      {
        // remove tokens from ham corpus
        if(G->spamFilter.goodCount > 0)
        {
          G->spamFilter.goodCount--;
          G->spamFilter.numDirtyingMessages++;
          tokenizerForgetTokens(&G->spamFilter.goodTokens, &te);
        }
      }
      break;

      case BC_OTHER:
        // nothing
      break;
    }

    switch(newClass)
    {
      case BC_SPAM:
      {
        // put tokens into spam corpus
        G->spamFilter.badCount++;
        G->spamFilter.numDirtyingMessages++;
        tokenizerRememberTokens(&G->spamFilter.badTokens, &te);
      }
      break;

      case BC_HAM:
      {
        // put tokens into ham corpus
        G->spamFilter.goodCount++;
        G->spamFilter.numDirtyingMessages++;
        tokenizerRememberTokens(&G->spamFilter.goodTokens, &te);
      }
      break;

      case BC_OTHER:
        // nothing
      break;
    }
  }

  ReleaseSemaphore(&G->spamFilter.lockSema);

  LEAVE();
}

///
/// compareTokens
// compare to tokens to sort them
static int compareTokens(const void *p1,
                         const void *p2)
{
  struct Token *t1 = (struct Token *)p1;
  struct Token *t2 = (struct Token *)p2;
  double delta;
  int cmp;

  ENTER();

  delta = t1->distance - t2->distance;
  cmp = ((delta == 0.0) ? 0 : ((delta > 0.0) ? 1 : -1));

  RETURN(cmp);
  return cmp;
}

///

static const double C_1 = 1.0 / 12.0;
static const double C_2 = -1.0 / 360.0;
static const double C_3 = 1.0 / 1260.0;
static const double C_4 = -1.0 / 1680.0;
static const double C_5 = 1.0 / 1188.0;
static const double C_6 = -691.0 / 360360.0;
static const double C_7 = 1.0 / 156.0;
static const double C_8 = -3617.0 / 122400.0;
static const double C_9 = 43867.0 / 244188.0;
static const double C_10 = -174611.0 / 125400.0;
static const double C_11 = 77683.0 / 5796.0;

/// lngamma_asymp
// truncated asymptotic series in 1/z
INLINE double lngamma_asymp(double z)
{
  double w, w2, sum;

  w = 1.0 / z;
  w2 = w * w;
  sum = w * (w2 * (w2 * (w2 * (w2 * (w2 * (w2 * (w2 * (w2 * (w2
        * (C_11 * w2 + C_10) + C_9) + C_8) + C_7) + C_6)
        + C_5) + C_4) + C_3) + C_2) + C_1);

  return sum;
}

///

struct fact_table_s
{
  double fact;
  double lnfact;
};

// for speed and accuracy
static const struct fact_table_s FactTable[] =
{
  { 1.000000000000000, 0.0000000000000000000000e+00 },
  { 1.000000000000000, 0.0000000000000000000000e+00 },
  { 2.000000000000000, 6.9314718055994530942869e-01 },
  { 6.000000000000000, 1.7917594692280550007892e+00 },
  { 24.00000000000000, 3.1780538303479456197550e+00 },
  { 120.0000000000000, 4.7874917427820459941458e+00 },
  { 720.0000000000000, 6.5792512120101009952602e+00 },
  { 5040.000000000000, 8.5251613610654142999881e+00 },
  { 40320.00000000000, 1.0604602902745250228925e+01 },
  { 362880.0000000000, 1.2801827480081469610995e+01 },
  { 3628800.000000000, 1.5104412573075515295248e+01 },
  { 39916800.00000000, 1.7502307845873885839769e+01 },
  { 479001600.0000000, 1.9987214495661886149228e+01 },
  { 6227020800.000000, 2.2552163853123422886104e+01 },
  { 87178291200.00000, 2.5191221182738681499610e+01 },
  { 1307674368000.000, 2.7899271383840891566988e+01 },
  { 20922789888000.00, 3.0671860106080672803835e+01 },
  { 355687428096000.0, 3.3505073450136888885825e+01 },
  { 6402373705728000., 3.6395445208033053576674e+01 }
};

#define FactTableLength (int)(sizeof(FactTable)/sizeof(FactTable[0]))

// for speed
static const double ln_2pi_2 = 0.918938533204672741803; // log(2*PI)/2

/// nsLnGamma
// A simple lgamma function, not very robust.
//
// Valid for z_in > 0 ONLY.
//
// For z_in > 8 precision is quite good, relative errors < 1e-14 and
// usually better. For z_in < 8 relative errors increase but are usually
// < 1e-10. In two small regions, 1 +/- .001 and 2 +/- .001 errors
// increase quickly.
static double nsLnGamma (double z_in, int *gsign)
{
  double scale, z, sum, result;
  int zi = (int) z_in;
  *gsign = 1;

  if(z_in == (double) zi)
  {
    if(0 < zi && zi <= FactTableLength)
      return FactTable[zi - 1].lnfact;    // gamma(z) = (z-1)!
  }

  for(scale = 1.0, z = z_in; z < 8.0; ++z)
    scale *= z;

  sum = lngamma_asymp (z);
  result = (z - 0.5) * log (z) - z + ln_2pi_2 - log (scale);
  result += sum;

  return result;
}

///
/// lnPQfactor
// log( e^(-x)*x^a/Gamma(a) )
INLINE double lnPQfactor (double a, double x)
{
  int gsign;                // ignored because a > 0
  return a * log (x) - x - nsLnGamma (a, &gsign);
}

///
/// Pseries
static double Pseries (double a, double x, int *error)
{
  double sum, term;
  const double eps = 2.0 * DBL_EPSILON;
  const int imax = 5000;
  int i;

  sum = term = 1.0 / a;
  for(i = 1; i < imax; ++i)
  {
      term *= x / (a + i);
      sum += term;
      if(fabs (term) < eps * fabs (sum))
          break;
  }

  if(i >= imax)
      *error = 1;

  return sum;
}

///
/// Qcontfrac
//
static double Qcontfrac (double a, double x, int *error)
{
  double result, DD, CC, e, f, term;
  const double eps = 2.0 * DBL_EPSILON;
  const double small = DBL_EPSILON * DBL_EPSILON * DBL_EPSILON * DBL_EPSILON;
  const int imax = 5000;
  int i;

  // modified Lentz method
  f = x - a + 1.0;
  if(fabs (f) < small)
    f = small;
  CC = f + 1.0 / small;
  DD = 1.0 / f;
  result = DD;
  for(i = 1; i < imax; ++i)
  {
    e = i * (a - i);
    f += 2.0;
    DD = f + e * DD;
    if(fabs (DD) < small)
      DD = small;
    DD = 1.0 / DD;
    CC = f + e / CC;
    if(fabs (CC) < small)
      CC = small;
    term = CC * DD;
    result *= term;
    if(fabs (term - 1.0) < eps)
      break;
  }

  if(i >= imax)
    *error = 1;

  return result;
}

///
/// incompleteGammaP
//
double incompleteGammaP( double a, double x, int *error )
{
  double result, dom, ldom;

  //  domain errors. the return values are meaningless but have
  //  to return something.
  *error = -1;
  if(a <= 0.0)
    return 1.0;

  if(x < 0.0)
    return 0.0;

  *error = 0;
  if(x == 0.0)
    return 0.0;

  ldom = lnPQfactor (a, x);
  dom = exp(ldom);

  // might need to adjust the crossover point
  if(a <= 0.5)
  {
    if(x < a + 1.0)
      result = dom * Pseries (a, x, error);
    else
      result = 1.0 - dom * Qcontfrac (a, x, error);
  }
  else
  {
    if(x < a)
      result = dom * Pseries (a, x, error);
    else
      result = 1.0 - dom * Qcontfrac (a, x, error);
  }

  // not clear if this can ever happen
  if(result > 1.0)
    result = 1.0;

  if(result < 0.0)
    result = 0.0;

  return result;
}

///
/// chi2P
//
INLINE double chi2P(double chi2, double nu, int *error)
{
  if(chi2 < 0.0 || nu < 0.0)
  {
    *error = -1;
    return 0.0;
  }

  return incompleteGammaP(nu / 2.0, chi2 / 2.0, error);
}

///
/// tokenAnalyzerClassifyMessage
// classify a mail based upon the information gathered so far
static BOOL tokenAnalyzerClassifyMessage(const struct Tokenizer *t,
                                         const struct Mail *mail)
{
  BOOL isSpam;
  BOOL isInWhiteList;

  ENTER();

  D(DBF_SPAM, "analyzing mail from '%s' with subject '%s'", mail->From.Address, mail->Subject);

  if(C->SpamAddressBookIsWhiteList == TRUE)
  {
    // try to find the sender's address in the address book
    isInWhiteList = (AB_FindEntry(mail->From.Address, ABF_RX_EMAIL, NULL) != 0);
  }
  else
  {
    // address book should not be considered as white list
    isInWhiteList = FALSE;
  }

  if(isInWhiteList == FALSE)
  {
    // the mail's sender was not found in the address book, so let's analyze the mail contents
    struct Token *tokens = NULL;

    ObtainSemaphoreShared(&G->spamFilter.lockSema);

    SHOWVALUE(DBF_SPAM, G->spamFilter.goodCount);
    SHOWVALUE(DBF_SPAM, G->spamFilter.badCount);

    if((tokens = tokenizerCopyTokens(t)) != NULL)
    {
      double nGood = G->spamFilter.goodCount;

      if(nGood != 0 || G->spamFilter.goodTokens.tokenTable.entryCount != 0)
      {
        double nBad = G->spamFilter.badCount;

        if(nBad != 0 || G->spamFilter.badTokens.tokenTable.entryCount != 0)
        {
          ULONG i;
          ULONG goodClues = 0;
          ULONG count = t->tokenTable.entryCount;
          ULONG first;
          ULONG last;
          ULONG Hexp;
          ULONG Sexp;
          double prob;
          double H;
          double S;

          for(i = 0; i < count; i++)
          {
            struct Token *token = &tokens[i];
            const char *word = token->word;
            struct Token *_t;
            double hamCount;
            double spamCount;
            double denom;
            double tokenProb;
            double n;
            double distance;

            _t = tokenizerGet(&G->spamFilter.goodTokens, word);
            hamCount = (_t != NULL) ? _t->count : 0;
            _t = tokenizerGet(&G->spamFilter.badTokens, word);
            spamCount = (_t != NULL) ? _t->count : 0;

            denom = hamCount * nBad + spamCount * nGood;
            // avoid division by zero error
            if(denom == 0.0)
              denom = nBad + nGood;

            tokenProb = (spamCount * nGood) / denom;
            n = hamCount + spamCount;
            tokenProb = (0.225 + n * tokenProb) / (0.45 + n);
            distance = fabs(tokenProb - 0.5);

            if(distance >= 0.1)
            {
              D(DBF_SPAM, "probability for token '%s' is %.2f", word, tokenProb);
              goodClues++;
              token->distance = distance;
              token->probability = tokenProb;
            }
            else
            {
              // ignore this clue
              token->distance = -1.0;
            }
          }

          D(DBF_SPAM, "found %ld good clues in the first scan", goodClues);

          // sort array of token distances
          qsort(tokens, count, sizeof(*tokens), compareTokens);

          first = (goodClues > 150) ? count - 150 : 0;
          last = count;
          H = 1.0;
          S = 1.0;
          Hexp = 0;
          Sexp = 0;

          // reset this counter, so we can check later the real number of *really* good clues
          goodClues = 0;

          for(i = first; i < last; i++)
          {
            if(tokens[i].distance != -1.0)
            {
              double value;
              int e;

              goodClues++;
              value = tokens[i].probability;
              S *= (1.0 - value);
              H *= value;

              // if the probability values become too small we rescale them
              if(S < 1e-200)
              {
                S = frexp(S, &e);
                Sexp += e;
              }
              if(H < 1e-200)
              {
                H = frexp(H, &e);
                Hexp += e;
              }
            }
          }

          S = log(S) + Sexp * M_LN2;
          H = log(H) + Hexp * M_LN2;

          D(DBF_SPAM, "found %ld good clues in the second scan", goodClues);

          if(goodClues > 0)
          {
            int chiError = 0;

            S = chi2P(-2.0 * S, 2.0 * goodClues, &chiError);

            if(chiError == 0)
              H = chi2P(-2.0 * H, 2.0 * goodClues, &chiError);

            // if any error, then toss the complete calculation
            if(chiError != 0)
            {
              E(DBF_SPAM, "chi2P error, H=%.8f, S=%.8f, good clues=%ld", H, S, goodClues);
              prob = 0.5;
            }
            else
              prob = (S - H + 1.0) / 2.0;
          }
          else
            prob = 0.5;

          D(DBF_SPAM, "spam probability is %.2f, ham score: %.2f, spam score: %.2f", prob, H, S);
          isSpam = (prob * 100 >= C->SpamProbabilityThreshold);
        }
        else
        {
          // no bad tokens so far, assume ham
          E(DBF_SPAM, "no bad tokens so far, assuming non-spam");
          isSpam = FALSE;
        }
      }
      else
      {
        // no good tokens so far, assume spam
        E(DBF_SPAM, "no good tokens so far, assuming spam");
        isSpam = TRUE;
      }

      free(tokens);
    }
    else
    {
      // cannot copy tokens, assume spam
      E(DBF_SPAM, "cannot copy tokens, assuming spam");
      isSpam = TRUE;
    }

    ReleaseSemaphore(&G->spamFilter.lockSema);

  }
  else
  {
    // sender found in address book, assume ham
    D(DBF_SPAM, "found sender '%s' in address book, assuming non-spam", mail->From.Address);
    isSpam = FALSE;
  }

  RETURN(isSpam);
  return isSpam;
}

///
/// BayesFilterInit
// initialize the spam filter structures and read the training data from disk
BOOL BayesFilterInit(void)
{
  BOOL result = FALSE;

  ENTER();

  if(tokenAnalyzerInit() == TRUE)
  {
    tokenAnalyzerReadTrainingData();

    result = TRUE;
  }

  // no matter if the initialization succeeded or not we treat ourself as initialized
  G->spamFilter.initialized = TRUE;

  RETURN(result);
  return result;
}

///
/// BayesFilterCleanup
// write the training data to disk and cleanup all structures
void BayesFilterCleanup(void)
{
  ENTER();

  // check whether BayesFilterInit() has been called before, otherwise we must not access the semaphore
  if(G->spamFilter.initialized == TRUE)
  {
    ObtainSemaphore(&G->spamFilter.lockSema);

    // only write the spam training data to disk if there are any tokens and if something has changed since the last flush
    if(G->spamFilter.numDirtyingMessages > 0 && (G->spamFilter.goodCount > 0 || G->spamFilter.badCount > 0))
    {
      tokenAnalyzerWriteTrainingData();
    }

    tokenAnalyzerCleanup();

    ReleaseSemaphore(&G->spamFilter.lockSema);

    // we are no longer initialized
    G->spamFilter.initialized = FALSE;
  }

  LEAVE();
}

///
/// tokenizeMail
// tokenize a complete mail, including all headers
static void tokenizeMail(struct Tokenizer *t,
                         const struct Mail *mail)
{
  struct ReadMailData *rmData;

  ENTER();

  if((rmData = AllocPrivateRMData(mail, PM_ALL|PM_QUIET)))
  {
    char *rptr;

    if((rptr = RE_ReadInMessage(rmData, RIM_QUIET)) != NULL)
    {
      // first tokenize all texts
      tokenizerTokenize(t, rptr);

      if(isMultiPartMail(mail))
      {
        struct Part *part;

        // iterate through all mail parts to tokenize attachments, too
        for(part = rmData->firstPart; part != NULL; part = part->Next)
        {
          if(part->headerList != NULL)
            tokenizerTokenizeHeaders(t, part);

          if(part->Nr > PART_RAW && part->Nr != rmData->letterPartNum)
            tokenizerTokenizeAttachment(t, part->ContentType, part->Filename);
        }
      }

      free(rptr);
    }

    FreePrivateRMData(rmData);
  }

  LEAVE();
}

///
/// BayesFilterClassifyMessage
// classify a given message based upon the information gathered so far
BOOL BayesFilterClassifyMessage(const struct Mail *mail)
{
  BOOL isSpam = FALSE;
  struct Tokenizer t;

  ENTER();

  if(tokenizerInit(&t) == TRUE)
  {
    tokenizeMail(&t, mail);

    isSpam = tokenAnalyzerClassifyMessage(&t, mail);

    tokenizerCleanup(&t);
  }

  RETURN(isSpam);
  return isSpam;
}

///
/// BayesFilterSetClassification
// change the classification of a message
void BayesFilterSetClassification(const struct Mail *mail,
                                  const enum BayesClassification newClass)
{
  struct Tokenizer t;

  ENTER();

  if(tokenizerInit(&t) == TRUE)
  {
    enum BayesClassification oldClass;

    if(hasStatusUserSpam(mail))
      oldClass = BC_SPAM;
    else if(hasStatusHam(mail))
      oldClass = BC_HAM;
    else
      oldClass = BC_OTHER;

    tokenizeMail(&t, mail);

    // now we invert the current classification
    tokenAnalyzerSetClassification(&t, oldClass, newClass);

    tokenizerCleanup(&t);
  }

  LEAVE();
}

///
/// BayesFilterNumerOfSpamClassifiedMails
// return the number of spam classified mail
ULONG BayesFilterNumberOfSpamClassifiedMails(void)
{
  ULONG num;

  ENTER();

  ObtainSemaphoreShared(&G->spamFilter.lockSema);
  num = G->spamFilter.badCount;
  ReleaseSemaphore(&G->spamFilter.lockSema);

  RETURN(num);
  return num;
}

///
/// BayesFilterNumerOfSpamClassifiedWords
// return the number of spam classified words
ULONG BayesFilterNumberOfSpamClassifiedWords(void)
{
  ULONG num;

  ENTER();

  ObtainSemaphoreShared(&G->spamFilter.lockSema);
  num = G->spamFilter.badTokens.tokenTable.entryCount;
  ReleaseSemaphore(&G->spamFilter.lockSema);

  RETURN(num);
  return num;
}

///
/// BayesFilterNumberOfHamClassifiedMails
// return the number of ham classified mails
ULONG BayesFilterNumberOfHamClassifiedMails(void)
{
  ULONG num;

  ENTER();

  ObtainSemaphoreShared(&G->spamFilter.lockSema);
  num = G->spamFilter.goodCount;
  ReleaseSemaphore(&G->spamFilter.lockSema);

  RETURN(num);
  return num;
}

///
/// BayesFilterNumberOfHamClassifiedWords
// return the number of ham classified words
ULONG BayesFilterNumberOfHamClassifiedWords(void)
{
  ULONG num;

  ENTER();

  ObtainSemaphoreShared(&G->spamFilter.lockSema);
  num = G->spamFilter.goodTokens.tokenTable.entryCount;
  ReleaseSemaphore(&G->spamFilter.lockSema);

  RETURN(num);
  return num;
}

///
/// BayesFilterFlushTrainingData
// flush training data to disk
void BayesFilterFlushTrainingData(void)
{
  ENTER();

  ObtainSemaphore(&G->spamFilter.lockSema);
  BusyText(tr(MSG_BUSYFLUSHINGSPAMTRAININGDATA), "");

  if(C->SpamFlushTrainingDataThreshold > 0 && G->spamFilter.numDirtyingMessages > (ULONG)C->SpamFlushTrainingDataThreshold)
  {
    tokenAnalyzerWriteTrainingData();
    G->spamFilter.numDirtyingMessages = 0;
  }

  BusyEnd();
  ReleaseSemaphore(&G->spamFilter.lockSema);

  LEAVE();
}

///
/// BayesFilterResetTrainingData
// reset the training data
void BayesFilterResetTrainingData(void)
{
  ENTER();

  tokenAnalyzerResetTrainingData();

  LEAVE();
}

///
/// BayesFilterOptimizeTrainingData
// Optimize the training data
void BayesFilterOptimizeTrainingData(void)
{
  ENTER();

  tokenAnalyzerOptimizeTrainingData();

  LEAVE();
}

///
