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

#include <girepository.h>
#include <glib-object.h>

#include "yapi.h"
#include "pstdlib.h"

#include <stdio.h>
#include <fenv.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
//#include <pthread.h>
//#include <stdio.h>

typedef struct _gy_signal_data {
  GIBaseInfo * info;
  GIRepository * repo;
  const char * cmd;
  void * data;
} gy_signal_data;

gboolean gy_debug() ;

#define GY_DEBUG( ... ) \
  if (gy_debug()) fprintf(stderr, "GY DEBUG: " __VA_ARGS__ );

typedef struct _gy_Object {
  GIBaseInfo * info;
  GObject * object;
  GIRepository * repo;
} gy_Object;
gy_Object* yget_gy_Object(int);
gy_Object* ypush_gy_Object();
void gy_Argument_pushany(GIArgument * arg, GITypeInfo * info, gy_Object* o);


void gy_sa_handler(int sig) ;

typedef struct _gy_Typelib {
  GITypelib * typelib;
  gchar * namespace;
  GIRepository * repo;
} gy_Typelib;

void gy_Typelib_free(void *obj);
void gy_Typelib_print(void *obj);
//void gy_Typelib_eval(void *obj, int argc);
void gy_Typelib_extract(void *, char *);
void gy_Typelib_free(void *obj) ;
void gy_Typelib_print(void *obj);
gy_Typelib* yget_gy_Typelib(int iarg) ;
gy_Typelib* ypush_gy_Typelib() ;

/// GYREPOSITORY

typedef struct _gy_Repository {
  GIRepository * repo;
  char * method;
} gy_Repository;

//void gy_Repository_free(void *obj);
void gy_Repository_print(void *obj);
void gy_Repository_eval(void *obj, int argc);
void gy_Repository_extract(void *, char *);
int yarg_gy_Object(int iarg) ;
gy_Repository* yget_gy_Repository(int iarg) ;
gy_Repository* ypush_gy_Repository();

/// GIBASEINFO

void gy_Object_free(void *obj);
void gy_Object_print(void *obj);
void gy_Object_eval(void *obj, int argc);
void gy_Object_extract(void *, char *);

void gy_Argument_getany(GIArgument * arg, GITypeInfo * info, int iarg) ;
void gy_Argument_pushany(GIArgument * arg, GITypeInfo * info, gy_Object* o) ;
int yarg_gy_Object(int iarg) ;
gy_Object* yget_gy_Object(int iarg);
gy_Object* ypush_gy_Object() ;

void gy_callback0(void* arg1, gy_signal_data* sd) ;
gboolean gy_callback0_bool(void* arg1, gy_signal_data* sd) ;
void gy_callback1(void* arg1, void* arg2, gy_signal_data* sd) ;
gboolean gy_callback1_bool(void* arg1, void* arg2, gy_signal_data* sd) ;
void gy_callback2(void* arg1, void* arg2, void* arg3, gy_signal_data* sd) ;
gboolean gy_callback2_bool(void* arg1, void* arg2, void*arg3,
			   gy_signal_data* sd) ;

/// Properties
GIPropertyInfo * gy_base_info_find_property_info(GIBaseInfo * objectinfo,
						char * name);
GIPropertyInfo * gy_base_info_find_field_info(GIBaseInfo * objectinfo,
						char * name);
void gy_value_init(GValue* val, GITypeInfo *info);
void gy_value_set_iarg(GValue* val, GITypeInfo * info, int iarg);
void gy_value_push(GValue * pval, GITypeInfo * info, gy_Object *o);

// This should be OK for most mainstream architectures, but...
#define GY_BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define GY_HAVE_INT64 GLIB_SIZEOF_LONG==8
#define ygets_gint8(iarg)   ((gint8)  ygets_c(iarg))
#define ygets_guint8(iarg)  ((guint8) ygets_uc(iarg))
#define ygets_gint16(iarg)  ((gint16) ygets_s(iarg))
#define ygets_guint16(iarg) ((guint16)ygets_s(iarg))
#define ygets_gint32(iarg)  ((gint32) ygets_i(iarg))
#define ygets_guint32(iarg) ((guint32)ygets_i(iarg))
#if GY_HAVE_INT64
# define ygets_gint64(iarg) ((gint64)ygets_l(iarg))
# define ygets_guint64(iarg) ((guint64)ygets_l(iarg))
#else
# define ygets_gint64(iarg) 1/0
# define ygets_guint64(iarg) 1/0
#endif
#define ygeta_gint8(iarg, ntot, dims)   ((gint8*)  ygeta_c(iarg, ntot, dims))
#define ygeta_guint8(iarg, ntot, dims)  ((guint8*) ygeta_uc(iarg, ntot, dims))
#define ygeta_gint16(iarg, ntot, dims)  ((gint16*) ygeta_s(iarg, ntot, dims))
#define ygeta_guint16(iarg, ntot, dims) ((guint16*)ygeta_s(iarg, ntot, dims))
#define ygeta_gint32(iarg, ntot, dims)  ((gint32*) ygeta_i(iarg, ntot, dims))
#define ygeta_guint32(iarg, ntot, dims) ((guint32*)ygeta_i(iarg, ntot, dims))
#if GY_HAVE_INT64
# define ygeta_gint64(iarg, ntot, dims)  ((gint64*)ygeta_l(iarg, ntot, dims))
# define ygeta_guint64(iarg, ntot, dims) ((guint64*)ygeta_l(iarg, ntot, dims))
#else
# define ygeta_gint64(iarg, ntot, dims) 1/0
# define ygeta_guint64(iarg, ntot, dims) 1/0
#endif
#define ypush_gint8(dims)   ((gint8*)  ypush_c(dims))
#define ypush_guint8(dims)  ((guint8*) ypush_uc(dims))
#define ypush_gint16(dims)  ((gint16*) ypush_s(dims))
#define ypush_guint16(dims) ((guint16*)ypush_s(dims))
#define ypush_gint32(dims)  ((gint32*) ypush_i(dims))
#define ypush_guint32(dims) ((guint32*)ypush_i(dims))
#if GY_HAVE_INT64
# define ypush_gint64(dims)  ((gint64*) ypush_l(dims))
# define ypush_guint64(dims) ((guint64*) ypush_l(dims))
#else
# define ypush_gint64(dims)  1/0
# define ypush_guint64(dims) 1/0
#endif
