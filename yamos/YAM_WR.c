/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 2000  Marcel Beck <mbeck@yam.ch>

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

***************************************************************************/

#include "YAM.h"

/* local protos */
LOCAL void WR_ComposeMulti(FILE*, struct Compose*, char*);
LOCAL struct WritePart *BuildPartsList(int);
LOCAL char *GetDateTime(void);
LOCAL char *NewID(BOOL);
LOCAL int WhichEncodingForFile(char*, char*);
LOCAL int WR_CharOut(char);
LOCAL char *firstbad(char*);
LOCAL void PutQP(unsigned char, FILE*);
LOCAL void HeaderFputs(char*, FILE*);
LOCAL void EmitRcptField(FILE*, char*);
LOCAL void EmitRcptHeader(FILE*, char*, char*);
LOCAL void FPutsQuoting(char*, FILE*);
LOCAL void WriteCtypeNicely(FILE*, char*);
LOCAL void WriteContentTypeAndEncoding(FILE*, struct WritePart*);
LOCAL void WR_WriteUIItem(FILE*, int*, char*, char*);
LOCAL void WR_WriteUserInfo(FILE*);
LOCAL void EncodePart(FILE*, struct WritePart*);
LOCAL BOOL WR_CreateHashTable(char*, char*, char*);
LOCAL void WR_AddTagline(FILE*);
LOCAL void WR_WriteSignature(FILE*, int);
LOCAL void WR_Anonymize(FILE*, char*);
LOCAL char *WR_GetPGPId(struct Person*);
LOCAL char *WR_GetPGPIds(char*, char*);
LOCAL BOOL WR_Bounce(FILE*, struct Compose*);
LOCAL BOOL WR_SaveDec(FILE*, struct Compose*);
LOCAL void WR_EmitExtHeader(FILE*, struct Compose*);
LOCAL void WR_ComposeReport(FILE*, struct Compose*, char*);
LOCAL void SetDefaultSecurity(struct Compose*);
LOCAL BOOL WR_ComposePGP(FILE*, struct Compose*, char*);
LOCAL char *WR_TransformText(char*, int, char*);
LOCAL void WR_SharedSetup(struct WR_ClassData*, int);
LOCAL APTR MakeAddressField(APTR*, char*, APTR, int, int, BOOL);
LOCAL struct WR_ClassData *WR_NewBounce(int);


/***************************************************************************
 Module: Write
***************************************************************************/

/*** Translate aliases ***/
/// WR_ResolveName
//  Looks for an alias, email address or name in the address book
char FailedAlias[SIZE_NAME];
int WR_ResolveName(int winnum, char *name, char **adrstr, BOOL nolists)
{
   int hits = 0, i, retcode;
   char *p;
   struct ABEntry *ab;
   struct MUIS_Listtree_TreeNode *tn, *tn2;
   struct Person pe;

   ExtractAddress(name, &pe);
   if (pe.Address[0]) if (p = strchr(pe.Address, '@')) // is it an address?
   {
      if (!p[1]) strcpy(p, strchr(C->EmailAddress, '@'));
      if (**adrstr) *adrstr = StrBufCat(*adrstr, ", ");
      *adrstr = StrBufCat(*adrstr, BuildAddrName2(&pe));
      return 0; // no error or warning
   }
   pe.Address[0] = 0;
   stccpy(FailedAlias, name, SIZE_NAME);
   AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, name, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &hits, &tn);
   if (hits > 1) return 3; // multiple matches
   if (!hits)
   {
      AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, name, ASM_REALNAME|ASM_USER|ASM_LIST|ASM_GROUP, &hits, &tn);
      if (hits > 1) return 3; else if (!hits) return 2;
   }
   ab = tn->tn_User;
   switch (ab->Type)
   {
      case AET_USER:
         if (**adrstr) *adrstr = StrBufCat(*adrstr, ", ");
         *adrstr = StrBufCat(*adrstr, BuildAddrName(ab->Address, ab->RealName));
         break;
      case AET_LIST:
         if (nolists) return 4;
         if (winnum >= 0) if ((ab->Address[0] || ab->RealName[0]) && !G->WR[winnum]->ListEntry) G->WR[winnum]->ListEntry = ab;
         if (ab->Members)
         {
            char *ptr;
            for (ptr = ab->Members; *ptr; ptr++)
            {
               char *nptr = strchr(ptr, '\n');
               if (nptr) *nptr = 0; else break;
               retcode = WR_ResolveName(winnum, ptr, adrstr, nolists);
               *nptr = '\n'; ptr = nptr;
               if (retcode) return retcode;
            }
         }
         break;
      case AET_GROUP:
         if (nolists) return 4;
         for (i=0; ; i++)
            if (tn2 = (struct MUIS_Listtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADRESSES, MUIM_Listtree_GetEntry, tn, i, MUIV_Lt_GetEntry_Flags_SameLevel))
            {
               struct ABEntry *ab2 = tn2->tn_User;
               if (retcode = WR_ResolveName(winnum, ab2->Alias, adrstr, nolists)) return retcode;
            }
            else break;
         break;
   }
   return 0;
}


///
/// WR_ExpandAddresses
//  Expands aliases and names in a recipient field to valid e-mail addresses
char *WR_ExpandAddresses(int winnum, char *src, BOOL quiet, BOOL single)
{
   char *source, *buffer = malloc(strlen(src)+1), *next, *adr = AllocStrBuf(SIZE_DEFAULT);
   int err;

   strcpy(source = buffer, src);
   while (source)
   {
      if (!*source) break;
      if (next = MyStrChr(source, ',')) *next++ = 0;
      if (err = WR_ResolveName(winnum, Trim(source), &adr, single))
      {
         if (err == 2 && !quiet) ER_NewError(GetStr(MSG_ER_AliasNotFound), FailedAlias, NULL);
         if (err == 3 && !quiet) ER_NewError(GetStr(MSG_ER_AmbiguousAlias), FailedAlias, NULL);
         if (err == 4 && !quiet) ER_NewError(GetStr(MSG_ER_InvalidAlias), FailedAlias, NULL);
         FreeStrBuf(adr); adr = NULL;
         break;
      }
      if (single) break;
      source = next;
   }
   free(buffer);
   return adr;
}


///
/// WR_VerifyAutoFunc
//  Checks recipient field (user pressed return key)
SAVEDS ASM void WR_VerifyAutoFunc(REG(a1) int *arg)
{
   APTR str = (APTR)arg[0], next = str;
   char *value, *adr;
   get(str, MUIA_String_Contents, &value);
   if (adr = WR_ExpandAddresses(arg[1], value, TRUE, arg[2]))
   {
      FreeStrBuf(adr);
      get(str, MUIA_UserData, &next);
   }
   else DisplayBeep(0);
   if (next) set(_win(next), MUIA_Window_ActiveObject, next);
}
MakeHook(WR_VerifyAutoHook, WR_VerifyAutoFunc);


///
/// WR_VerifyManualFunc
//  Checks and expands recipient field (user clicked gadget)
SAVEDS ASM void WR_VerifyManualFunc(REG(a1) int *arg)
{
   APTR object = (APTR)arg[0];
   char *value, *adr;
   get(object, MUIA_String_Contents, &value);
   if (adr = WR_ExpandAddresses(arg[1], value, FALSE, arg[2]))
   {
      setstring(object, adr);
      FreeStrBuf(adr);
   }
}
MakeHook(WR_VerifyManualHook, WR_VerifyManualFunc);
///


/*** Attachments list ***/
/// WR_GetFileEntry
//  Fills form with data from selected list entry
SAVEDS ASM void WR_GetFileEntry(REG(a1) int *arg)
{
   int winnum = *arg;
   struct Attach *attach = NULL;
   struct WR_GUIData *gui = &G->WR[winnum]->GUI;

   DoMethod(gui->LV_ATTACH, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &attach);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, attach ? FALSE : TRUE, gui->RA_ENCODING, gui->ST_CTYPE, gui->ST_DESC, gui->BT_DEL, gui->BT_DISPLAY, NULL);
   if (attach)
   {
      nnset(gui->RA_ENCODING, MUIA_Radio_Active, attach->IsMIME ? 0 : 1);
      nnset(gui->ST_CTYPE, MUIA_String_Contents, attach->ContentType);
      nnset(gui->ST_DESC, MUIA_String_Contents, attach->Description);
   }
}
MakeHook(WR_GetFileEntryHook, WR_GetFileEntry);


///
/// WR_PutFileEntry
//  Fills form data into selected list entry
SAVEDS ASM void WR_PutFileEntry(REG(a1) int *arg)
{
   int winnum = *arg;
   struct Attach *attach = NULL;
   struct WR_GUIData *gui = &G->WR[winnum]->GUI;
   int ismime;

   DoMethod(gui->LV_ATTACH, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &attach);
   if (attach)
   {
      get(gui->RA_ENCODING, MUIA_Radio_Active, &ismime);
      attach->IsMIME = ismime == 0;
      GetMUIString(attach->ContentType, gui->ST_CTYPE);
      GetMUIString(attach->Description, gui->ST_DESC);
      DoMethod(gui->LV_ATTACH, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }
}
MakeHook(WR_PutFileEntryHook, WR_PutFileEntry);


///
/// WR_AddFileToList
//  Adds a file to the attachment list, gets its size and type
BOOL WR_AddFileToList(int winnum, char *filename, char *name, BOOL istemp)
{
   static struct Attach attach;
   struct FileInfoBlock *fib;
   struct WR_GUIData *gui = &G->WR[winnum]->GUI;
   char *ctype;
   int encoding;
   BPTR lock;

   if (!filename) return FALSE;
   clear(&attach, sizeof(struct Attach));
   if (!(lock = Lock(filename, ACCESS_READ))) return FALSE;
   fib = AllocDosObject(DOS_FIB, NULL);
   Examine(lock, fib);
   attach.Size = fib->fib_Size;
   stccpy(attach.Description, fib->fib_Comment, SIZE_DEFAULT);
   FreeDosObject(DOS_FIB, fib);
   UnLock(lock);
   ctype = IdentifyFile(filename);
   if (*ctype)
   {  
      strcpy(attach.FilePath, filename); 
      strcpy(attach.Name, name ? name : (char *)FilePart(filename));
      get(gui->RA_ENCODING, MUIA_Radio_Active, &encoding);
      attach.IsMIME = encoding == 0;
      attach.IsTemp = istemp;
      strcpy(attach.ContentType, ctype);
      nnset(gui->ST_CTYPE, MUIA_String_Contents, attach.ContentType);
      nnset(gui->ST_DESC, MUIA_String_Contents, attach.Description);
      DoMethod(gui->LV_ATTACH, MUIM_List_InsertSingle, &attach, MUIV_List_Insert_Bottom);
      set(gui->LV_ATTACH, MUIA_List_Active, MUIV_List_Active_Bottom);
      return TRUE;
   }
   return FALSE;
}  
///


/*** Compose Message ***/
/// GetDateTime
//  Formats current date and time for Date header field
LOCAL char *GetDateTime(void)
{
   static char dt[SIZE_DEFAULT];
   char *tz;
   time_t now;
   struct tm tm;

   time(&now);
   tm = *(localtime(&now));
   sprintf(dt, "%s, %02ld %s %ld %02ld:%02ld:%02ld", wdays[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
   if (tz = GetTZ()) { strcat(dt, " "); strcat(dt, tz); }
   return dt;  
}


///
/// NewID
//  Creates a unique id, used for Message-ID header field
LOCAL char *NewID(BOOL is_msgid)
{
   static char idbuf[SIZE_MSGID];
   static int ctr = 0;
   struct DateStamp ds;

   DateStamp(&ds);
   if (is_msgid) sprintf(idbuf, "yam%ld.%ld.%ld@%s", ds.ds_Days, ds.ds_Tick, FindTask(NULL), C->SMTP_Server);
   else          sprintf(idbuf, "%ld.%ld", FindTask(NULL), ++ctr);
   return idbuf;
}


///
/// WhichEncodingForFile
//  Determines best MIME encoding mode for a file
LOCAL int WhichEncodingForFile(char *fname, char *ctype)
{
   int c, linesize=0, total=0, unsafechars=0, binarychars=0, longlines=0;
   FILE *fh = fopen(fname, "r");

   if (!fh) return ENC_B64;
   while ((c = fgetc(fh)) != -1)
   {
      if (c > 127) ++unsafechars;
      if (c < 32 && c != '\t' && c != '\n' && c != '\r') ++binarychars;
      ++total;
      if (c == '\n') 
      {
         if (linesize > 79) ++longlines;
         linesize = 0;
      } else ++linesize;
      if (total > 4000 && (longlines || unsafechars || binarychars)) break;
   }
   fclose(fh);
   if (longlines || unsafechars || binarychars)
   {
      if (!strnicmp(ctype, "image/", 6) ||
          !strnicmp(ctype, "audio/", 6) ||
          !strnicmp(ctype, "application/octet-stream", 24) ||
          !strnicmp(ctype, "video/", 6))
         return ENC_B64;
      if (!strnicmp(ctype, "message/", 8)) return (longlines) ? ENC_BIN : ENC_8BIT;
      if (!unsafechars && !binarychars) return C->Allow8bit ? ENC_BIN : ENC_QP;
      if (C->Allow8bit && !binarychars) return ENC_8BIT;
      return (total/++unsafechars < 16) ? ENC_B64 : ENC_QP;
   }
   return ENC_NONE;
}


///
/// NewPart
//  Initializes a new message part
struct WritePart *NewPart(int winnum)
{
   struct WritePart *p;
   p = (struct WritePart *)calloc(1,sizeof(struct WritePart));
   p->ContentType = "text/plain";
   p->EncType = ENC_NONE;
   p->Filename = G->WR_Filename[winnum];
//   p->Name = NULL; // redundant due to calloc() -msbethke
   return p;
}


///
/// BuildPartsList
//  Builds message parts from attachment list
LOCAL struct WritePart *BuildPartsList(int winnum)
{
   int i;
   struct Attach *att;
   struct WritePart *first, *p, *np;

   first = p = NewPart(winnum); p->IsTemp = TRUE;
   p->EncType = WhichEncodingForFile(p->Filename, p->ContentType);
   for (i = 0; ; i++)
   {
      DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_List_GetEntry, i, &att);
      if (!att) break;
      p->Next = np = NewPart(winnum); p = np;
      p->ContentType = att->ContentType;
      p->Filename    = att->FilePath;
      p->Description = att->Description;     
      p->Name        = att->Name;
      p->IsTemp      = att->IsTemp;
      if (att->IsMIME) p->EncType = WhichEncodingForFile(p->Filename, p->ContentType);
      else p->EncType = ENC_UUE;
   }
   return first;
}

///
/// FreePartsList
//  Clears message parts and deletes temporary files
void FreePartsList(struct WritePart *p)
{
   struct WritePart *np;
   for (; p; p = np)
   {
      np = p->Next;
      if (p->IsTemp) DeleteFile(p->Filename);
      free(p);
   }
}

///
/// WR_CharOut
//  Outputs a single byte of a message header
LOCAL int WR_CharOut(char c)
{
   if (G->TTout) if (G->TTout->Header) return (int)G->TTout->Table[(UBYTE)c];
   return (int)c;
}

///
/// firstbad
//  Returns a pointer to the first non-ascii or control character in a string
LOCAL char *firstbad(char *s)
{
   unsigned char *dum;
   for (dum = (unsigned char *)s; *dum; ++dum)
      if (!isascii(WR_CharOut(*dum)) || iscntrl(WR_CharOut(*dum))) return (char *)dum;
   return NULL;
}

///
/// PutQP
//  Outputs a base64 encoded, single byte of a message header
LOCAL void PutQP(unsigned char c, FILE *fh)
{
   static char basis_hex[] = "0123456789ABCDEF";
   fputc('=', fh);
   fputc(basis_hex[c>>4], fh);
   fputc(basis_hex[c&0xF], fh);
}

///
/// HeaderFputs
//  Outputs the value of a header line (optional QP encoding and charset translation)
LOCAL void HeaderFputs(char *s, FILE *fh)
{
   char *firstnonascii, *wordstart;

   if (!s) return;
   while (firstnonascii = firstbad(s))
   {
      for (wordstart = firstnonascii; wordstart >= s; wordstart--) if (ISpace(*wordstart)) break;
      while (s <= wordstart) { fputc(WR_CharOut(*s), fh); ++s; }
      fprintf(fh, "=?%s?Q?", G->TTout ? G->TTout->DestCharset : C->LocalCharset);
      for (; *s && !ISpace(*s); ++s)
      {
         char c = (char)WR_CharOut(*s);
         if (c > ' ') fputc(c, fh); else PutQP((unsigned char)c, fh);
         }
      fputs("?=", fh);
   }
   while (*s) { fputc(WR_CharOut(*s), fh); ++s; }
}

///
/// EmitHeader
//  Outputs a complete header line
void EmitHeader(FILE *fh, char *hdr, char *body)
{
   fprintf(fh, "%s: ", hdr);
   HeaderFputs(body, fh);
   fputc('\n', fh);
}

///
/// EmitRcptField
//  Outputs the value of a recipient header line, one entry per line
LOCAL void EmitRcptField(FILE *fh, char *body)
{
   char *part, *next, *bodycpy = malloc(strlen(body)+1);

   strcpy(part = bodycpy, body);
   while (part)
   {
      if (!*part) break;
      if (next = MyStrChr(part, ',')) *next++ = 0;
      HeaderFputs(Trim(part), fh);
      if (part = next) fputs(",\n\t", fh);
   }
   free(bodycpy);
} 

///
/// EmitRcptHeader
//  Outputs a complete recipient header line
LOCAL void EmitRcptHeader(FILE *fh, char *hdr, char *body)
{
   fprintf(fh, "%s: ", hdr);
   EmitRcptField(fh, body ? body : "");
   fputc('\n', fh);
} 

///
/// FPutsQuoting
//  Handles quotes and slashes
LOCAL void FPutsQuoting(char *s, FILE *fh)
{
   char *end = s + strlen(s) - 1;
   while (ISpace(*end) && end > s) --end;
   if (*s == '\"') 
   {
      fputc(*s, fh);
      while (*++s) 
      {
         if (*s == '\"') break;
         if (*s == '\\') { fputc(*s, fh); ++s; if (!*s) break; }
         fputc(*s, fh);
      }
      fputc('\"', fh);
   }
   else 
   {
      fputc('\"', fh); fputc(*s, fh);
      while (*++s) 
      {
         if (*s == '\"' || *s == '\\') fputc('\\', fh);
         fputc(*s, fh);
      }
      fputc('\"', fh);
   }
}

///
/// WriteCtypeNicely
//  Outputs content type
LOCAL void WriteCtypeNicely(FILE *fh, char *ct)
{
   char *semi, *slash, *eq, *s;

   for (s = ct; *s; ++s) if (*s == '\n') *s = ' ';
   if (semi = (char *)index(ct, ';')) *semi = '\0';
   slash = (char *)index(ct, '/');
   fputs(ct, fh);
   if (!slash) fputs("/unknown", fh);
   while (semi) 
   {
      ct = semi + 1;
      *semi = ';';
      if (semi = (char *) index(ct, ';')) *semi = '\0';
      if (eq = (char *) index(ct, '=')) *eq = '\0';
      fputs(";\n\t", fh);
      while (ISpace(*ct)) ++ct;
      fputs(ct, fh);
      if (eq) 
      {
         s = eq;
         fputc('=', fh);
         ++s;
         while (ISpace(*s)) ++s;
         FPutsQuoting(s, fh);
         *eq = '=';
      }
   }
}
///
/// WriteContentTypeAndEncoding
//  Outputs content type header including parameters
LOCAL void WriteContentTypeAndEncoding(FILE *fh, struct WritePart *part)
{
   char *p;
   fputs("Content-Type: ", fh);
   WriteCtypeNicely(fh, part->ContentType);
   if (!strncmp(part->ContentType, "text/", 5) && (part->EncType != ENC_NONE || part->TTable))
      fprintf(fh, "; charset=%s", part->TTable ? part->TTable->DestCharset : C->LocalCharset);
   if (p = part->Name) if (*p) 
   {
      fputs("; name=\"", fh);
      HeaderFputs(p, fh);
      fputs("\"\nContent-Disposition: attachment; filename=\"", fh);
      HeaderFputs(p, fh);
      fputc('\"', fh);
   }
   fputc('\n', fh);
   if (part->EncType != ENC_NONE)
   {
      fputs("Content-Transfer-Encoding: ", fh);
      switch (part->EncType)
      {
         case ENC_B64:  fputs("base64\n", fh); break;
         case ENC_QP:   fputs("quoted-printable\n", fh); break;
         case ENC_UUE:  fputs("x-uue\n", fh); break;
         case ENC_8BIT: fputs("8bit\n", fh); break;
         case ENC_BIN:  fputs("binary\n", fh); break;
      }
   }
   if (p = part->Description) if (*p) EmitHeader(fh, "Content-Description", p);
}

///
/// WR_WriteUIItem
//  Outputs a single parameter of the X-SenderInfo header
LOCAL void WR_WriteUIItem(FILE *fh, int *len, char *parameter, char *value)
{
   int l = 6+strlen(parameter)+strlen(value);
   *len += l;
   fputc(';', fh); if (*len > 80) { fputs("\n  ", fh); *len = l; }
   fprintf(fh, " %s=\"", parameter);
   HeaderFputs(value, fh);
   fputc('\"', fh);
}

///
/// WR_WriteUserInfo
//  Outputs X-SenderInfo header line
LOCAL void WR_WriteUserInfo(FILE *fh)
{
   int len = 15, hits = 0;
   struct ABEntry *ab = NULL;
   struct MUIS_Listtree_TreeNode *tn;

   if (AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, C->EmailAddress, ASM_ADDRESS|ASM_USER, &hits, &tn))
   {
      ab = tn->tn_User;
      if (ab->Type != AET_USER) ab = NULL;
      else if (!*ab->Homepage && !*ab->Phone && !*ab->Street && !*ab->City && !*ab->Country && !ab->BirthDay) ab = NULL;
   }
   if (!ab && !*C->MyPictureURL) return;
   fputs("X-SenderInfo: 1", fh);
   if (*C->MyPictureURL) WR_WriteUIItem(fh, &len, "picture", C->MyPictureURL);
   if (ab)
   {
      if (*ab->Homepage) WR_WriteUIItem(fh, &len, "homepage", ab->Homepage);
      if (*ab->Street)   WR_WriteUIItem(fh, &len, "street", ab->Street);
      if (*ab->City)     WR_WriteUIItem(fh, &len, "city", ab->City);
      if (*ab->Country)  WR_WriteUIItem(fh, &len, "country", ab->Country);
      if (*ab->Phone)    WR_WriteUIItem(fh, &len, "phone", ab->Phone);
      if (ab->BirthDay)  fprintf(fh, "; dob=%ld", ab->BirthDay);
   }
   fputc('\n', fh);
}
///
/// EncodePart
//  Encodes a message part
LOCAL void EncodePart(FILE *ofh, struct WritePart *part)
{
   FILE *ifh;

   if (ifh = fopen(part->Filename, "r"))
   {
      int size;
      switch (part->EncType) 
      {
         case ENC_B64: to64(ifh, ofh, DoesNeedPortableNewlines(part->ContentType));
                       break;
         case ENC_QP:  toqp(ifh, ofh);
                       break;
         case ENC_UUE: size = FileSize(part->Filename);
                       fprintf(ofh, "begin 644 %s\n", *part->Name ? part->Name : (char *)FilePart(part->Filename));
                       touue(ifh, ofh);
                       fprintf(ofh, "end\nsize %ld\n", size);
                       break;
         default:      CopyFile(NULL, ofh, NULL, ifh);
      }
      fclose(ifh);
   }
}

///
/// WR_CreateHashTable
//  Creates an index table for a database file
LOCAL BOOL WR_CreateHashTable(char *source, char *hashfile, char *sep)
{
   char buffer[SIZE_LARGE];
   long fpos, l = strlen(sep);
   FILE *in, *out;
   BOOL success = FALSE;

   if (in = fopen(source, "r"))
   {
      if (out = fopen(hashfile, "w"))
      {
         fpos = 0; fwrite(&fpos, sizeof(long), 1, out);
         while (fgets(buffer, SIZE_LARGE, in))
            if (!strncmp(buffer, sep, l)) { fpos = ftell(in); fwrite(&fpos, sizeof(long), 1, out); }
         success = TRUE;
         fclose(out);
      }
      fclose(in);
   }
   return success;
}

///
/// WR_AddTagline
//  Randomly selects a tagline and writes it to the message file
LOCAL void WR_AddTagline(FILE *fh_mail)
{
   FILE *fh_tag, *fh_hash;
   char buf[SIZE_LARGE], hashfile[SIZE_PATHFILE];
   long fpos, hsize;

   if (*C->TagsFile) 
   {
      sprintf(hashfile, "%s.hsh", C->TagsFile);
      if (getft(C->TagsFile) > getft(hashfile)) WR_CreateHashTable(C->TagsFile, hashfile, C->TagsSeparator);
      if (fh_tag = fopen(C->TagsFile, "r"))
      {
         if (fh_hash = fopen(hashfile, "r"))
         {
            fseek(fh_hash, 0, SEEK_END); hsize = ftell(fh_hash);
            if ((hsize = hsize/sizeof(long)) > 1)
            {
               fpos = (((long)rand())%hsize)*sizeof(long);
               fseek(fh_hash, fpos, SEEK_SET); fread(&fpos, sizeof(long), 1, fh_hash);
               fseek(fh_tag, fpos, SEEK_SET);
               if (GetLine(fh_tag, buf, SIZE_LARGE)) fprintf(fh_mail, buf);
               while (GetLine(fh_tag, buf, SIZE_LARGE))
                  if (!strncmp(buf, C->TagsSeparator, strlen(C->TagsSeparator))) break;
                  else fprintf(fh_mail, "\n%s", buf);
            }
            fclose(fh_tag);
         }
         else ER_NewError(GetStr(MSG_ER_CantOpenFile), hashfile, NULL);
      }
      else ER_NewError(GetStr(MSG_ER_CantOpenFile), C->TagsFile, NULL);
   }
}

///
/// WR_WriteSignature
//  Writes signature to the message file
LOCAL void WR_WriteSignature(FILE *out, int signat)
{
   FILE *in;
   int ch;
   if (in = fopen(CreateFilename(SigNames[signat]), "r"))
   {
      fputs("-- \n", out);
      while ((ch = fgetc(in)) != EOF)
      {
         if (ch == '%')
         {
            ch = fgetc(in);
            if (ch == 't') { WR_AddTagline(out); continue; }
            if (ch == 'e') { CopyFile(NULL, out, "ENV:SIGNATURE", NULL); continue; }
            ungetc(ch, in); ch = '%';
         }
         fputc(ch, out);
      }
      fclose(in);
   }
}

///
/// WR_AddSignature
//  Adds a signature to the end of the file
void WR_AddSignature(char *mailfile, int signat)
{
   FILE *fh_mail;
   BOOL addline = FALSE;

   if (signat == -1) 
   {
      signat = C->UseSignature ? 1 : 0;
      if (fh_mail = fopen(mailfile, "r"))
      {
         fseek(fh_mail, -1, SEEK_END);
         addline = fgetc(fh_mail) != '\n';
         fclose(fh_mail);
      }
   }
   if (fh_mail = fopen(mailfile, "a"))
   {
      if (addline) fputc('\n', fh_mail);
      if (signat) WR_WriteSignature(fh_mail, signat-1);
      fclose(fh_mail);
   }
}

///
/// WR_Anonymize
//  Inserts recipient header field for remailer service
LOCAL void WR_Anonymize(FILE *fh, char *body)
{
   char *ptr;

   for (ptr = C->RMCommands; *ptr; ptr++)
   {
      if (*ptr == '\\') if (*(ptr+1) == 'n') { ptr++; fputs("\n", fh); continue; }
      if (*ptr == '%') if (*(ptr+1) == 's') { ptr++; EmitRcptField(fh, body); continue; }
      fputc(*ptr, fh);
   }
   fputs("\n", fh);
}

///
/// WR_GetPGPId
//  Gets PGP key id for a person
LOCAL char *WR_GetPGPId(struct Person *pe)
{
   int hits;
   char *pgpid = NULL;
   struct MUIS_Listtree_TreeNode *tn;
   if (!AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, pe->RealName, ASM_REALNAME|ASM_USER, &hits, &tn))
        AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, pe->Address, ASM_ADDRESS|ASM_USER, &hits, &tn);
   if (hits && tn && tn->tn_User)
		if (((struct ABEntry *)(tn->tn_User))->PGPId[0])
			pgpid = ((struct ABEntry *)(tn->tn_User))->PGPId;
   return pgpid;
}
///
/// WR_GetPGPIds
//  Collects PGP key ids for all persons in a recipient field
LOCAL char *WR_GetPGPIds(char *source, char *ids)
{
   struct Person pe;
   char *next, *pid;

   for (; source; source = next)
   {
      if (!*source) break;
      if (next = MyStrChr(source, ',')) *next++ = 0;
      ExtractAddress(source, &pe);
      if (!(pid = WR_GetPGPId(&pe)))
      {
         pid = pe.RealName[0] ? pe.RealName : pe.Address;
         ER_NewError(GetStr(MSG_ER_ErrorNoPGPId), source, pid);
      }
      ids = StrBufCat(ids, (G->PGPVersion == 5) ? "-r \"" : "\"");
      ids = StrBufCat(ids, pid);
      ids = StrBufCat(ids, "\" ");
   }
   return ids;
}
///

/// WR_Bounce
//  Bounce message: inserts resent-headers while copying the message
LOCAL BOOL WR_Bounce(FILE *fh, struct Compose *comp)
{
   FILE *oldfh;
   if (oldfh = fopen(GetMailFile(NULL, NULL, comp->OrigMail), "r"))
   {
      BOOL infield = FALSE, inbody = FALSE;
      char buf[SIZE_LINE];
      while (fgets(buf, SIZE_LINE, oldfh))
      {
         if (*buf == '\n' && !inbody)
         {
            inbody = TRUE;
            EmitRcptHeader(fh, "To", comp->MailTo);
            EmitHeader(fh, "Resent-From", BuildAddrName(C->EmailAddress, C->RealName));
            EmitHeader(fh, "Resent-Date", GetDateTime());
         }
         if (!ISpace(*buf) && !inbody) infield = !strnicmp(buf, "to:", 3);
         if (!infield || inbody) fputs(buf, fh);
      }
      fclose(oldfh);
      return TRUE;
   }
   return FALSE;
}

///
/// WR_SaveDec
//  Creates decrypted copy of a PGP encrypted message
LOCAL BOOL WR_SaveDec(FILE *fh, struct Compose *comp)
{
   FILE *oldfh;
   if (oldfh = fopen(GetMailFile(NULL, NULL, comp->OrigMail), "r"))
   {
      BOOL infield = FALSE;
      char buf[SIZE_LINE];
      while (fgets(buf, SIZE_LINE, oldfh))
      {
         if (*buf == '\n') { fprintf(fh, "X-YAM-Decrypted: PGP; %s\n", GetDateTime()); break; }
         if (!ISpace(*buf)) infield = !strnicmp(buf, "content-type:", 13)
                                   || !strnicmp(buf, "content-transfer-encoding", 25)
                                   || !strnicmp(buf, "mime-version:", 13);
         if (!infield) fputs(buf, fh);
      }
      fclose(oldfh);
      return TRUE;
   }
   return FALSE;
}

///
/// WR_EmitExtHeader
//  Outputs special X-YAM-Header lines to remember user-defined headers
LOCAL void WR_EmitExtHeader(FILE *fh, struct Compose *comp)
{
   if (*comp->ExtHeader)
   {
      char *p, ch = '\n';
      for (p = comp->ExtHeader; *p; ++p)
      {
         if (ch == '\n') fputs("X-YAM-Header-", fh);
         if (*p != '\\') ch = *p;
         else if (*++p == '\\') ch = '\\'; else if (*p == 'n') ch = '\n';
         fputc(ch, fh);
      }
      if (ch != '\n') fputc('\n', fh);
   }
}

///
/// WR_ComposeReport
//  Assembles the parts of a message disposition notification
LOCAL const char *MIMEwarn = "Warning: This is a message in MIME format. Your mail reader does not\n"
                             "support MIME. Some parts of this message will be readable as plain text.\n"
                             "To see the rest, you will need to upgrade your mail reader. Following are\n"
                             "some URLs where you can find MIME-capable mail programs for common platforms:\n\n"
                             "  Amiga............: YAM          http://www.yam.ch/\n"
                             "  Unix.............: Metamail     ftp://ftp.bellcore.com/nsb/\n"
                             "  Windows/Macintosh: Eudora       http://www.qualcomm.com/\n\n"
                             "General info about MIME can be found at:\n\n"
                             "http://www.cis.ohio-state.edu/hypertext/faq/usenet/mail/mime-faq/top.html\n\n";
LOCAL const char *PGPwarn  = "The following body part contains a PGP encrypted message. Either your\n"
                             "mail reader doesn't support MIME/PGP as specified in RFC 2015, or\n"
                             "the message was encrypted for someone else. To read the encrypted\n"
                             "message, run the next body part through Pretty Good Privacy.\n\n";
LOCAL void WR_ComposeReport(FILE *fh, struct Compose *comp, char *boundary)
{
   struct WritePart *p;
   fprintf(fh, "Content-type: multipart/report; report-type=disposition-notification; boundary=\"%s\"\n\n", boundary);
   for (p = comp->FirstPart; p; p = p->Next)
   {
      fprintf(fh, "\n--%s\n", boundary);
      WriteContentTypeAndEncoding(fh, p);
      fputs("\n", fh);
      EncodePart(fh, p);
   }
   fprintf(fh, "\n--%s--\n\n", boundary);
}

LOCAL void SetDefaultSecurity(struct Compose *comp)
{
STRPTR CheckThese[3],buf;
int i, Security=0;
BOOL FirstAddr=TRUE;

	/* collect address pointers for easier iteration */
	CheckThese[0] = comp->MailTo;
	CheckThese[1] = comp->MailCC;
	CheckThese[2] = comp->MailBCC;

	/* go through all addresses */
	for(i=0; i<3; i++)
	{
		if(CheckThese[i] == NULL) continue;		// skip empty fields
		DB(KPrintf("SetDefaultSecurity(): checking address field: '%s'\n",CheckThese[i])); 
		// copy string as strtok() will modify it
		if(buf = strdup(CheckThese[i]))
		{
		int hits,currsec;
		STRPTR s;
		struct MUIS_Listtree_TreeNode *tn;

			// loop through comma-separated addresses in string
			s = strtok(buf,",");
			while(s)
			{
				DB(KPrintf("SetDefaultSecurity(): looking for user '%s'\n",s)); 
				AB_SearchEntry(MUIV_Lt_GetEntry_ListNode_Root, s, ASM_REALNAME|ASM_ADDRESS|ASM_COMPLETE, &hits, &tn);
				if(hits == 0) currsec = 0;		// entry not in address book -> no security
				else currsec = ((struct ABEntry*)(tn->tn_User))->DefSecurity;	// else get default
				if(currsec != Security)
				{
					if(FirstAddr)		// first address' setting is always used
					{
						FirstAddr = FALSE;
						Security = currsec;
					} else				// conflict: two addresses have different defaults
					{
						DB(KPrintf("SetDefaultSecurity(): conflicting security for address '%s': %ld\n",s,currsec));
						Security = 0;	// disable
					}
				}
				s = strtok(NULL,",");
			}
			free(buf);
		}
	}
	comp->Security = Security;		// FIXME: consider user's manual changes before setting defaults!
}

///
/// WR_ComposePGP
//  Creates a signed and/or encrypted PGP/MIME message
LOCAL BOOL WR_ComposePGP(FILE *fh, struct Compose *comp, char *boundary)
{
   int sec = comp->Security;
   BOOL success = FALSE;
   struct WritePart pgppart, *firstpart = comp->FirstPart;
   char *ids = AllocStrBuf(SIZE_DEFAULT), pgpfile[SIZE_PATHFILE], options[SIZE_LARGE];
   struct TempFile *tf, *tf2;

   pgppart.Filename = pgpfile; *pgpfile = 0;
   pgppart.EncType = ENC_NONE;
   if (sec & 2)
   {
      if (comp->MailTo) ids = WR_GetPGPIds(comp->MailTo, ids);
      if (comp->MailCC) ids = WR_GetPGPIds(comp->MailCC, ids);
      if (comp->MailBCC) ids = WR_GetPGPIds(comp->MailBCC, ids);
      if (C->EncryptToSelf && *C->MyPGPID)
      {
         if (G->PGPVersion == 5) ids = StrBufCat(ids, "-r ");
         ids = StrBufCat(ids, C->MyPGPID);
      }
   }
   tf2 = OpenTempFile(NULL);
   if (tf = OpenTempFile("w"))
   {
      WriteContentTypeAndEncoding(tf->FP, firstpart);
      fputc('\n', tf->FP);
      EncodePart(tf->FP, firstpart);
      fclose(tf->FP); tf->FP = NULL;
      ConvertCRLF(tf->Filename, tf2->Filename, TRUE);
      CloseTempFile(tf);
      sprintf(pgpfile, "%s.asc", tf2->Filename);
      if (sec & 1) PGPGetPassPhrase();
      switch (sec)
      {
         case 1: /* sign */
            fprintf(fh, "Content-type: multipart/signed; boundary=\"%s\"; micalc=pgp-md5; protocol=\"application/pgp-signature\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
            WriteContentTypeAndEncoding(fh, firstpart);
            fputc('\n', fh);
            EncodePart(fh, firstpart);
            fprintf(fh, "\n--%s\nContent-Type: application/pgp-signature\n\n", boundary);
            sprintf(options, (G->PGPVersion == 5) ? "-ab %s +batchmode=1 +force" : "-sab %s +bat +f", tf2->Filename);
            if (*C->MyPGPID) { strcat(options, " -u "); strcat(options, C->MyPGPID); }
            if (!PGPCommand((G->PGPVersion == 5) ? "pgps" : "pgp", options, 0)) success = TRUE;
            break;
         case 2: /* encrypt */
            fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
            fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);
            sprintf(options, (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force" : "-ea %s %s +bat +f", tf2->Filename, ids);
            if (!PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0)) success = TRUE;
            break;
         case 3: /* sign+encrypt */
            fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
            fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);
            sprintf(options, (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force -s" : "-sea %s %s +bat +f", tf2->Filename, ids);
            if (*C->MyPGPID) { strcat(options, " -u "); strcat(options, C->MyPGPID); }
            if (!PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0)) success = TRUE;
            break;
      }
      if (success) EncodePart(fh, &pgppart);
   }
   CloseTempFile(tf2);
   if (*pgpfile) remove(pgpfile);
   fprintf(fh, "\n--%s--\n\n", boundary);
   FreeStrBuf(ids);
   PGPClearPassPhrase(!success);
   return success;
}

///
/// WR_ComposeMulti
//  Assembles a multipart message
LOCAL void WR_ComposeMulti(FILE *fh, struct Compose *comp, char *boundary)
{
   struct WritePart *p;
   fprintf(fh, "Content-type: multipart/mixed; boundary=\"%s\"\n\n", boundary);
   fputs(MIMEwarn, fh);
   for (p = comp->FirstPart; p; p = p->Next)
   {
      fprintf(fh, "\n--%s\n", boundary);
      WriteContentTypeAndEncoding(fh, p);
      if (comp->Security == 4) WR_Anonymize(fh, comp->MailTo);
      fputs("\n", fh);
      EncodePart(fh, p);
   }
   fprintf(fh, "\n--%s--\n\n", boundary);
}

///
/// WriteOutMessage
//  Outputs header and body of a new message
BOOL WriteOutMessage(struct Compose *comp)
{
BOOL success=FALSE;
struct TempFile *tf=NULL;
FILE *fh = comp->FH;
struct WritePart *firstpart = comp->FirstPart;
char boundary[SIZE_DEFAULT], options[SIZE_DEFAULT], *rcptto;
   
   if (comp->Mode == NEW_BOUNCE)
   {
      if (comp->DelSend) EmitHeader(fh, "X-YAM-Options", "delsent");
      return WR_Bounce(fh, comp);
   }
   if (comp->Mode == NEW_SAVEDEC) if (!WR_SaveDec(fh, comp)) return FALSE; else goto mimebody;
   if (!firstpart) return FALSE;

	// encrypted multipart message requested?
   if (firstpart->Next && comp->Security >= 1 && comp->Security <= 3)
   {
	struct Compose tcomp;
	FILE *tfh;

			if((tf = OpenTempFile(NULL)) && (tfh = fopen(tf->Filename,"w")))
			{
				memcpy(&tcomp,comp,sizeof(tcomp));	// clone struct Compose
				tcomp.FH = tfh;							// set new filehandle
				tcomp.Security = 0;						// clear security field

				// clear a few other fields to avoid redundancies
				tcomp.MailCC = tcomp.MailBCC = tcomp.ExtHeader = NULL;
				tcomp.Receipt = tcomp.Importance = 0;
				tcomp.DelSend = tcomp.UserInfo = FALSE;

				if(WriteOutMessage(&tcomp))			// recurse!
				{
				struct WritePart *tpart = comp->FirstPart; // save parts list so we're able to recover from a calloc() error

					// replace with single new part
   				if(comp->FirstPart = (struct WritePart *)calloc(1,sizeof(struct WritePart)))
					{
						comp->FirstPart->EncType = tpart->EncType;			// reuse encoding
	   				FreePartsList(tpart);										// free old parts list
						comp->FirstPart->ContentType = "message/rfc822";	// the only part is an email message
						comp->FirstPart->Filename = tf->Filename;				// set filename to tempfile
						comp->Signature = 0;											// only use sig in enclosed mail
					} else
					{
						// no errormsg here - the window probably won't open anyway...
						DisplayBeep(NULL);
						comp->FirstPart = tpart;			// just restore old parts list
						comp->Security = 0;					// switch off security
						// we'll most likely get more errors further down :(
					}
				} else
				{
//					ER_NewError(GetStr(MSG_ER_PGPMultipart),NULL,NULL);		// gotta define this!
					ER_NewError("Error while creating multipart PGP message. Encryption/signing disabled!",NULL,NULL);
					comp->Security = 0;
				}
				fclose(tfh);
			} else
			{
//				ER_NewError(GetStr(MSG_ER_PGPMultipart),NULL,NULL);
				ER_NewError("Error while creating multipart PGP message. Encryption/signing disabled!",NULL,NULL);
				comp->Security = 0;
			}
   }
   *options = 0;
   if (comp->DelSend) strcat(options, ",delsent");
   if (comp->Security) sprintf(&options[strlen(options)], ",%s", SecCodes[comp->Security]);
   if (comp->Signature) sprintf(&options[strlen(options)], ",sigfile%ld", comp->Signature-1);
   if (*options) EmitHeader(fh, "X-YAM-Options", &options[1]);
   EmitHeader(fh, "From", comp->From ? comp->From : BuildAddrName(C->EmailAddress, C->RealName));
   if (comp->ReplyTo) EmitHeader(fh, "Reply-To", comp->ReplyTo);
   if (comp->MailTo) EmitRcptHeader(fh, "To", comp->Security == 4 ? C->ReMailer : comp->MailTo);
   if (comp->MailCC) EmitRcptHeader(fh, "CC", comp->MailCC);
   if (comp->MailBCC) EmitRcptHeader(fh, "BCC", comp->MailBCC);
   EmitHeader(fh, "Date", GetDateTime());
   fprintf(fh, "Message-ID: <%s>\n", NewID(True));
   if (comp->IRTMsgID) EmitHeader(fh, "In-Reply-To", comp->IRTMsgID);
   rcptto = comp->ReplyTo ? comp->ReplyTo : (comp->From ? comp->From : C->EmailAddress);
   if (comp->Receipt & 1) EmitHeader(fh, "Return-Receipt-To", rcptto);
   if (comp->Receipt & 2) EmitHeader(fh, "Disposition-Notification-To", rcptto);
   if (comp->Importance) EmitHeader(fh, "Importance", comp->Importance == 1 ? "High" : "Low");
   fprintf(fh, "X-Mailer: YAM %s AmigaOS E-Mail Client (c) 1995-2000 by Marcel Beck  http://www.yam.ch\n", __VERSION__);
   if (comp->UserInfo) WR_WriteUserInfo(fh);
   if (*C->Organization) EmitHeader(fh, "Organization", C->Organization);
   if (*comp->Subject) EmitHeader(fh, "Subject", comp->Subject);
   if (comp->ExtHeader) WR_EmitExtHeader(fh, comp);
mimebody:
   fputs("MIME-Version: 1.0\n", fh);
   sprintf(boundary, "BOUNDARY.%s", NewID(False));
   if (comp->ReportType > 0)
   {
		WR_ComposeReport(fh, comp, boundary);
		success = TRUE;
	} else if (comp->Security >= 1 && comp->Security <= 3)
	{
		success = WR_ComposePGP(fh, comp, boundary);
	} else if (firstpart->Next)
	{
		WR_ComposeMulti(fh, comp, boundary);
		success = TRUE;
	} else
   {
      WriteContentTypeAndEncoding(fh, firstpart);
      if (comp->Security == 4 && comp->OldSecurity != 4) WR_Anonymize(fh, comp->MailTo);
      fputs("\n", fh);
      EncodePart(fh, firstpart);
		success = TRUE;
   }
	CloseTempFile(tf);
	DBpr("WriteOutMessage() done\n");
	return success;
}

///
/// WR_AutoSaveFile
//  Returns filename of the auto-save file
char *WR_AutoSaveFile(int winnr)
{
   static char fname[SIZE_PATHFILE];
   strmfp(fname, G->ProgDir, ".autosave");
   strcat(fname, itoa(winnr));
   strcat(fname, ".txt");
   return fname;
}
///

/*** Buttons ***/
/// WR_NewMail
//  Validates write window options and generates a new message
void WR_NewMail(int mode, int winnum)
{
   struct Compose comp;
   char *addr, *er;
   static struct Mail mail;
   struct Mail *new = NULL, *mlist[3];
   int i, att = 0;
   struct WR_ClassData *wr = G->WR[winnum];
   struct WR_GUIData *gui = &wr->GUI;
   struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);

   set(gui->RG_PAGE, MUIA_Group_ActivePage, 0);
   clear(&mail, sizeof(struct Mail));
   clear(&comp, sizeof(struct Compose));
   mlist[0] = (struct Mail *)1; mlist[1] = NULL;
   get(gui->ST_TO, MUIA_String_Contents, &addr);
   er = GetStr(MSG_WR_ErrorNoRcpt);
   if (!*addr) if (MUI_Request(G->App, gui->WI, 0, NULL, GetStr(MSG_WR_NoRcptReqGad), er)) mode = WRITE_HOLD;
      else return;
   Busy(GetStr(MSG_BusyComposing), "", 0, 0);
   get(gui->ST_SUBJECT, MUIA_String_Contents, &comp.Subject);
   comp.Mode = wr->Mode;
   comp.OrigMail = wr->Mail;
   comp.OldSecurity = wr->OldSecurity;
   wr->ListEntry = NULL;
   if (*addr) if (!(comp.MailTo = WR_ExpandAddresses(winnum, addr, FALSE, FALSE))) goto skip;
   if (wr->Mode != NEW_BOUNCE)
   {
      get(gui->ST_CC, MUIA_String_Contents, &addr);
      if (*addr) if (!(comp.MailCC = WR_ExpandAddresses(winnum, addr, FALSE, FALSE))) goto skip;
      get(gui->ST_BCC, MUIA_String_Contents, &addr);
      if (*addr) if (!(comp.MailBCC = WR_ExpandAddresses(winnum, addr, FALSE, FALSE))) goto skip;
      get(gui->ST_FROM, MUIA_String_Contents, &addr);
      if (*addr) if (!(comp.From = WR_ExpandAddresses(winnum, addr, FALSE, TRUE))) goto skip;
      get(gui->ST_REPLYTO, MUIA_String_Contents, &addr);
      if (*addr) if (!(comp.ReplyTo = WR_ExpandAddresses(winnum, addr, FALSE, TRUE))) goto skip;
      get(gui->ST_EXTHEADER, MUIA_String_Contents, &comp.ExtHeader);
      if (wr->ListEntry)
      {
         if (wr->ListEntry->Address[0]) comp.ReplyTo = StrBufCpy(NULL, wr->ListEntry->Address);
         if (wr->ListEntry->RealName[0]) comp.From = StrBufCpy(comp.From, BuildAddrName(C->EmailAddress, wr->ListEntry->RealName));
      }
      if (wr->MsgID[0]) comp.IRTMsgID = wr->MsgID;
      comp.Importance = 1-GetMUICycle(gui->CY_IMPORTANCE);
      if (GetMUICheck(gui->CH_RECEIPT)) comp.Receipt |= 1;
      if (GetMUICheck(gui->CH_DISPNOTI)) comp.Receipt |= 2;
      comp.Signature = GetMUIRadio(gui->RA_SIGNATURE);
      comp.Security = GetMUIRadio(gui->RA_SECURITY);
      comp.DelSend = GetMUICheck(gui->CH_DELSEND);
      comp.UserInfo = GetMUICheck(gui->CH_ADDINFO);
      get(G->WR[winnum]->GUI.LV_ATTACH, MUIA_List_Entries, &att);
      EditorToFile(gui->TE_EDIT, G->WR_Filename[winnum], G->TTout);
      comp.FirstPart = BuildPartsList(winnum);
      comp.FirstPart->TTable = G->TTout;
   }

//	SetDefaultSecurity(&comp);

   if (wr->Mode == NEW_EDIT)
   {
      struct Mail *edmail = wr->Mail;
      outfolder = edmail->Folder;
      if (MailExists(edmail, NULL))
      {
         comp.FH = fopen(GetMailFile(NULL, outfolder, edmail), "w");
         strcpy(mail.MailFile, edmail->MailFile);
      }
      else
      {
         wr->Mode = NEW_NEW;
         comp.FH = fopen(MA_NewMailFile(outfolder, mail.MailFile, 0), "w");
      }
   }
   else comp.FH = fopen(MA_NewMailFile(outfolder, mail.MailFile, 0), "w");
   if (comp.FH)
   {
      struct MailInfo *mi;
      struct ExtendedMail *email;
      int stat = mode == WRITE_HOLD ? STATUS_HLD : STATUS_WFS;
      BOOL done = WriteOutMessage(&comp);
      fclose(comp.FH);
      if (!done) { DeleteFile(GetMailFile(NULL, outfolder, &mail)); goto skip; }
      if (wr->Mode != NEW_BOUNCE) EndNotify(&G->WR_NRequest[winnum]);
      if (email = MA_ExamineMail(outfolder, mail.MailFile, Status[stat], FALSE))
      {
         new = AddMailToList((struct Mail *)email, outfolder);
         MA_FreeEMailStruct(email);
         if (FO_GetCurrentFolder() == outfolder) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_InsertSingle, new, MUIV_NList_Insert_Sorted);
         MA_SetMailStatus(new, stat);
         if (wr->Mode == NEW_EDIT)
         {
            mi = GetMailInfo(wr->Mail);
            if (mi->Display) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Remove, mi->Pos);
            RemoveMailFromList(wr->Mail);
            wr->Mail = new;
         }
      }
      if (wr->Mode != NEW_NEW)
      {
         struct Mail *m, **ml = wr->MList ? wr->MList : mlist;
         mlist[2] = wr->Mail;
         for (i = 0; i < (int)ml[0]; i++)
         {
            m = ml[i+2];
            if (!Virtual(m)) if (m->Folder->Type != FT_OUTGOING && m->Folder->Type != FT_SENT)
            {
               if (m->Status == STATUS_NEW || m->Status == STATUS_UNR)
               {
                  int mdntype = wr->Mode == NEW_REPLY ? MDN_DISP : MDN_PROC;
                  if (winnum == 2) mdntype |= MDN_AUTOACT;
                  RE_DoMDN(mdntype, m);
               }
               switch (wr->Mode)
               {
                  case NEW_REPLY:  MA_SetMailStatus(m, STATUS_RPD);
                                   DisplayStatistics(m->Folder); break;
                  case NEW_FORWARD:
                  case NEW_BOUNCE: MA_SetMailStatus(m, STATUS_FWD);
                                   DisplayStatistics(m->Folder); break;
               }
            }
         }
      }
      switch (wr->Mode)
      {
         case NEW_NEW:     AppendLog(10, GetStr(MSG_LOG_Creating),   AddrName(new->To), new->Subject, (void *)att, ""); break;
         case NEW_REPLY:   AppendLog(11, GetStr(MSG_LOG_Replying),   AddrName(wr->MList[2]->From), wr->MList[2]->Subject, "", ""); break;
         case NEW_FORWARD: AppendLog(12, GetStr(MSG_LOG_Forwarding), AddrName(wr->MList[2]->From), wr->MList[2]->Subject, AddrName(new->To), ""); break;
         case NEW_BOUNCE:  AppendLog(13, GetStr(MSG_LOG_Bouncing),   AddrName(wr->Mail->From), wr->Mail->Subject, AddrName(new->To), ""); break;
         case NEW_EDIT:    AppendLog(14, GetStr(MSG_LOG_Editing),    AddrName(new->From), AddrName(new->To), new->Subject, ""); break;
      }
      MA_StartMacro(MACRO_POSTWRITE, itoa(winnum));
   }
   else ER_NewError(GetStr(MSG_ER_CreateMailError), NULL, NULL);
   FreePartsList(comp.FirstPart);
   if (wr->MList) free(wr->MList);
   if (mode == WRITE_SEND && new && !G->TR)
   {
      set(gui->WI, MUIA_Window_Open, FALSE);
      mlist[2] = new; MA_SendMList(mlist);
   }
   DeleteFile(WR_AutoSaveFile(winnum));
   DisposeModulePush(&G->WR[winnum]);
skip:
   FreeStrBuf(comp.MailTo);
   FreeStrBuf(comp.MailCC);
   FreeStrBuf(comp.MailBCC);
   FreeStrBuf(comp.From);
   FreeStrBuf(comp.ReplyTo);
   DisplayStatistics(outfolder);
   BusyEnd;
}

SAVEDS ASM void WR_NewMailFunc(REG(a1) int *arg)
{
   WR_NewMail(arg[0], arg[1]);
}
MakeHook(WR_NewMailHook, WR_NewMailFunc);

///
/// WR_Cleanup
//  Terminates file notification and removes temporary files
void WR_Cleanup(int winnum)
{
   int i;
   struct Attach *att;
   if (G->WR[winnum]->Mode != NEW_BOUNCE)
   {
      EndNotify(&G->WR_NRequest[winnum]);
      DeleteFile(G->WR_Filename[winnum]);
      for (i = 0; ; i++)
      {
         DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_List_GetEntry, i, &att);
         if (!att) break;
         if (att->IsTemp) DeleteFile(att->FilePath);
      }
   }
}

///
/// WR_CancelFunc
//  User clicked the Cancel button
SAVEDS ASM void WR_CancelFunc(REG(a1) int *arg)
{
   int haschanged, winnum = *arg;
   if (G->WR[winnum]->Mode != NEW_BOUNCE)
   {
      if (winnum < 2)
      {
         get(G->WR[winnum]->GUI.TE_EDIT, MUIA_TextEditor_HasChanged, &haschanged);
         if (haschanged)
            switch (MUI_Request(G->App, G->WR[winnum]->GUI.WI, 0, NULL, GetStr(MSG_WR_DiscardChangesGad), GetStr(MSG_WR_DiscardChanges)))
            {
               case 0: return;
               case 1: WR_NewMail(WRITE_QUEUE, winnum);
                       return;
               case 2: break;
            }
      }
      WR_Cleanup(winnum);
   }
   DisposeModulePush(&G->WR[winnum]);
}
MakeHook(WR_CancelHook, WR_CancelFunc);

///
/// WR_SaveAsFunc
//  Saves contents of internal editor to a file
SAVEDS ASM void WR_SaveAsFunc(REG(a1) int *arg)
{
   int winnum = *arg;
   set(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage, 0);
   if (ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, GetStr(MSG_WR_SaveTextAs), 1, C->AttachDir, ""))
   {
      char filename[SIZE_PATHFILE];
      strmfp(filename, G->ASLReq[ASL_ATTACH]->fr_Drawer, G->ASLReq[ASL_ATTACH]->fr_File);
      EditorToFile(G->WR[winnum]->GUI.TE_EDIT, G->WR_Filename[winnum], NULL);
      if (!CopyFile(filename, NULL, G->WR_Filename[winnum], NULL))
         ER_NewError(GetStr(MSG_ER_CantCreateFile), filename, NULL);
   }
}
MakeHook(WR_SaveAsHook, WR_SaveAsFunc);

///
/// WR_Edit
//  Launches external editor with message text
SAVEDS ASM void WR_Edit(REG(a1) int *arg)
{
   int winnum = *arg;
   if (*(C->Editor))
   {
      char buffer[SIZE_COMMAND+SIZE_PATHFILE];
      set(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage, 0);
      EditorToFile(G->WR[winnum]->GUI.TE_EDIT, G->WR_Filename[winnum], NULL);
      sprintf(buffer,"%s \"%s\"", C->Editor, G->WR_Filename[winnum]);
      ExecuteCommand(buffer, TRUE, OUT_NIL);
   }
}
MakeHook(WR_EditHook, WR_Edit);

///
/// WR_AddFileFunc
//  Adds one or more files to the attachment list
SAVEDS ASM void WR_AddFileFunc(REG(a1) int *arg)
{
   int i, winnum = *arg;
   char filename[SIZE_PATHFILE];
   struct FileRequester *ar = G->ASLReq[ASL_ATTACH];

   if (ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, GetStr(MSG_WR_AddFile), 2, C->AttachDir, ""))
      if (!ar->fr_NumArgs)
      {
         strmfp(filename, G->ASLReq[ASL_ATTACH]->fr_Drawer, G->ASLReq[ASL_ATTACH]->fr_File);
         WR_AddFileToList(winnum, filename, NULL, FALSE);
      }
      else for (i = 0; i < ar->fr_NumArgs; i++)
      {
         strmfp(filename, G->ASLReq[ASL_ATTACH]->fr_Drawer, G->ASLReq[ASL_ATTACH]->fr_ArgList[i].wa_Name);
         WR_AddFileToList(winnum, filename, NULL, FALSE);
      }
}
MakeHook(WR_AddFileHook, WR_AddFileFunc);

///
/// WR_AddArchiveFunc
//  Creates an archive of one or more files and adds it to the attachment list
SAVEDS ASM void WR_AddArchiveFunc(REG(a1) int *arg)
{
   int i, winnum = *arg;
   static char chr[2] = { 0,0 };
   char *src, *dst, filename[SIZE_PATHFILE], arcpath[SIZE_PATHFILE], arcname[SIZE_FILE];
   struct TempFile *tf = NULL;
   struct FileRequester *ar = G->ASLReq[ASL_ATTACH];
   BPTR olddir, filedir;

   if (ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, GetStr(MSG_WR_AddFile), 2, C->AttachDir, ""))
   {
      strsfn(ar->fr_ArgList[0].wa_Name, NULL, NULL, arcname, NULL);
      if (!*arcname) strsfn(ar->fr_ArgList[0].wa_Name, NULL, NULL, NULL, arcname);
      if (!StringRequest(arcname, SIZE_FILE, GetStr(MSG_WR_CreateArc), GetStr(MSG_WR_CreateArcReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), FALSE, G->WR[winnum]->GUI.WI)) return;
      strmfp(filename, C->TempDir, arcname);
      sprintf(arcpath, strchr(filename, ' ') ? "\"%s\"" : "%s", filename);
      if (strstr(C->PackerCommand, "%l")) if (tf = OpenTempFile("w"))
      {
         for (i = 0; i < ar->fr_NumArgs; i++)
         {
//            strmfp(filename, ar->fr_Drawer, ar->fr_ArgList[i].wa_Name);
//            fprintf(tf->FP, strchr(filename, ' ') ? "\"%s\"\n" : "%s\n", filename);
            fprintf(tf->FP, strchr(ar->fr_ArgList[i].wa_Name, ' ') ? "\"%s\"\n" : "%s\n", ar->fr_ArgList[i].wa_Name);
         }
         fclose(tf->FP); tf->FP = NULL;
      }
      dst = AllocStrBuf(SIZE_DEFAULT);
      for (src = C->PackerCommand; *src; src++)
         if (*src == '%') switch (*++src)
         {
            case '%': dst = StrBufCat(dst, "%"); break;
            case 'a': dst = StrBufCat(dst, arcpath); break;
            case 'l': dst = StrBufCat(dst, tf->Filename); break;
            case 'f': for (i = 0; i < ar->fr_NumArgs; i++)
                      {
//                         strcpy(filename, "\"");
//                         strmfp(&filename[1], ar->fr_Drawer, ar->fr_ArgList[i].wa_Name);
//                         strcat(filename, "\" ");
                         sprintf(filename, "\"%s\"", ar->fr_ArgList[i].wa_Name);
                         dst = StrBufCat(dst, filename);
                      }
                      break;
         }
         else
         {
            chr[0] = *src;
            dst = StrBufCat(dst, chr);
         }
      filedir = Lock(ar->fr_Drawer, ACCESS_READ); olddir = CurrentDir(filedir);
      ExecuteCommand(dst, FALSE, OUT_NIL);
      CurrentDir(olddir); UnLock(filedir);
      FreeStrBuf(dst);
      CloseTempFile(tf);
      strcpy(filename, arcpath);
      if (FileSize(filename) == -1)
      {
         sprintf(filename, "%s.lha", arcpath);
         if (FileSize(filename) == -1)
         {
            sprintf(filename, "%s.lzx", arcpath);
            if (FileSize(filename) == -1)
               sprintf(filename, "%s.zip", arcpath);
         }
      }
      WR_AddFileToList(winnum, filename, NULL, TRUE);
   }
}
MakeHook(WR_AddArchiveHook, WR_AddArchiveFunc);

///
/// WR_DisplayFile
//  Displays an attached file using a MIME viewer
SAVEDS ASM void WR_DisplayFile(REG(a1) int *arg)
{
   struct Attach *attach = NULL;

   DoMethod(G->WR[*arg]->GUI.LV_ATTACH, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &attach);
   if (attach) RE_DisplayMIME(attach->FilePath, attach->ContentType);
}
MakeHook(WR_DisplayFileHook, WR_DisplayFile);

///
/// WR_ChangeSignatureFunc
//  Changes the current signature
SAVEDS ASM void WR_ChangeSignatureFunc(REG(a1) int *arg)
{
   struct TempFile *tf;
   int signat = arg[0], winnum = arg[1];
   char buffer[SIZE_LINE];
   FILE *in, *out;

   if (tf = OpenTempFile(NULL))
   {
      EditorToFile(G->WR[winnum]->GUI.TE_EDIT, tf->Filename, NULL);
      if (in = fopen(tf->Filename, "r"))
      {
         if (out = fopen(G->WR_Filename[winnum], "w"))
         {
            while (fgets(buffer, SIZE_LINE, in))
               if (strcmp(buffer, "-- \n")) fputs(buffer, out); else break;
            if (signat) WR_WriteSignature(out, signat-1);
            fclose(out);
         }
         fclose(in);
      }
      CloseTempFile(tf);;
   }
}
MakeHook(WR_ChangeSignatureHook, WR_ChangeSignatureFunc);
///

/*** Menus ***/
/// WR_TransformText
//  Inserts or pastes text as plain, ROT13 or quoted
LOCAL char *WR_TransformText(char *source, int mode, char *qtext)
{
   FILE *fp;
   char *dest = NULL;
   int ch, i, size = FileSize(source), pos = 0, p = 0, qtextlen = strlen(qtext);

   if (size > 0)
   {
      size += SIZE_DEFAULT;
      if (mode == ED_INSQUOT || mode == ED_PASQUOT) size += size/20*qtextlen;
      if (dest = calloc(size, 1))
      {
         if (fp = fopen(source, "r"))
         {
            while ((ch = fgetc(fp)) != EOF)
            {
               if (!pos && (mode == ED_INSQUOT || mode == ED_PASQUOT))
               {
                  if (p+qtextlen > size-2) dest = realloc(dest,(size += SIZE_LARGE));
                  for (i = 0; i < qtextlen; i++) dest[p++] = qtext[i]; dest[p++] = ' ';
               }
               if (ch == '\n') pos = 0; else ++pos;
               if (mode == ED_INSROT13 || mode == ED_PASROT13)
               {
                  if (ch >= 'a' && ch <= 'z') if ((ch += 13) > 'z') ch -= 26;
                  if (ch >= 'A' && ch <= 'Z') if ((ch += 13) > 'Z') ch -= 26;
               }
               if (p > size-3) dest = realloc(dest,(size += SIZE_LARGE));
               dest[p++] = ch;
            }
            dest[p] = 0;
            fclose(fp);
         }
      }
   }
   return dest;
}

///
/// WR_InsertSeparator
//  Inserts a separator bar at the cursor position
SAVEDS ASM void WR_InsertSeparatorFunc(REG(a1) int *arg)
{
   APTR ed = G->WR[arg[1]]->GUI.TE_EDIT;
   set(ed, MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain);
   DoMethod(ed, MUIM_TextEditor_InsertText, arg[0] ? "\n\033c\033[s:18]\n" : "\n\033c\033[s:2]\n");
   set(ed, MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_EMail);
}
MakeHook(WR_InsertSeparatorHook, WR_InsertSeparatorFunc);

///
/// WR_EditorCmd
//  Inserts file or clipboard into editor
SAVEDS ASM void WR_EditorCmd(REG(a1) int *arg)
{
   int cmd = arg[0], winnum = arg[1];
   char *text, filename[SIZE_PATHFILE];
   struct TempFile *tf;
   struct WR_ClassData *wr = G->WR[winnum];

   if (cmd == ED_INSERT || cmd == ED_INSQUOT || cmd == ED_INSROT13 || cmd == ED_OPEN)
   {
      if (!ReqFile(ASL_ATTACH, wr->GUI.WI, GetStr(MSG_WR_InsertFile), 0, C->AttachDir, "")) return;
      strmfp(filename, G->ASLReq[ASL_ATTACH]->fr_Drawer, G->ASLReq[ASL_ATTACH]->fr_File);
      text = WR_TransformText(filename, cmd, wr->QuoteText);
   }
   else
   {
      if (!(tf = OpenTempFile("w"))) return;
      DumpClipboard(tf->FP);
      fclose(tf->FP); tf->FP = NULL;
      text = WR_TransformText(tf->Filename, cmd, wr->QuoteText);
      CloseTempFile(tf);
   }
   if (text)
   {
      if (cmd == ED_OPEN) DoMethod(wr->GUI.TE_EDIT, MUIM_TextEditor_ClearText);
      DoMethod(wr->GUI.TE_EDIT, MUIM_TextEditor_InsertText, text);
      free(text);
   }
}
MakeHook(WR_EditorCmdHook, WR_EditorCmd);

///
/// WR_AddClipboardFunc
//  Adds contents of clipboard as attachment
SAVEDS ASM void WR_AddClipboardFunc(REG(a1) int *arg)
{
   int winnum = *arg;
   struct TempFile *tf = OpenTempFile("w");
   if (DumpClipboard(tf->FP))
   {
      fclose(tf->FP); tf->FP = NULL;
      WR_AddFileToList(winnum, tf->Filename, "clipboard.text", TRUE);
      free(tf);
      return;
   }
   CloseTempFile(tf);
}
MakeHook(WR_AddClipboardHook, WR_AddClipboardFunc);

///
/// WR_AddPGPKeyFunc
//  Adds ASCII version of user's public PGP key as attachment
SAVEDS ASM void WR_AddPGPKeyFunc(REG(a1) int *arg)
{
   int winnum = *arg;
   char *myid = *C->MyPGPID ? C->MyPGPID : C->EmailAddress;
   char options[SIZE_LARGE], *fname = "T:PubKey.asc";
   sprintf(options, (G->PGPVersion == 5) ? "-x %s -o %s +force +batchmode=1" : "-kxa %s %s +f +bat", myid, fname);
   if (!PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0)) if (FileSize(fname) > 0)
   {
      WR_AddFileToList(winnum, fname, NULL, TRUE);
      setstring(G->WR[winnum]->GUI.ST_CTYPE, "application/pgp-keys");
   }
   else ER_NewError(GetStr(MSG_ER_ErrorAppendKey), myid, NULL);
}
MakeHook(WR_AddPGPKeyHook, WR_AddPGPKeyFunc);
///

/*** Open ***/
/// WR_Open
//  Initializes a write window
int WR_Open(int winnum, BOOL bounce)
{
   if (winnum == -1) if (G->WR[winnum = 0]) if (G->WR[winnum = 1]) return -1;
   G->WR[winnum] = bounce ? WR_NewBounce(winnum) : WR_New(winnum);
   if (!G->WR[winnum]) return -1;
   if (!bounce)
   {
      struct WR_GUIData *gui = &G->WR[winnum]->GUI;
      setstring(gui->ST_FROM, BuildAddrName(C->EmailAddress, C->RealName));
      setstring(gui->ST_REPLYTO, C->ReplyTo);
      setstring(gui->ST_EXTHEADER, C->ExtraHeaders);
      setcheckmark(gui->CH_DELSEND, !C->SaveSent);
      setcheckmark(gui->CH_ADDINFO, C->AddMyInfo);
   }
   MA_StartMacro(MACRO_PREWRITE, itoa(winnum));
   return winnum;
}

///
/// WR_SetupOldMail
//  When editing a message, sets write window options to old values
void WR_SetupOldMail(int winnum)
{
   static struct Attach attach;
   struct Part *part = G->RE[4]->FirstPart->Next;

   if (part) for (part = part->Next; part; part = part->Next)
      if (stricmp(part->ContentType, "application/pgp-signature"))
      {
         Busy(GetStr(MSG_BusyDecSaving), "", 0, 0);
         RE_DecodePart(part);
         clear(&attach, sizeof(struct Attach));
         attach.Size = part->Size;
         attach.IsMIME = part->EncodingCode != ENC_UUE;
         attach.IsTemp = TRUE;
         if (part->Name) stccpy(attach.Name, part->Name, SIZE_FILE);
         strcpy(attach.FilePath, part->Filename);
         *part->Filename = 0;
         stccpy(attach.ContentType, part->ContentType, SIZE_CTYPE);
         stccpy(attach.Description, part->Description, SIZE_DEFAULT);
         DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_List_InsertSingle, &attach, MUIV_List_Insert_Bottom);
         BusyEnd;
      }
}

///
/// WR_UpdateTitleFunc
//  Shows cursor coordinates
SAVEDS ASM void WR_UpdateWTitleFunc(REG(a1) int *arg)
{
   struct WR_ClassData *wr = G->WR[*arg];
   APTR ed = wr->GUI.TE_EDIT;

   sprintf(wr->WTitle, "%03ld\n%03ld", GetMUI(ed,MUIA_TextEditor_CursorY)+1, GetMUI(ed,MUIA_TextEditor_CursorX)+1);
   set(wr->GUI.TX_POSI, MUIA_Text_Contents, wr->WTitle);
}
MakeHook(WR_UpdateWTitleHook,WR_UpdateWTitleFunc);
///

/*** Hooks ***/
/// WR_AppFunc
//  Handles Drag&Drop
void WR_App(int winnum, struct AppMessage *amsg)
{
   struct WBArg *ap;
   int i, mode;
   char buf[SIZE_PATHFILE];

   get(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage, &mode);
   for (i = 0; i < amsg->am_NumArgs; i++)
   {
      ap = &amsg->am_ArgList[i];
      NameFromLock(ap->wa_Lock, buf, SIZE_PATHFILE);
      AddPart(buf, ap->wa_Name, SIZE_PATHFILE);
      if (!mode)
      {
         FILE *fh;
         int len, j, notascii = 0;
         char buffer[SIZE_LARGE];
         if (fh = fopen(buf, "r"))
         {
            len = fread(buffer, 1, SIZE_LARGE-1, fh);
            buffer[len] = 0;
            fclose(fh);
            for (j = 0; j < len; j++) if ((int)buffer[j] < 32 || (int)buffer[j] > 127) if (buffer[j] != '\t' && buffer[j] != '\n') notascii++;
            if (notascii) if (len/notascii <= 16) mode = 1;
         }
      }
      if (!mode)
      {
         char *text = WR_TransformText(buf, ED_INSERT, "");
         if (text) DoMethod(G->WR[winnum]->GUI.TE_EDIT, MUIM_TextEditor_InsertText, text);
         free(text);
      }
      else WR_AddFileToList(winnum, buf, NULL, FALSE);
   }
}

SAVEDS ASM LONG WR_AppFunc(REG(a1) ULONG *arg)
{
   WR_App((int)arg[1],  (struct AppMessage *)arg[0]);
   return 0;
}
MakeHook(WR_AppHook, WR_AppFunc);

///
/// WR_LV_ConFunc
//  Attachment listview construct hook
SAVEDS ASM struct Attach *WR_LV_ConFunc(REG(a1) struct Attach *attach)
{
   struct Attach *entry = malloc(sizeof(struct Attach));
   *entry = *attach;
   return entry;
}
MakeHook(WR_LV_ConFuncHook, WR_LV_ConFunc);

///
/// WR_LV_DspFunc
//  Attachment listview display hook
SAVEDS ASM long WR_LV_DspFunc(REG(a2) char **array, REG(a1) struct Attach *entry)
{
   static char dispsz[SIZE_SMALL];
   if (entry)
   {
      array[0] = entry->Name;
      sprintf(array[1] = dispsz, "%d", entry->Size);
      array[2] = DescribeCT(entry->ContentType);
      array[3] = entry->IsMIME ? "MIME" : "UU";
      array[4] = entry->Description;
   }
   else 
   {
      array[0] = GetStr(MSG_WR_TitleFile); 
      array[1] = GetStr(MSG_WR_TitleSize);
      array[2] = GetStr(MSG_WR_TitleContents);
      array[3] = GetStr(MSG_WR_TitleEncoding);
      array[4] = GetStr(MSG_WR_TitleDescription);
   }
   return 0;
}
MakeHook(WR_LV_DspFuncHook, WR_LV_DspFunc);
///

/*** GUI ***/
/// WR_SharedSetup
//  Common setup for write and bounce windows
LOCAL void WR_SharedSetup(struct WR_ClassData *data, int winnum)
{
   SetHelp(data->GUI.ST_TO      ,MSG_HELP_WR_ST_TO      );
   SetHelp(data->GUI.BT_QUEUE   ,MSG_HELP_WR_BT_QUEUE   );
   SetHelp(data->GUI.BT_HOLD    ,MSG_HELP_WR_BT_HOLD    );
   SetHelp(data->GUI.BT_SEND    ,MSG_HELP_WR_BT_SEND    );
   SetHelp(data->GUI.BT_CANCEL  ,MSG_HELP_WR_BT_CANCEL  );
   DoMethod(data->GUI.BT_HOLD    ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_HOLD,winnum);
   DoMethod(data->GUI.BT_QUEUE   ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_QUEUE,winnum);
   DoMethod(data->GUI.BT_SEND    ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_SEND,winnum);
   DoMethod(data->GUI.BT_CANCEL  ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_CancelHook,winnum);
   DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_CancelHook,winnum);
}

///
/// MakeAddressField
//  Creates a recipient field
LOCAL APTR MakeAddressField(APTR *string, char *label, APTR help, int abmode, int winnum, BOOL allowmulti)
{
   APTR obj, bt_ver, bt_adr;
   if (obj = HGroup,
      GroupSpacing(1),
      Child, *string = NewObject(CL_DDString->mcc_Class, NULL,
         MUIA_ControlChar, ShortCut(label),
         MUIA_UserData, allowmulti,
      End,
      Child, bt_ver = PopButton(MUII_ArrowLeft),
      Child, bt_adr = PopButton(MUII_PopUp),
   End)
   {
      set(bt_ver, MUIA_CycleChain, TRUE);
      set(bt_adr, MUIA_CycleChain, TRUE);
      SetHelp(*string,help);
      SetHelp(bt_ver, MSG_HELP_WR_BT_VER);
      SetHelp(bt_adr, MSG_HELP_WR_BT_ADR);
      DoMethod(*string,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,MUIV_Notify_Window,5,MUIM_CallHook,&WR_VerifyAutoHook,*string,winnum,!allowmulti);
      DoMethod(bt_ver,MUIM_Notify,MUIA_Pressed,FALSE,MUIV_Notify_Application,5,MUIM_CallHook,&WR_VerifyManualHook,*string,winnum,!allowmulti);
      DoMethod(bt_adr,MUIM_Notify,MUIA_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&AB_OpenHook,abmode,winnum);
   }
   return obj;
}

///
/// WR_New
//  Creates a write window
enum { WMEN_NEW=1,WMEN_OPEN,WMEN_INSFILE,WMEN_SAVEAS,WMEN_INSQUOT,
       WMEN_INSROT13,WMEN_EDIT,WMEN_CUT,WMEN_COPY,WMEN_PASTE,
       WMEN_PASQUOT,WMEN_PASROT13,WMEN_DICT,WMEN_STYLE1,WMEN_STYLE2,WMEN_STYLE3,
       WMEN_STYLE4,WMEN_EMOT0,WMEN_EMOT1,WMEN_EMOT2,WMEN_EMOT3,WMEN_UNDO,WMEN_REDO,
       WMEN_AUTOSP,WMEN_SEP0,WMEN_SEP1,WMEN_ADDFILE, WMEN_ADDCLIP, WMEN_ADDPGP,
       WMEN_DELSEND,WMEN_RECEIPT,WMEN_DISPNOTI,WMEN_ADDINFO,WMEN_IMPORT0,WMEN_IMPORT1,
       WMEN_IMPORT2,WMEN_SIGN0,WMEN_SIGN1,WMEN_SIGN2,WMEN_SIGN3,
       WMEN_SECUR0,WMEN_SECUR1,WMEN_SECUR2,WMEN_SECUR3,WMEN_SECUR4 };
extern long cmap[8];

struct WR_ClassData *WR_New(int winnum)
{
   struct WR_ClassData *data;

   if (data = calloc(1,sizeof(struct WR_ClassData)))
   {
      static char *rtitles[4], *encoding[3], *security[6], *priority[4], *signat[5];
      static char *emoticons[4] = { ":-)", ":-|", ":-(", ";-)" };
      APTR mi_copy, mi_cut, mi_redo, mi_undo, mi_bold, mi_italic, mi_underl, mi_color;
      APTR strip, mi_autospell, mi_delsend, mi_receipt, mi_dispnoti, mi_addinfo;
      APTR slider = ScrollbarObject, End;
      APTR tb_butt[13] = { MSG_WR_TBEditor,MSG_WR_TBInsert,MSG_Space,
                           MSG_WR_TBCut,MSG_WR_TBCopy,MSG_WR_TBPaste,MSG_WR_TBUndo,MSG_Space,
                           MSG_WR_TBBold,MSG_WR_TBItalic,MSG_WR_TBUnderlined,MSG_WR_TBColored,NULL };
      APTR tb_help[13] = { MSG_HELP_WR_BT_EDITOR,MSG_HELP_WR_BT_LOAD,NULL,
                           MSG_HELP_WR_BT_CUT,MSG_HELP_WR_BT_COPY,MSG_HELP_WR_BT_PASTE,MSG_HELP_WR_BT_UNDO,NULL,
                           MSG_HELP_WR_BT_BOLD,MSG_HELP_WR_BT_ITALIC,MSG_HELP_WR_BT_UNDERL,MSG_HELP_WR_BT_COLOR,NULL };
      int i, spell;
      for (i = 0; i < 13; i++) SetupToolbar(&(data->GUI.TB_TOOLBAR[i]), tb_butt[i]?(tb_butt[i]==MSG_Space?"":GetStr(tb_butt[i])):NULL, tb_help[i]?GetStr(tb_help[i]):NULL, (i>=8 && i<=11)?TDF_TOGGLE:0);
      rtitles[0] = GetStr(MSG_Message);
      rtitles[1] = GetStr(MSG_Attachments);
      rtitles[2] = GetStr(MSG_Options);
      rtitles[3] = NULL;
      encoding[0] = "Base64/QP";
      encoding[1] = "UUencode";
      encoding[2] = NULL;
      security[0] = GetStr(MSG_WR_SecNone);
      security[1] = GetStr(MSG_WR_SecSign);
      security[2] = GetStr(MSG_WR_SecEncrypt);
      security[3] = GetStr(MSG_WR_SecBoth);
      security[4] = GetStr(MSG_WR_SecAnon);
      security[5] = NULL;
      priority[0] = GetStr(MSG_WR_ImpHigh);
      priority[1] = GetStr(MSG_WR_ImpNormal);
      priority[2] = GetStr(MSG_WR_ImpLow);
      priority[3] = NULL;
      signat[0] = GetStr(MSG_WR_NoSig);
      signat[1] = GetStr(MSG_WR_DefSig);
      signat[2] = GetStr(MSG_WR_AltSig1);
      signat[3] = GetStr(MSG_WR_AltSig2);
      signat[4] = NULL;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_WR_WriteWT),
         MUIA_HelpNode, "WR_W",
         MUIA_Window_ID, MAKE_ID('W','R','I','T'),
         MUIA_Window_Menustrip, strip = MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_WR_Text),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_New), MUIA_Menuitem_Shortcut,"N", MUIA_UserData,WMEN_NEW, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,WMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_InsertAs),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Plain), MUIA_Menuitem_Shortcut,"P", MUIA_UserData,WMEN_INSFILE, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Quoted), MUIA_UserData,WMEN_INSQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_ROT13), MUIA_UserData,WMEN_INSROT13, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,(char)NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_SaveAs), MUIA_UserData,WMEN_SAVEAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,(char)NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_LaunchEd), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,WMEN_EDIT, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_WR_Edit),
               MUIA_Family_Child, mi_cut = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MCut), MUIA_Menuitem_Shortcut,"ramiga X", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,WMEN_CUT, End,
               MUIA_Family_Child, mi_copy = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MCopy), MUIA_Menuitem_Shortcut,"ramiga C", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,WMEN_COPY, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MPaste), MUIA_Menuitem_Shortcut,"ramiga V", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,WMEN_PASTE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_PasteAs),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Quoted), MUIA_Menuitem_Shortcut,"Q", MUIA_UserData,WMEN_PASQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_ROT13), MUIA_UserData,WMEN_PASROT13, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,(char)NM_BARLABEL, End,
               MUIA_Family_Child, mi_undo = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MUndo), MUIA_Menuitem_Shortcut,"ramiga Z", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,WMEN_UNDO, End,
               MUIA_Family_Child, mi_redo = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Redo), WMEN_REDO, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,(char)NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Dictionary), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,WMEN_DICT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Textstyle),
                  MUIA_Family_Child, mi_bold = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Bold), MUIA_Menuitem_Shortcut,"B", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE1, End,
                  MUIA_Family_Child, mi_italic = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Italic), MUIA_Menuitem_Shortcut,"I", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE2, End,
                  MUIA_Family_Child, mi_underl = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Underlined), MUIA_Menuitem_Shortcut,"U", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE3, End,
                  MUIA_Family_Child, mi_color = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Colored), MUIA_Menuitem_Shortcut,"A", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE4, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Separators),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_WR_Thin), MUIA_Menuitem_Shortcut,"-", MUIA_UserData,WMEN_SEP0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_WR_Thick), MUIA_Menuitem_Shortcut,"=", MUIA_UserData,WMEN_SEP1, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Emoticons),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Happy), MUIA_UserData,WMEN_EMOT0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Indifferent), MUIA_UserData,WMEN_EMOT1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Sad), MUIA_UserData,WMEN_EMOT2, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_Ironic), MUIA_UserData,WMEN_EMOT3, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,(char)NM_BARLABEL, End,
               MUIA_Family_Child, mi_autospell = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_WR_SpellCheck), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_AUTOSP, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_Attachments),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MAddFile), MUIA_Menuitem_Shortcut,"F", MUIA_UserData,WMEN_ADDFILE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_AddCB), MUIA_UserData,WMEN_ADDCLIP, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_AddKey), MUIA_UserData,WMEN_ADDPGP, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_Options),
               MUIA_Family_Child, mi_delsend = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MDelSend), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_DELSEND, End,
               MUIA_Family_Child, mi_receipt = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MReceipt), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_RECEIPT, End,
               MUIA_Family_Child, mi_dispnoti= MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MGetMDN),  MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_DISPNOTI, End,
               MUIA_Family_Child, mi_addinfo = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MAddInfo), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_ADDINFO, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_WR_MImportance),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[0], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x06, MUIA_UserData,WMEN_IMPORT0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[1], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x05, MUIA_Menuitem_Checked,TRUE, MUIA_UserData,WMEN_IMPORT1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[2], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x03, MUIA_UserData,WMEN_IMPORT2, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_CrdSignature),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[0], MUIA_Menuitem_Shortcut,"0", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0E, MUIA_Menuitem_Checked,!C->UseSignature, MUIA_UserData,WMEN_SIGN0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[1], MUIA_Menuitem_Shortcut,"7", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0D, MUIA_Menuitem_Checked,C->UseSignature, MUIA_UserData,WMEN_SIGN1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[2], MUIA_Menuitem_Shortcut,"8", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0B, MUIA_UserData,WMEN_SIGN2, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[3], MUIA_Menuitem_Shortcut,"9", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x07, MUIA_UserData,WMEN_SIGN3, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_CrdSecurity),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,security[0], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x1E, MUIA_Menuitem_Checked,TRUE, MUIA_UserData,WMEN_SECUR0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,security[1], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x1D, MUIA_UserData,WMEN_SECUR1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,security[2], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x1B, MUIA_UserData,WMEN_SECUR2, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,security[3], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x17, MUIA_UserData,WMEN_SECUR3, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,security[4], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0F, MUIA_UserData,WMEN_SECUR4, End,
               End,
            End,
         End,
         MUIA_Window_AppWindow, TRUE,
         WindowContents, VGroup,
            Child, data->GUI.RG_PAGE = RegisterGroup(rtitles),
               MUIA_CycleChain, 1,
               Child, VGroup, /* Message */
                  MUIA_HelpNode, "WR00",
                  Child, ColGroup(2),
                     Child, Label(GetStr(MSG_WR_To)),
                     Child, MakeAddressField(&data->GUI.ST_TO, GetStr(MSG_WR_To), MSG_HELP_WR_ST_TO, ABM_TO, winnum, TRUE),
                     Child, Label(GetStr(MSG_WR_Subject)),
                     Child, data->GUI.ST_SUBJECT = MakeString(SIZE_SUBJECT,GetStr(MSG_WR_Subject)),
                  End,
                  Child, (C->HideGUIElements & HIDE_TBAR) ?
                     (RectangleObject, MUIA_ShowMe, FALSE, End) :
                     (HGroup, GroupSpacing(0),
                        Child, HGroupV,
                           Child, data->GUI.TO_TOOLBAR = ToolbarObject,
                              MUIA_Toolbar_ImageType,      MUIV_Toolbar_ImageType_File,
                              MUIA_Toolbar_ImageNormal,    "PROGDIR:Icons/Write.toolbar",
                              MUIA_Toolbar_ImageGhost,     "PROGDIR:Icons/Write_G.toolbar",
                              MUIA_Toolbar_ImageSelect,    "PROGDIR:Icons/Write_S.toolbar",
                              MUIA_Toolbar_Description,    data->GUI.TB_TOOLBAR,
                              MUIA_Font,                   MUIV_Font_Tiny,
                              MUIA_ShortHelp, TRUE,
                           End,
                           Child, HSpace(0),
                        End,
                        Child, (C->HideGUIElements & HIDE_XY) ?
                           HSpace(1) :
                           (VCenter((data->GUI.TX_POSI = TextObject,
                              TextFrame,
                              MUIA_Weight    ,0,
                              MUIA_Text_Contents, "000\n000",
                              MUIA_Background,MUII_TextBack,
                              MUIA_Frame     ,MUIV_Frame_Text,
                              MUIA_Font      ,MUIV_Font_Tiny,
                           End))),
                     End),
                  Child, HGroup,
                     MUIA_HelpNode, "EDIT",
                     MUIA_Group_Spacing, 0,
                     Child, data->GUI.TE_EDIT = NewObject(CL_TextEditor->mcc_Class,NULL,
                        InputListFrame,
                        MUIA_TextEditor_Slider, slider,
                        MUIA_TextEditor_ColorMap, G->EdColMap,
                        MUIA_TextEditor_FixedFont, C->FixedFontEdit,
                        MUIA_TextEditor_WrapBorder, C->EdWrapMode == 1 ? C->EdWrapCol : 0,
                        MUIA_TextEditor_ExportWrap, C->EdWrapMode == 2 ? C->EdWrapCol : 0,
                        MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_EMail,
                        MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_EMail,
                        MUIA_CycleChain, TRUE,
                     End,
                     Child, slider,
                  End,
               End,
               Child, VGroup, /* Attachments */
                  MUIA_HelpNode, "WR01",
                  Child, ListviewObject,
                     MUIA_CycleChain, 1,
                     MUIA_Listview_DragType, 1,
                     MUIA_Listview_List,data->GUI.LV_ATTACH = NewObject(CL_AttachList->mcc_Class,NULL,
                        InputListFrame,
                        MUIA_List_DragSortable ,TRUE,
                        MUIA_List_Format       ,"D=8 BAR,P=\033r D=8 BAR,D=8 BAR,P=\033c D=8 BAR,",
                        MUIA_List_Title        ,TRUE,
                        MUIA_List_ConstructHook,&WR_LV_ConFuncHook,
                        MUIA_List_DestructHook ,&GeneralDesHook,
                        MUIA_List_DisplayHook  ,&WR_LV_DspFuncHook, 
                     End,
                  End,
                  Child, ColGroup(4),
                     Child, data->GUI.BT_ADD     = MakeButton(GetStr(MSG_WR_Add)),
                     Child, data->GUI.BT_ADDPACK = MakeButton(GetStr(MSG_WR_AddPack)),
                     Child, data->GUI.BT_DEL     = MakeButton(GetStr(MSG_Del)),
                     Child, data->GUI.BT_DISPLAY = MakeButton(GetStr(MSG_WR_Display)),
                  End,
                  Child, HGroup,
                     Child, data->GUI.RA_ENCODING = RadioObject,
                        GroupFrameT(GetStr(MSG_WR_Encoding)),
                        MUIA_Radio_Entries, encoding,
                        MUIA_CycleChain, 1,
                     End,
                     Child, ColGroup(2),
                        Child, Label2(GetStr(MSG_WR_ContentType)),
                        Child, PoplistObject,
                           MUIA_Popstring_String, data->GUI.ST_CTYPE = MakeString(SIZE_CTYPE,GetStr(MSG_WR_ContentType)),
                           MUIA_Popstring_Button, PopButton(MUII_PopUp),
                           MUIA_Poplist_Array   , ContType,
                        End,
                        Child, Label2(GetStr(MSG_WR_Description)),
                        Child, data->GUI.ST_DESC = MakeString(SIZE_DEFAULT,GetStr(MSG_WR_Description)),
                        Child, HSpace(0),
                     End,
                  End,
               End,
               Child, VGroup, /* Options */
                  MUIA_HelpNode, "WR02",
                  Child, ColGroup(2),
                     Child, Label(GetStr(MSG_WR_CopyTo)),
                     Child, MakeAddressField(&data->GUI.ST_CC, GetStr(MSG_WR_CopyTo), MSG_HELP_WR_ST_CC, ABM_CC, winnum, TRUE),
                     Child, Label(GetStr(MSG_WR_BlindCopyTo)),
                     Child, MakeAddressField(&data->GUI.ST_BCC, GetStr(MSG_WR_BlindCopyTo), MSG_HELP_WR_ST_BCC, ABM_BCC, winnum, TRUE),
                     Child, Label(GetStr(MSG_WR_From)),
                     Child, MakeAddressField(&data->GUI.ST_FROM, GetStr(MSG_WR_From), MSG_HELP_WR_ST_FROM, ABM_FROM, winnum, FALSE),
                     Child, Label(GetStr(MSG_WR_ReplyTo)),
                     Child, MakeAddressField(&data->GUI.ST_REPLYTO, GetStr(MSG_WR_ReplyTo), MSG_HELP_WR_ST_REPLYTO, ABM_REPLYTO, winnum, FALSE),
                     Child, Label(GetStr(MSG_WR_ExtraHeaders)),
                     Child, data->GUI.ST_EXTHEADER = MakeString(SIZE_LARGE,GetStr(MSG_WR_ExtraHeaders)),
                  End,
                  Child, HGroup,
                     Child, VGroup, GroupFrameT(GetStr(MSG_WR_SendOpt)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_DELSEND, GetStr(MSG_WR_DelSend)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_RECEIPT, GetStr(MSG_WR_Receipt)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_DISPNOTI, GetStr(MSG_WR_GetMDN)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_ADDINFO, GetStr(MSG_WR_AddInfo)),
                        Child, HGroup,
                           Child, Label(GetStr(MSG_WR_Importance)),
                           Child, data->GUI.CY_IMPORTANCE = MakeCycle(priority, GetStr(MSG_WR_Importance)),
                        End,
                     End,
                     Child, HSpace(0),
                     Child, data->GUI.RA_SIGNATURE = RadioObject, GroupFrameT(GetStr(MSG_WR_Signature)),
                        MUIA_Radio_Entries, signat,
                        MUIA_Radio_Active, C->UseSignature ? 1 : 0,
                        MUIA_CycleChain, 1,
                     End,
                     Child, HSpace(0),
                     Child, data->GUI.RA_SECURITY = RadioObject, GroupFrameT(GetStr(MSG_WR_Security)),
                        MUIA_Radio_Entries, security,
                        MUIA_CycleChain, 1,
                     End,
                  End,
               End,
            End,
            Child, ColGroup(4),
               Child, data->GUI.BT_SEND   = MakeButton(GetStr(MSG_WR_Send)),
               Child, data->GUI.BT_QUEUE  = MakeButton(GetStr(MSG_WR_ToQueue)),
               Child, data->GUI.BT_HOLD   = MakeButton(GetStr(MSG_WR_Hold)),
               Child, data->GUI.BT_CANCEL = MakeButton(GetStr(MSG_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         set(data->GUI.ST_TO, MUIA_UserData, data->GUI.ST_SUBJECT);
         set(data->GUI.ST_CC, MUIA_UserData, data->GUI.ST_BCC);
         set(data->GUI.ST_BCC, MUIA_UserData, data->GUI.ST_FROM);
         set(data->GUI.ST_FROM, MUIA_UserData, data->GUI.ST_REPLYTO);
         set(data->GUI.ST_REPLYTO, MUIA_UserData, data->GUI.ST_EXTHEADER);
         get(data->GUI.TE_EDIT, MUIA_TextEditor_TypeAndSpell, &spell);
         set(mi_autospell, MUIA_Menuitem_Checked, spell);
         set(data->GUI.CY_IMPORTANCE, MUIA_Cycle_Active, 1);
         DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.RA_ENCODING, data->GUI.ST_CTYPE, data->GUI.ST_DESC, data->GUI.BT_DEL, data->GUI.BT_DISPLAY, NULL);
         SetHelp(data->GUI.ST_SUBJECT   ,MSG_HELP_WR_ST_SUBJECT   );
         SetHelp(data->GUI.BT_ADD       ,MSG_HELP_WR_BT_ADD       );
         SetHelp(data->GUI.BT_ADDPACK   ,MSG_HELP_WR_BT_ADDPACK   );
         SetHelp(data->GUI.BT_DEL       ,MSG_HELP_WR_BT_DEL       );
         SetHelp(data->GUI.BT_DISPLAY   ,MSG_HELP_WR_BT_DISPLAY   );
         SetHelp(data->GUI.RA_ENCODING  ,MSG_HELP_WR_RA_ENCODING  );
         SetHelp(data->GUI.ST_CTYPE     ,MSG_HELP_WR_ST_CTYPE     );
         SetHelp(data->GUI.ST_DESC      ,MSG_HELP_WR_ST_DESC      );
         SetHelp(data->GUI.ST_EXTHEADER ,MSG_HELP_WR_ST_EXTHEADER );
         SetHelp(data->GUI.CH_DELSEND   ,MSG_HELP_WR_CH_DELSEND   );
         SetHelp(data->GUI.CH_RECEIPT   ,MSG_HELP_WR_CH_RECEIPT   );
         SetHelp(data->GUI.CH_DISPNOTI  ,MSG_HELP_WR_CH_DISPNOTI  );
         SetHelp(data->GUI.CH_ADDINFO   ,MSG_HELP_WR_CH_ADDINFO   );
         SetHelp(data->GUI.CY_IMPORTANCE,MSG_HELP_WR_CY_IMPORTANCE);
         SetHelp(data->GUI.RA_SIGNATURE ,MSG_HELP_WR_RA_SIGNATURE );
         SetHelp(data->GUI.RA_SECURITY  ,MSG_HELP_WR_RA_SECURITY  );
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_NEW      ,data->GUI.TE_EDIT      ,1,MUIM_TextEditor_ClearText);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_OPEN     ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_OPEN,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSFILE  ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSERT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSQUOT  ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSROT13 ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSROT13,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SAVEAS   ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_SaveAsHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_EDIT     ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_EditHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_CUT      ,data->GUI.TE_EDIT      ,3,MUIM_TextEditor_ARexxCmd,"Cut");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_COPY     ,data->GUI.TE_EDIT      ,3,MUIM_TextEditor_ARexxCmd,"Copy");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASTE    ,data->GUI.TE_EDIT      ,3,MUIM_TextEditor_ARexxCmd,"Paste");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASQUOT  ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_PASQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASROT13 ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_PASROT13,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_DICT     ,MUIV_Notify_Application,3,MUIM_CallHook   ,&DI_OpenHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_UNDO     ,data->GUI.TE_EDIT      ,3,MUIM_TextEditor_ARexxCmd,"Undo");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_REDO     ,data->GUI.TE_EDIT      ,3,MUIM_TextEditor_ARexxCmd,"Redo");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDFILE  ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddFileHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDCLIP  ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddClipboardHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDPGP   ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddPGPKeyHook,winnum);
         for (i = 0; i < 4; i++) DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_MenuAction,WMEN_EMOT0+i,data->GUI.TE_EDIT,2,MUIM_TextEditor_InsertText,emoticons[i]);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SEP0     ,data->GUI.TE_EDIT      ,4,MUIM_CallHook   ,&WR_InsertSeparatorHook,FALSE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SEP1     ,data->GUI.TE_EDIT      ,4,MUIM_CallHook   ,&WR_InsertSeparatorHook,TRUE,winnum);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_AppMessage          ,MUIV_EveryTime,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_AppHook,MUIV_TriggerValue,winnum);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_AreaMarked,MUIV_EveryTime,MUIV_Notify_Application,5,MUIM_MultiSet  ,MUIA_Menuitem_Enabled,MUIV_TriggerValue,mi_copy,mi_cut,NULL);
         if (data->GUI.TO_TOOLBAR)
         {
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_AreaMarked,MUIV_EveryTime,data->GUI.TO_TOOLBAR  ,6,MUIM_Toolbar_MultiSet,MUIV_Toolbar_Set_Ghosted, MUIV_NotTriggerValue,3,4,-1);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_UndoAvailable,MUIV_EveryTime,data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,6,MUIV_Toolbar_Set_Ghosted,MUIV_NotTriggerValue);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&WR_EditHook,winnum);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&WR_EditorCmdHook,ED_INSERT,winnum);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed,FALSE, data->GUI.TE_EDIT,2,MUIM_TextEditor_ARexxCmd, "CUT");
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 4, MUIV_Toolbar_Notify_Pressed,FALSE, data->GUI.TE_EDIT,2,MUIM_TextEditor_ARexxCmd, "COPY");
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 5, MUIV_Toolbar_Notify_Pressed,FALSE, data->GUI.TE_EDIT,2,MUIM_TextEditor_ARexxCmd, "PASTE");
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed,FALSE, data->GUI.TE_EDIT,2,MUIM_TextEditor_ARexxCmd, "UNDO");
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 8, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleBold,     MUIV_TriggerValue);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 9, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleItalic,   MUIV_TriggerValue);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleUnderline,MUIV_TriggerValue);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,TRUE,           data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_Pen,           7);
            DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,FALSE,          data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_Pen,           0);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set, 8,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set, 9,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,10,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            7,              data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,11,MUIV_Toolbar_Set_Selected,TRUE);
            DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            0,              data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,11,MUIV_Toolbar_Set_Selected,FALSE);
            DoMethod(mi_bold              ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set, 8,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(mi_italic            ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set, 9,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(mi_underl            ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,10,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
            DoMethod(mi_color             ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TO_TOOLBAR,4,MUIM_Toolbar_Set,11,MUIV_Toolbar_Set_Selected,MUIV_TriggerValue);
         }
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_UndoAvailable,MUIV_EveryTime,mi_undo            ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_RedoAvailable,MUIV_EveryTime,mi_redo            ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);
         if (data->GUI.TX_POSI)
         {
            DoMethod(data->GUI.TE_EDIT ,MUIM_Notify,MUIA_TextEditor_CursorX,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&WR_UpdateWTitleHook,winnum);
            DoMethod(data->GUI.TE_EDIT ,MUIM_Notify,MUIA_TextEditor_CursorY,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&WR_UpdateWTitleHook,winnum);
         }
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, mi_bold        ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, mi_italic      ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, mi_underl      ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            7,              mi_color       ,3,MUIM_Set        ,MUIA_Menuitem_Checked,TRUE);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            0,              mi_color       ,3,MUIM_Set        ,MUIA_Menuitem_Checked,FALSE);
         DoMethod(mi_bold              ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleBold,     MUIV_TriggerValue);
         DoMethod(mi_italic            ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleItalic,   MUIV_TriggerValue);
         DoMethod(mi_underl            ,MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_StyleUnderline,MUIV_TriggerValue);
         DoMethod(mi_color             ,MUIM_Notify, MUIA_Menuitem_Checked,          TRUE,           data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_Pen,           7);
         DoMethod(mi_color             ,MUIM_Notify, MUIA_Menuitem_Checked,          FALSE,          data->GUI.TE_EDIT,3,MUIM_Set,MUIA_TextEditor_Pen,           0);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,0             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,FALSE);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,1             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,TRUE);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,2             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,TRUE);
         DoMethod(data->GUI.ST_SUBJECT ,MUIM_Notify,MUIA_String_Acknowledge  ,MUIV_EveryTime,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_ActiveObject,data->GUI.TE_EDIT);
         DoMethod(data->GUI.BT_ADD     ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddFileHook,winnum);
         DoMethod(data->GUI.BT_ADDPACK ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddArchiveHook,winnum);
         DoMethod(data->GUI.BT_DEL     ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,data->GUI.LV_ATTACH    ,2,MUIM_List_Remove,MUIV_List_Remove_Active);
         DoMethod(data->GUI.BT_DISPLAY ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,data->GUI.LV_ATTACH    ,3,MUIM_CallHook   ,&WR_DisplayFileHook,winnum);
         DoMethod(data->GUI.LV_ATTACH  ,MUIM_Notify,MUIA_List_Active         ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_GetFileEntryHook,winnum);
         DoMethod(data->GUI.RA_ENCODING,MUIM_Notify,MUIA_Radio_Active        ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.ST_CTYPE   ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.ST_DESC    ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.RA_SIGNATURE,MUIM_Notify,MUIA_Radio_Active        ,MUIV_EveryTime,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_ChangeSignatureHook,MUIV_TriggerValue,winnum);
         DoMethod(data->GUI.CH_DELSEND ,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_delsend             ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.CH_RECEIPT ,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_receipt             ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.CH_DISPNOTI,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_dispnoti            ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.CH_ADDINFO ,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_addinfo             ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(mi_autospell         ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.TE_EDIT      ,3,MUIM_Set        ,MUIA_TextEditor_TypeAndSpell,MUIV_TriggerValue);
         DoMethod(mi_delsend           ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_DELSEND   ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(mi_receipt           ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_RECEIPT   ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(mi_dispnoti          ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_DISPNOTI  ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(mi_addinfo           ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_ADDINFO   ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(data->GUI.RA_SECURITY,MUIM_Notify,MUIA_Radio_Active        ,4             ,data->GUI.RA_SIGNATURE ,3,MUIM_Set        ,MUIA_Radio_Active,0);
         DoMethod(data->GUI.RA_SECURITY,MUIM_Notify,MUIA_Radio_Active        ,4             ,data->GUI.CH_ADDINFO   ,3,MUIM_Set        ,MUIA_Selected,FALSE);
         for (i = 0; i < 3; i++)
         {
            DoMethod(data->GUI.CY_IMPORTANCE,MUIM_Notify,MUIA_Cycle_Active     ,i              ,strip                  ,4,MUIM_SetUData,WMEN_IMPORT0+i,MUIA_Menuitem_Checked,TRUE);
            DoMethod(data->GUI.WI           ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_IMPORT0+i ,data->GUI.CY_IMPORTANCE,3,MUIM_Set     ,MUIA_Cycle_Active,i);
         }
         for (i = 0; i < 4; i++) 
         {
            DoMethod(data->GUI.RA_SIGNATURE ,MUIM_Notify,MUIA_Radio_Active     ,i              ,strip                  ,4,MUIM_SetUData,WMEN_SIGN0+i,MUIA_Menuitem_Checked,TRUE);
            DoMethod(data->GUI.WI           ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_SIGN0+i   ,data->GUI.RA_SIGNATURE ,3,MUIM_Set     ,MUIA_Radio_Active,i);
         }
         for (i = 0; i < 5; i++) 
         {
           DoMethod(data->GUI.WI           ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_SECUR0+i  ,data->GUI.RA_SECURITY  ,3,MUIM_Set     ,MUIA_Radio_Active,i);
         }
         WR_SharedSetup(data, winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}

///
/// WR_NewBounce
//  Creates a bounce window
LOCAL struct WR_ClassData *WR_NewBounce(int winnum)
{
   struct WR_ClassData *data;

   if (data = calloc(1,sizeof(struct WR_ClassData)))
   {
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_WR_BounceWT),
         MUIA_HelpNode, "WR_W",
         MUIA_Window_ID, MAKE_ID('W','R','I','B'),
         WindowContents, VGroup, GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, ColGroup(2),
               Child, Label2(GetStr(MSG_WR_BounceTo)),
               Child, MakeAddressField(&data->GUI.ST_TO, GetStr(MSG_WR_BounceTo), MSG_HELP_WR_ST_TO, ABM_TO, winnum, TRUE),
            End,
            Child, ColGroup(4),
               Child, data->GUI.BT_SEND   = MakeButton(GetStr(MSG_WR_Send)),
               Child, data->GUI.BT_QUEUE  = MakeButton(GetStr(MSG_WR_ToQueue)),
               Child, data->GUI.BT_HOLD   = MakeButton(GetStr(MSG_WR_Hold)),
               Child, data->GUI.BT_CANCEL = MakeButton(GetStr(MSG_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         set(data->GUI.ST_TO, MUIA_UserData, data->GUI.BT_SEND);
         WR_SharedSetup(data, winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
