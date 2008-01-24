#ifndef BAYES_FILTER_H
#define BAYES_FILTER_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include "HashTable.h"

// forward declarations
struct Mail;

/*
 YAM's spam filter is based upon Mozilla Thunderbird's junk filter.
 For further information on Thunderbird go to http://www.mozilla.com.

 The core functions of the ThunderBird 2.0.0.0 junk filter can be found here:
 http://mxr.mozilla.org/mozilla1.8/source/mailnews/extensions/bayesian-spam-filter/src/nsBayesianFilter.cpp
 http://mxr.mozilla.org/mozilla1.8/source/mailnews/extensions/bayesian-spam-filter/src/nsBayesianFilter.h
 http://mxr.mozilla.org/mozilla1.8/source/mailnews/extensions/bayesian-spam-filter/src/nsIncompleteGamma.h
 http://mxr.mozilla.org/mozilla1.8/source/xpcom/glue/pldhash.c
 http://mxr.mozilla.org/mozilla1.8/source/xpcom/glue/pldhash.h
*/

// a mail can either be spam, ham (no spam) or not yet classified
enum BayesClassification
{
  BC_SPAM = 0,
  BC_HAM,
  BC_OTHER,
};

#define DEFAULT_SPAM_PROBABILITY_THRESHOLD      90
#define DEFAULT_FLUSH_TRAINING_DATA_INTERVAL    (15 * 60)
#define DEFAULT_FLUSH_TRAINING_DATA_THRESHOLD   50

struct Tokenizer
{
  struct HashTable tokenTable;
};

struct TokenAnalyzer
{
  struct Tokenizer goodTokens;
  struct Tokenizer badTokens;
  ULONG goodCount;
  ULONG badCount;
  ULONG numDirtyingMessages;
};

/*** Public functions ***/
BOOL BayesFilterInit(void);
void BayesFilterCleanup(void);
BOOL BayesFilterClassifyMessage(struct Mail *mail);
void BayesFilterSetClassification(struct Mail *mail, enum BayesClassification newClass);
ULONG BayesFilterNumberOfSpamClassifiedMails(void);
ULONG BayesFilterNumberOfHamClassifiedMails(void);
void BayesFilterFlushTrainingData(void);
void BayesFilterResetTrainingData(void);

#endif /* BAYES_FILTER_H */

