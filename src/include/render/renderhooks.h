#ifndef RENDERHOOKS_H
#define RENDERHOOKS_H
/*
**    $VER: renderhooks.h 18.2 (5.3.97)
**
**    render.library definitions for callback hooks
*/



#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

/******************************************************************************

	Progress Hook Message

	Whenever your progress hook is called, your function
	receives a pointer to this structure in a1. Check the
	message type and proceed accordingly.

	Also, you get a pointer to the object of concern in a2.
	Warning: This is intended for identification only. You
	are NOT allowed to perform operations inside your hook
	function that could modify this object. If you try to
	do so, your code will run into a deadlock.

	Your progress hook has to return TRUE or FALSE
	for continuation respective abortion.

******************************************************************************/

struct	RND_ProgressMessage
{
	ULONG	RND_PMsg_type;		/* type of message, see below      */
	ULONG	RND_PMsg_count;		/* number to be displayed...       */
	ULONG	RND_PMsg_total;		/* ...inside this range of numbers */
};

/******************************************************************************

	Types of progress messages

	Neither depend on a certain number of calls nor on
	calls in a specific order. It's up to the library
	to decide
	- how often to call your progress hook
	- in what order to submit different types of messages
	- in what step rate to call your progress hook
	- whether to call your progress hook at all

******************************************************************************/

	/* number of lines added to a histogram.
	   a2 is a pointer to the histogram. */

#define	PMSGTYPE_LINES_ADDED		1


	/* number of colors chosen during quantization.
	   a2 is a pointer to the histogram. */

#define PMSGTYPE_COLORS_CHOSEN		2


	/* number of histogram entries adapted to the palette.
	   a2 is a pointer to the histogram. */

#define PMSGTYPE_COLORS_ADAPTED		3


	/* number of lines rendered to a palette.
	   a2 is a pointer to the palette. */

#define PMSGTYPE_LINES_RENDERED		4


	/* number of lines converted.
	   a2 is NULL. */

#define PMSGTYPE_LINES_CONVERTED	5




/******************************************************************************

	Line Hook Message

	This hook is executed by functions such as Render() once
	before and once after converting a line.

	When your line hook is called, your function receives a
	pointer to this structure in a1. Check the message type and
	proceed accordingly. Also, you get a pointer to the object
	of concern in a2. This is either the source or destination
	buffer.

	This allows you to draw, save, convert etc. while rendering,
	and to save memory. Specify RND_DestWidth = 0 to render into
	a single-line buffer, and RND_SourceWidth = 0 to fetch from a
	single-line buffer.

	Your line hook has to return TRUE or FALSE
	for continuation respective abortion.

******************************************************************************/

struct	RND_LineMessage
{
	ULONG	RND_LMsg_type;		/* type of message, see below */
	ULONG	RND_LMsg_row;		/* the row number being processed */
};



/* just completed a line. a2 is a pointer to the rendered data.
   You may read from this buffer. */

#define	LMSGTYPE_LINE_RENDERED		6


/* now converting a new line. a2 is a pointer to the source buffer
   where the input is expected. You may write to this buffer. */

#define	LMSGTYPE_LINE_FETCH		7



/*****************************************************************************/

#endif
