#include <proto/intuition.h>
#include <proto/utility.h>

#warning possible incomplete tag list parsing!!

ULONG xset(Object *obj, ...)
{
  struct TagItem *tags = (struct TagItem *)(&obj + 1);
  struct TagItem *tag;
  struct TagItem newTags[6];
  struct TagItem *newTag = newTags;
  int i;

  // We will try to rebuild a new tag list with a terminating TAG_DONE.
  // This approach is not perfect as we have to "guess" the end of the
  // supplied tag list or at least have to set a definite limit on the
  // number of usable attributes. xset() calls usually are quite "simple",
  // hence we accept up to 5 attributes at most to be set with one call.
  i = 0;
  while(i < (sizeof(newTags) / sizeof(newTags[0])) - 1 && (tag = NextTagItem(&tags)) != NULL)
  {
    if(tag->ti_Tag >= TAG_USER)
    {
      // copy any user defined tag item
      newTag->ti_Tag = tag->ti_Tag;
      newTag->ti_Data = tag->ti_Data;
      newTag++;
      i++;
    }
  }

  // add the terminating TAG_DONE
  newTag->ti_Tag = TAG_DONE;

  return SetAttrsA(obj, newTags);
}
