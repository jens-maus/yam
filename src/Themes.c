/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/icon.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "Locale.h"
#include "ImageCache.h"
#include "Themes.h"

#include "Debug.h"

// The themelayout version defines the current "version" of
// the theme layout we are currently using
//
// Please note that as soon as you change something to the internal
// image arrays of the theme you have to bump the VERSION accordingly
// to make sure the user is reminded of having the correct image layout
// installed or not.
//
// VERSIONS:
// 1: YAM 2.5
// 2: YAM 2.8
//
#define THEME_REQVERSION 2

// static image identifiers
/// config image IDs
static const char * const configImageIDs[CI_MAX] =
{
  "config_abook",     "config_abook_big",
  "config_answer",    "config_answer_big",
  "config_filters",   "config_filters_big",
  "config_firststep", "config_firststep_big",
  "config_lookfeel",  "config_lookfeel_big",
  "config_mime",      "config_mime_big",
  "config_misc",      "config_misc_big",
  "config_network",   "config_network_big",
  "config_read",      "config_read_big",
  "config_scripts",   "config_scripts_big",
  "config_security",  "config_security_big",
  "config_signature", "config_signature_big",
  "config_spam",      "config_spam_big",
  "config_start",     "config_start_big",
  "config_update",    "config_update_big",
  "config_write",     "config_write_big",
  "config_identities","config_identities_big"
};
///
/// folder image IDs
static const char * const folderImageIDs[FI_MAX] =
{
  "folder_fold",
  "folder_unfold",
  "folder_incoming",
  "folder_incoming_new",
  "folder_outgoing",
  "folder_outgoing_new",
  "folder_sent",
  "folder_protected",
  "folder_spam",
  "folder_spam_new",
  "folder_trash",
  "folder_trash_new",
  "folder_drafts",
  "folder_drafts_new",
  "folder_archive",
  "folder_archive_fold",
  "folder_archive_unfold"
};
///
/// icon image IDs
static const char * const iconImageIDs[II_MAX] =
{
  "check",
  "empty",
  "new",
  "old"
};
///
/// status image IDs
static const char * const statusImageIDs[SI_MAX] =
{
  "status_attach",
  "status_crypt",
  "status_delete",
  "status_download",
  "status_error",
  "status_forward",
  "status_group",
  "status_hold",
  "status_mark",
  "status_new",
  "status_old",
  "status_reply",
  "status_report",
  "status_sent",
  "status_signed",
  "status_spam",
  "status_unread",
  "status_urgent",
  "status_waitsend"
};
///
/// toolbar image IDs
static const char * const tbii[TBI_MAX][TBIM_MAX] =
{
 // Normal            Selected            Ghosted
  { "tb_read",        "tb_read_s",        "tb_read_g"       },
  { "tb_edit",        "tb_edit_s",        "tb_edit_g"       },
  { "tb_move",        "tb_move_s",        "tb_move_g"       },
  { "tb_delete",      "tb_delete_s",      "tb_delete_g"     },
  { "tb_getaddr",     "tb_getaddr_s",     "tb_getaddr_g"    },
  { "tb_newmail",     "tb_newmail_s",     "tb_newmail_g"    },
  { "tb_reply",       "tb_reply_s",       "tb_reply_g"      },
  { "tb_forward",     "tb_forward_s",     "tb_forward_g"    },
  { "tb_getmail",     "tb_getmail_s",     "tb_getmail_g"    },
  { "tb_sendall",     "tb_sendall_s",     "tb_sendall_g"    },
  { "tb_spam",        "tb_spam_s",        "tb_spam_g"       },
  { "tb_ham",         "tb_ham_s",         "tb_ham_g"        },
  { "tb_filter",      "tb_filter_s",      "tb_filter_g"     },
  { "tb_find",        "tb_find_s",        "tb_find_g"       },
  { "tb_addrbook",    "tb_addrbook_s",    "tb_addrbook_g"   },
  { "tb_config",      "tb_config_s",      "tb_config_g"     },
  { "tb_prev",        "tb_prev_s",        "tb_prev_g"       },
  { "tb_next",        "tb_next_s",        "tb_next_g"       },
  { "tb_prevthread",  "tb_prevthread_s",  "tb_prevthread_g" },
  { "tb_nextthread",  "tb_nextthread_s",  "tb_nextthread_g" },
  { "tb_display",     "tb_display_s",     "tb_display_g"    },
  { "tb_save",        "tb_save_s",        "tb_save_g"       },
  { "tb_print",       "tb_print_s",       "tb_print_g"      },
  { "tb_editor",      "tb_editor_s",      "tb_editor_g"     },
  { "tb_insert",      "tb_insert_s",      "tb_insert_g"     },
  { "tb_cut",         "tb_cut_s",         "tb_cut_g"        },
  { "tb_copy",        "tb_copy_s",        "tb_copy_g"       },
  { "tb_paste",       "tb_paste_s",       "tb_paste_g"      },
  { "tb_undo",        "tb_undo_s",        "tb_undo_g"       },
  { "tb_bold",        "tb_bold_s",        "tb_bold_g"       },
  { "tb_italic",      "tb_italic_s",      "tb_italic_g"     },
  { "tb_underline",   "tb_underline_s",   "tb_underline_g"  },
  { "tb_colored",     "tb_colored_s",     "tb_colored_g"    },
  { "tb_newuser",     "tb_newuser_s",     "tb_newuser_g"    },
  { "tb_newlist",     "tb_newlist_s",     "tb_newlist_g"    },
  { "tb_newgroup",    "tb_newgroup_s",    "tb_newgroup_g"   },
  { "tb_opentree",    "tb_opentree_s",    "tb_opentree_g"   },
  { "tb_closetree",   "tb_closetree_s",   "tb_closetree_g"  }
};
///

// window toolbar mappings
/// main window toolbar image IDs
static const int mainWindowToolbarImageIDs[MWTBI_MAX] =
{
  TBI_READ,
  TBI_EDIT,
  TBI_MOVE,
  TBI_DELETE,
  TBI_GETADDR,
  TBI_NEWMAIL,
  TBI_REPLY,
  TBI_FORWARD,
  TBI_GETMAIL,
  TBI_SENDALL,
  TBI_SPAM,
  TBI_HAM,
  TBI_FILTER,
  TBI_FIND,
  TBI_ADDRBOOK,
  TBI_CONFIG
};
///
/// read window toolbar image IDs
static const int readWindowToolbarImageIDs[RWTBI_MAX] =
{
  TBI_PREV,
  TBI_NEXT,
  TBI_PREVTHREAD,
  TBI_NEXTTHREAD,
  TBI_DISPLAY,
  TBI_SAVE,
  TBI_PRINT,
  TBI_DELETE,
  TBI_MOVE,
  TBI_REPLY,
  TBI_FORWARD,
  TBI_SPAM,
  TBI_HAM
};
///
/// write window toolbar image IDs
static const int writeWindowToolbarImageIDs[WWTBI_MAX] =
{
  TBI_EDITOR,
  TBI_INSERT,
  TBI_CUT,
  TBI_COPY,
  TBI_PASTE,
  TBI_UNDO,
  TBI_BOLD,
  TBI_ITALIC,
  TBI_UNDERLINE,
  TBI_COLORED,
  TBI_FIND
};
///
/// addressbook window toolbar image IDs
static const int abookWindowToolbarImageIDs[AWTBI_MAX] =
{
  TBI_SAVE,
  TBI_FIND,
  TBI_NEWUSER,
  TBI_NEWLIST,
  TBI_NEWGROUP,
  TBI_EDIT,
  TBI_DELETE,
  TBI_PRINT,
  TBI_OPENTREE,
  TBI_CLOSETREE
};
///

// public functions
/// AllocTheme
// allocate everything for a theme while setting everything
// to the default first.
void AllocTheme(struct Theme *theme, const char *themeName)
{
  int i;
  int j;
  char dirname[SIZE_PATHFILE];
  char filepath[SIZE_PATHFILE];

  ENTER();

  theme->name = NULL;
  theme->author = NULL;
  theme->version = NULL;

  // contruct the path to the themes directory
  AddPath(theme->directory, G->ThemesDir, themeName, sizeof(theme->directory));

  D(DBF_THEME, "theme directory: '%s' '%s'", theme->directory, G->ThemesDir);

  // construct pathes to config images
  AddPath(dirname, theme->directory, "config", sizeof(dirname));
  for(i=FI_FIRST; i < CI_MAX; i++)
  {
    AddPath(filepath, dirname, configImageIDs[i], sizeof(filepath));
    theme->configImages[i] = strdup(filepath);
  }

  // construct pathes to folder images
  AddPath(dirname, theme->directory, "folder", sizeof(dirname));
  for(i=FI_FIRST; i < FI_MAX; i++)
  {
    AddPath(filepath, dirname, folderImageIDs[i], sizeof(filepath));
    theme->folderImages[i] = strdup(filepath);
  }

  // construct pathes to icon images
  AddPath(dirname, theme->directory, "icon", sizeof(dirname));
  for(i=II_FIRST; i < II_MAX; i++)
  {
    AddPath(filepath, dirname, iconImageIDs[i], sizeof(filepath));
    theme->iconImages[i] = strdup(filepath);
  }

  // construct pathes to status images
  AddPath(dirname, theme->directory, "status", sizeof(dirname));
  for(i=SI_FIRST; i < SI_MAX; i++)
  {
    AddPath(filepath, dirname, statusImageIDs[i], sizeof(filepath));
    theme->statusImages[i] = strdup(filepath);
  }

  // construct pathes for the toolbar images
  AddPath(dirname, theme->directory, "toolbar", sizeof(dirname));
  for(j=TBIM_FIRST; j < TBIM_MAX; j++)
  {
    // main window toolbar
    for(i=MWTBI_FIRST; i < MWTBI_NULL; i++)
    {
      AddPath(filepath, dirname, tbii[mainWindowToolbarImageIDs[i]][j], sizeof(filepath));
      theme->mainWindowToolbarImages[j][i] = strdup(filepath);
    }
    // the array must be NULL terminated
    theme->mainWindowToolbarImages[j][MWTBI_NULL] = NULL;

    // read window toolbar
    for(i=RWTBI_FIRST; i < RWTBI_NULL; i++)
    {
      AddPath(filepath, dirname, tbii[readWindowToolbarImageIDs[i]][j], sizeof(filepath));
      theme->readWindowToolbarImages[j][i] = strdup(filepath);
    }
    // the array must be NULL terminated
    theme->readWindowToolbarImages[j][RWTBI_NULL] = NULL;

    // write window toolbar
    for(i=WWTBI_FIRST; i < WWTBI_NULL; i++)
    {
      AddPath(filepath, dirname, tbii[writeWindowToolbarImageIDs[i]][j], sizeof(filepath));
      theme->writeWindowToolbarImages[j][i] = strdup(filepath);
    }
    // the array must be NULL terminated
    theme->writeWindowToolbarImages[j][WWTBI_NULL] = NULL;

    // addressbook window toolbar
    for(i=AWTBI_FIRST; i < AWTBI_NULL; i++)
    {
      AddPath(filepath, dirname, tbii[abookWindowToolbarImageIDs[i]][j], sizeof(filepath));
      theme->abookWindowToolbarImages[j][i] = strdup(filepath);
    }
    // the array must be NULL terminated
    theme->abookWindowToolbarImages[j][AWTBI_NULL] = NULL;
  }

  theme->loaded = FALSE;

  LEAVE();
}
///
/// ParseThemeFile
// parse a complete theme file and returns > 0 in case of success
LONG ParseThemeFile(const char *themeFile, struct Theme *theme)
{
  LONG result = 0; // signals an error
  FILE *fh;

  ENTER();

  if((fh = fopen(themeFile, "r")) != NULL)
  {
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(getline(&buf, &buflen, fh) >= 3 && strnicmp(buf, "YTH", 3) == 0)
    {
      int version = atoi(&buf[3]);

      // check if the these has the correct version
      if(version == THEME_REQVERSION)
      {
        while(getline(&buf, &buflen, fh) > 0)
        {
          char *p;
          char *id;
          char *value;

          if((p = strpbrk(buf, ";#\r\n")) != NULL)
            *p = '\0';

          if((value = strchr(buf, '=')) != NULL)
          {
            *value++ = '\0';
            value = Trim(value);
          }
          else
          {
            // assume empty filename
            value = (char *)"";
          }

          id = Trim(buf);
          if(id[0] != '\0')
          {
            int i;
            int j;
            BOOL found = FALSE;

            // theme description
            if(stricmp(id, "Name") == 0)
            {
              free(theme->name);
              theme->name = strdup(value);
              found = TRUE;
            }
            else if(stricmp(id, "Author") == 0)
            {
              free(theme->author);
              theme->author = strdup(value);
              found = TRUE;
            }
            else if(stricmp(id, "URL") == 0)
            {
              free(theme->url);
              theme->url = strdup(value);
              found = TRUE;
            }
            else if(stricmp(id, "Version") == 0)
            {
              free(theme->version);
              theme->version = strdup(value);
              found = TRUE;
            }
            else if(stricmp(id, "IgnoreMissingImages") == 0)
            {
              G->NoImageWarning = Txt2Bool(value);
              found = TRUE;
            }
            else
            {
              char *image;

              if(strchr(value, ':') == NULL)
              {
                // image filename is relative to the theme directory
                if(asprintf(&image, "%s/%s", theme->directory, value) == -1)
                  image = NULL;
              }
              else
              {
                // leave the name as it is
                image = value;
              }

              if(image != NULL)
              {
                // config images
                for(i=CI_FIRST; i < CI_MAX && found == FALSE; i++)
                {
                  if(stricmp(id, configImageIDs[i]) == 0)
                  {
                    free(theme->configImages[i]);
                    theme->configImages[i] = strdup(image);
                    found = TRUE;
                  }
                }

                // folder images
                for(i=FI_FIRST; i < FI_MAX && found == FALSE; i++)
                {
                  if(stricmp(id, folderImageIDs[i]) == 0)
                  {
                    free(theme->folderImages[i]);
                    theme->folderImages[i] = strdup(image);
                    found = TRUE;
                  }
                }

                // icon images
                for(i=II_FIRST; i < II_MAX && found == FALSE; i++)
                {
                  if(stricmp(id, iconImageIDs[i]) == 0)
                  {
                    free(theme->iconImages[i]);
                    theme->iconImages[i] = strdup(image);
                    found = TRUE;
                  }
                }

                // status images
                for(i=SI_FIRST; i < SI_MAX && found == FALSE; i++)
                {
                  if(stricmp(id, statusImageIDs[i]) == 0)
                  {
                    free(theme->statusImages[i]);
                    theme->statusImages[i] = strdup(image);
                    found = TRUE;
                  }
                }

                // toolbar images
                for(j=TBIM_FIRST; j < TBIM_MAX && found == FALSE; j++)
                {
                  // main window toolbar
                  for(i=MWTBI_FIRST; i < MWTBI_NULL; i++)
                  {
                    if(stricmp(id, tbii[mainWindowToolbarImageIDs[i]][j]) == 0)
                    {
                      free(theme->mainWindowToolbarImages[j][i]);
                      theme->mainWindowToolbarImages[j][i] = strdup(image);
                      found = TRUE;
                    }
                  }

                  // read window toolbar
                  for(i=RWTBI_FIRST; i < RWTBI_NULL; i++)
                  {
                    if(stricmp(id, tbii[readWindowToolbarImageIDs[i]][j]) == 0)
                    {
                      free(theme->readWindowToolbarImages[j][i]);
                      theme->readWindowToolbarImages[j][i] = strdup(image);
                      found = TRUE;
                    }
                  }

                  // write window toolbar
                  for(i=WWTBI_FIRST; i < WWTBI_NULL; i++)
                  {
                    if(stricmp(id, tbii[writeWindowToolbarImageIDs[i]][j]) == 0)
                    {
                      free(theme->writeWindowToolbarImages[j][i]);
                      theme->writeWindowToolbarImages[j][i] = strdup(image);
                      found = TRUE;
                    }
                  }

                  // addressbook window toolbar
                  for(i=AWTBI_FIRST; i < AWTBI_NULL; i++)
                  {
                    if(stricmp(id, tbii[abookWindowToolbarImageIDs[i]][j]) == 0)
                    {
                      free(theme->abookWindowToolbarImages[j][i]);
                      theme->abookWindowToolbarImages[j][i] = strdup(image);
                      found = TRUE;
                    }
                  }
                }

                // free the image name if it was constructed from the theme path
                if(image != value)
                  free(image);
              }
            }

            if(found == FALSE)
            {
              W(DBF_IMAGE, "unknown theme setting '%s' = '%s'", id, value);
            }

            result = 1; // signal success
          }
        }
      }
      else
      {
        W(DBF_THEME, "incorrect theme version found: %ld != %ld", version, THEME_REQVERSION);

        result = -1; // signal a version problem
      }
    }
    else
      W(DBF_THEME, "invalid header in .theme file found");

    free(buf);

    fclose(fh);
  }
  else
    W(DBF_THEME, "couldn't open .theme file '%s'", themeFile);

  RETURN(result);
  return result;
}
///
/// FreeTheme
// free all the strings of a theme
void FreeTheme(struct Theme *theme)
{
  int i;
  int j;

  ENTER();

  if(theme->loaded == TRUE)
    UnloadTheme(theme);

  free(theme->name);
  theme->name = NULL;

  free(theme->author);
  theme->author = NULL;

  free(theme->url);
  theme->url = NULL;

  free(theme->version);
  theme->version = NULL;

  for(i=CI_FIRST; i < CI_MAX; i++)
  {
    free(theme->configImages[i]);
    theme->configImages[i] = NULL;
  }

  for(i=FI_FIRST; i < FI_MAX; i++)
  {
    free(theme->folderImages[i]);
    theme->folderImages[i] = NULL;
  }

  for(i=II_FIRST; i < II_MAX; i++)
  {
    free(theme->iconImages[i]);
    theme->iconImages[i] = NULL;
  }

  for(i=SI_FIRST; i < SI_MAX; i++)
  {
    free(theme->statusImages[i]);
    theme->statusImages[i] = NULL;
  }

  // free the toolbar images
  for(j=TBIM_FIRST; j < TBIM_MAX; j++)
  {
    // main window toolbar
    for(i=MWTBI_FIRST; i < MWTBI_NULL; i++)
    {
      free(theme->mainWindowToolbarImages[j][i]);
      theme->mainWindowToolbarImages[j][i] = NULL;
    }

    // read window toolbar
    for(i=RWTBI_FIRST; i < RWTBI_NULL; i++)
    {
      free(theme->readWindowToolbarImages[j][i]);
      theme->readWindowToolbarImages[j][i] = NULL;
    }

    // write window toolbar
    for(i=WWTBI_FIRST; i < WWTBI_NULL; i++)
    {
      free(theme->writeWindowToolbarImages[j][i]);
      theme->writeWindowToolbarImages[j][i] = NULL;
    }

    // addressbook window toolbar
    for(i=AWTBI_FIRST; i < AWTBI_NULL; i++)
    {
      free(theme->abookWindowToolbarImages[j][i]);
      theme->abookWindowToolbarImages[j][i] = NULL;
    }
  }

  LEAVE();
}
///
/// LoadTheme
// load all images of a theme
void LoadTheme(struct Theme *theme, const char *themeName)
{
  char themeFile[SIZE_PATHFILE];
  int i;
  LONG res;

  ENTER();

  // allocate all resources for the theme
  AllocTheme(theme, themeName);

  // Parse the .theme file within the
  // theme directory
  AddPath(themeFile, theme->directory, ".theme", sizeof(themeFile));
  res = ParseThemeFile(themeFile, theme);

  // check if parsing the theme file worked out or not
  if(res > 0)
    D(DBF_THEME, "successfully parsed theme file '%s'", themeFile);
  else
  {
    // check if it was the default theme that failed or
    // not.
    if(stricmp(themeName, "default") == 0)
    {
      W(DBF_THEME, "parsing of theme file '%s' failed! ignoring...", themeFile);

      // warn the user
      if(res == -1)
        ER_NewError(tr(MSG_ER_THEMEVER_IGNORE));
      else
        ER_NewError(tr(MSG_ER_THEME_FATAL));

      // free the theme resources
      FreeTheme(theme);

      LEAVE();
      return;
    }
    else
    {
      W(DBF_THEME, "parsing of theme file '%s' failed! trying default theme...", themeFile);

      // warn the user
      if(res == -1)
        ER_NewError(tr(MSG_ER_THEMEVER_FALLBACK), themeName);
      else
        ER_NewError(tr(MSG_ER_THEME_FALLBACK), themeName);

      // free the theme resources
      FreeTheme(theme);

      // allocate the default theme
      AllocTheme(theme, "default");
      AddPath(themeFile, theme->directory, ".theme", sizeof(themeFile));
      if(ParseThemeFile(themeFile, theme) <= 0)
      {
        // warn the user
        if(res == -1)
          ER_NewError(tr(MSG_ER_THEMEVER_IGNORE));
        else
          ER_NewError(tr(MSG_ER_THEME_FATAL));

        FreeTheme(theme);

        LEAVE();
        return;
      }
    }
  }

  for(i=CI_FIRST; i < CI_MAX; i++)
  {
    char *image = theme->configImages[i];

    if(IsStrEmpty(image) == FALSE)
    {
      if(ObtainImage(configImageIDs[i], image, NULL) == NULL)
        W(DBF_THEME, "couldn't obtain image '%s' of theme '%s'", image, theme->directory);
      else
        ReleaseImage(configImageIDs[i], FALSE);
    }
  }

  for(i=FI_FIRST; i < FI_MAX; i++)
  {
    char *image = theme->folderImages[i];

    if(IsStrEmpty(image) == FALSE)
    {
      if(ObtainImage(folderImageIDs[i], image, NULL) == NULL)
        W(DBF_THEME, "couldn't obtain image '%s' of theme '%s'", image, theme->directory);
      else
        ReleaseImage(folderImageIDs[i], FALSE);
    }
  }

  for(i=SI_FIRST; i < SI_MAX; i++)
  {
    char *image = theme->statusImages[i];

    if(IsStrEmpty(image) == FALSE)
    {
      if(ObtainImage(statusImageIDs[i], image, NULL) == NULL)
        W(DBF_THEME, "couldn't obtain image '%s' of theme '%s'", image, theme->directory);
      else
        ReleaseImage(statusImageIDs[i], FALSE);
    }
  }

  for(i=II_FIRST; i < II_MAX; i++)
  {
    char *image = theme->iconImages[i];

    if(IsStrEmpty(image) == FALSE)
    {
      char osIconImage[SIZE_PATHFILE];

      // first we try to obtain a system specific icon image
      #if defined(__amigaos3__)
      snprintf(osIconImage, sizeof(osIconImage), "%s_os3", theme->iconImages[i]);
      #elif defined(__amigaos4__)
      snprintf(osIconImage, sizeof(osIconImage), "%s_os4", theme->iconImages[i]);
      #elif defined(__MORPHOS__)
      snprintf(osIconImage, sizeof(osIconImage), "%s_mos", theme->iconImages[i]);
      #elif defined(__AROS__)
      snprintf(osIconImage, sizeof(osIconImage), "%s_aros", theme->iconImages[i]);
      #else
      #error no build system defined
      #endif

      D(DBF_THEME, "trying to load system specific icon file '%s'", osIconImage);
      // depending on the icon.library version we use either GetIconTags()
      // or the older GetDiskObject() function
      if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
        theme->icons[i] = GetIconTags(osIconImage, TAG_DONE);
      else
        theme->icons[i] = GetDiskObject(osIconImage);

      // if obtaining a system specific icon image failed we try the default icon
      if(theme->icons[i] == NULL)
      {
        // depending on the icon.library version we use either GetIconTags()
        // or the older GetDiskObject() function
        if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
          theme->icons[i] = GetIconTags(image, TAG_DONE);
        else
          theme->icons[i] = GetDiskObject(image);

        // load the diskobject and report an error if something went wrong.
        if(theme->icons[i] == NULL && G->NoImageWarning == FALSE)
          ER_NewError(tr(MSG_ER_ICONOBJECT_WARNING), FilePart(image), themeName);
      }
    }
  }

  theme->loaded = TRUE;

  LEAVE();
}
///
/// UnloadTheme
// unload all images of a theme
void UnloadTheme(struct Theme *theme)
{
  int i;

  ENTER();

  for(i=CI_FIRST; i < CI_MAX; i++)
    ReleaseImage(configImageIDs[i], TRUE);

  for(i=FI_FIRST; i < FI_MAX; i++)
    ReleaseImage(folderImageIDs[i], TRUE);

  for(i=SI_FIRST; i < SI_MAX; i++)
    ReleaseImage(statusImageIDs[i], TRUE);

  for(i=II_FIRST; i < II_MAX; i++)
  {
    if(theme->icons[i] != NULL)
    {
      FreeDiskObject(theme->icons[i]);
      theme->icons[i] = NULL;
    }
  }

  theme->loaded = FALSE;

  LEAVE();
}

///
