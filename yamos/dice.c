#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

char *index(const char *a, int b) { return strchr(a,b); }
int isascii (int c) { return ((c&0xff)<127); }
int max(int a, int b) { return (a >= b ? a : b); }

int astcsma(const char *s, const char *p)
{
   char *buf;

   if((buf=malloc(strlen(p)*2+2)))
   {
      if (ParsePatternNoCase(p, buf, strlen(p)*2+2) >= 0)
      {
         int ret=MatchPatternNoCase(buf, s);
         free(buf);
         return ret;
      }
      free(buf);
   }
   return 0;
}

void stccpy(char *dest,char *source,int len)
{
   strncpy(dest,source,len);
   dest[len-1]='\0';
}

void strmfp(char *d, const char *path, const char *file)
{
   int len;

   *d=0;
   if(path)
   {
      strcpy(d,path);
      len=strlen(d);
      if(len) if(d[len-1]!='/' && d[len-1]!=':') strcat(d,"/");
   }
   if(file) strcat(d,file);
}

char *stpblk(char *s)
{
   while(isspace(*s++));
   return s;
}

int getft(const char *fn)
{
   struct stat st;

   if(stat(fn,&st)) return -1;

   return st.st_mtime;
}

int stch_i(const char *s,int *res)
{
   int i=0;

   *res=0;
   while(s[i])
   {
      if(s[i]>='0' && s[i]<='9')
      {
         *res<<=4;
         *res+=s[i++]-'0';
         continue;
      }
      if(s[i]>='a' && s[i]<='f')
      {
         *res<<=4;
         *res+=s[i++]-'a'+10;
         continue;
      }
      if(s[i]>='A' && s[i]<='F')
      {
         *res<<=4;
         *res+=s[i++]-'A'+10;
         continue;
      }
      break;
   }

   return i;
}

int stcgfe(char *e, const char *fn)
{
   char *p=fn+strlen(fn);

   while(p>fn && *p!='.') p--;
   if(*p=='.') strcpy(e,++p); else *e=0;

   return strlen(e);
}

void strsfn(const char *fn,char *drive, char *path, char *file, char *ext)
{
   char *p,*i,*k,*filestart=file;
   *drive=*path=*file=*ext=0;

   if((p=strchr(fn,':')))
   {
      i=fn; while(i<=p) *drive++=*i++;
      *drive=0;
      p++;
   } else p=fn;

   k=strrchr(p,'/');

   i=p;
   if(k)
   {
      while(i<k) *path++=*i++;
      *path=0;
      i++;
   }
   k=file;
   while(*i) *k++=*i++;
   *k=0;

   if((p=strchr(filestart,'.')))
   {
      *p=0;
      strcpy(ext,++p);
   }
}

int stcgfn(char *file,const char *fn)
{
   char *p,*i,*k,*filestart=file;
   *file=0;

   if((p=strchr(fn,':')))
   {
      p++;
   } else p=fn;

   k=strrchr(p,'/');

   i=p;
   if(k)
   {
      i=k;
      i++;
   }
   while(*i) *file++=*i++;
   *file=0;

   if((p=strchr(filestart,'.')))
   {
      *p=0;
   }

   return strlen(filestart);
}

#include <proto/exec.h>
struct Library *WorkbenchBase;
struct Library *KeymapBase;
void dice_closelibs(void)
{
   CloseLibrary(WorkbenchBase);
   CloseLibrary(KeymapBase);
}

extern struct Library *XpkBase;
#ifdef __GNUC__
#include <inline/xpkmaster.h>
#else
#include <pragmas/xpkmaster_pragmas.h>
#endif
#ifndef __GNUC__
LONG XpkQueryTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkQuery(_tags);
}

LONG XpkPackTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkPack(_tags);
}

LONG XpkUnpackTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkUnpack(_tags);
}
#endif

extern struct Library *OpenURLBase;
#include <clib/openurl_protos.h>
#ifdef __GNUC__
#include <inline/openurl.h>
#else
#include <pragmas/openurl_pragmas.h>
#endif
#ifndef __GNUC__
BOOL URL_Open(STRPTR str, ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return URL_OpenA(str,_tags);
}
#endif
