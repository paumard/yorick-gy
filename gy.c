/*
    Copyright 2013 Thibaut Paumard

    This file is part of gy (GObject Introspection for Yorick).

    Gyoto is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Gyoto is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with gy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gy.h"
#include "ctype.h"

/// utility functions

char *
p_strtolower(const char * in)
{
  char * cur, * out = p_strcpy(in);
  for (cur=out; *cur; ++cur) *cur=tolower(*cur);
  return out;
}

/// YAPI FUNCTIONS

void
Y_gy_init(int argc)
{
#if !GLIB_CHECK_VERSION(2,35,1)
  g_type_init();
#endif
  ypush_gy_Repository()->repo = g_irepository_get_default();
}

void
Y_gy_list(int argc)
{
  if (yarg_gy_Object(argc-1)) gy_Object_list(argc);
  else gy_Typelist_list(argc);
}

static gboolean _gy_debug = 0;

gboolean gy_debug() { return _gy_debug; }

void
Y_gy_debug(int argc)
{
  ypush_long(_gy_debug);
  if (argc && !yarg_nil(argc)) _gy_debug = ygets_l(argc);
}

void
Y_gy_setlocale(int argc)
{
  if (argc > 2) y_error("gy_setlocale, [[CATEGORY, ], LOCALE]");
  char * scat="LC_ALL";
  char * sloc =NULL;
  int cat=0;
  if (argc == 2) scat = ygets_q(1);
  if (yarg_string(0)) sloc = ygets_q(0);
  if (!strcmp(scat, "LC_ALL")) cat = LC_ALL;
  else if (!strcmp(scat, "LC_COLLATE")) cat = LC_COLLATE;
  else if (!strcmp(scat, "LC_CTYPE")) cat = LC_CTYPE;
  else if (!strcmp(scat, "LC_MONETARY")) cat = LC_MONETARY;
  else if (!strcmp(scat, "LC_NUMERIC")) cat = LC_NUMERIC;
  else if (!strcmp(scat, "LC_TIME")) cat = LC_TIME;
  else y_error("unsupported locale category");

  if (sloc && cat==LC_NUMERIC && strcmp(sloc, "C"))
    y_error("Yorick does not support LC_NUMERIC != \"C\"");

  *ypush_q(0) = p_strcpy(setlocale(cat, sloc));

  setlocale(LC_NUMERIC, "C");

}

void
Y_gy_id(int argc)
{
  ypush_long((long) yget_gy_Object(argc-1)->object);
}

/*
void * gy_thread(void * cmd) {
  yexec_include(0,1);
  return NULL;
}

void
Y_gy_thread(int argc) {
  pthread_t thread;
  pthread_create(&thread, NULL, &gy_thread, NULL);
  sleep(100);
}
*/

// This code is never used: it serves only the purpose of doing some compile
// time tests.
void
__gy_build_bug_on()
{
  GY_BUILD_BUG_ON(sizeof(char)!=1);
  GY_BUILD_BUG_ON(sizeof(short)!=2);
  GY_BUILD_BUG_ON(sizeof(int)!=4);
  GY_BUILD_BUG_ON(sizeof(void*)!=sizeof(long));
}
