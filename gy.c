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
#include <gdk/gdkx.h>
#include <glib-object.h>

#include "yapi.h"
#include "pstdlib.h"

#include <stdio.h>
#include <fenv.h>
#include <string.h>
#include <signal.h>

static gboolean _gy_debug=1;
#define GY_DEBUG(a) if (_gy_debug) fprintf(stderr, "GY DEBUG: " a);

typedef struct _gy_Object {
  GIBaseInfo * info;
  GObject * object;
  GIRepository * repo;
} gy_Object;
gy_Object* yget_gy_Object(int);
gy_Object* ypush_gy_Object();

typedef struct _gy_Repository {
  GIRepository * repo;
} gy_Repository;
gy_Repository* yget_gy_Repository(int);
gy_Repository* ypush_gy_Repository();

/// TYPELIB

void gy_sa_handler(int sig) {
  const char * ssig="(signal name unknown)";
  switch (sig) {
  case SIGABRT:
    ssig="SIGABRT";
    break;
  case SIGSEGV:
    ssig="SIGSEGV";
    break;
  default:
    break;
  }
  y_errorq("gy action received signal %s", ssig);
}

typedef struct _gy_Typelib {
  GITypelib * typelib;
  gchar * namespace;
  GIRepository * repo;
} gy_Typelib;

void gy_Typelib_free(void *obj);
void gy_Typelib_print(void *obj);
//void gy_Typelib_eval(void *obj, int argc);
void gy_Typelib_extract(void *, char *);
static y_userobj_t gy_Typelib_obj =
  {"gy_Typelib",
   &gy_Typelib_free,
   &gy_Typelib_print,
   NULL, //&gy_Typelib_eval,
   &gy_Typelib_extract,
   NULL  //&uo_ops
  };

void gy_Typelib_free(void *obj) {
  p_free(((gy_Typelib*)obj)->namespace);
  //g_typelib_free(((gy_Typelib*)obj)->typelib);
}

void gy_Typelib_print(void *obj){
  y_print(((gy_Typelib*)obj)->namespace, 0);
}


void
gy_Typelib_extract(void *obj, char * name)
{
  gy_Typelib * tl = (gy_Typelib *) obj;
  GIBaseInfo * info = g_irepository_find_by_name(tl->repo,
						 tl->namespace,
						 name);
  
  if (!info) y_error("No such member");
  gy_Object * o = ypush_gy_Object();
  o->info = info;
  o->repo = tl->repo;
}

gy_Typelib* yget_gy_Typelib(int iarg) {
  return (gy_Typelib*) yget_obj(iarg, &gy_Typelib_obj);
}

gy_Typelib* ypush_gy_Typelib() {
  return (gy_Typelib*) ypush_obj(&gy_Typelib_obj, sizeof(gy_Typelib));
}


/// GYREPOSITORY

//void gy_Repository_free(void *obj);
void gy_Repository_print(void *obj);
//void gy_Repository_eval(void *obj, int argc);
void gy_Repository_extract(void *, char *);
static y_userobj_t gy_Repository_obj =
  {"gy_Repository",
   NULL, //&gy_Repository_free,
   &gy_Repository_print,
   NULL, //&gy_Repository_eval,
   &gy_Repository_extract,
   NULL  //&uo_ops
  };

void gy_Repository_print(void *obj){
  gy_Repository * r = (gy_Repository *) obj;
  gchar ** nspcs = g_irepository_get_loaded_namespaces(r->repo);
  if (!nspcs) {
    y_print("gy_Repository without any loaded namespaces", 0);
    return;
  }
  y_print("gy_Repository with loaded namespaces:", 1);
  for (;*nspcs;++nspcs) y_print(*nspcs, 1);
}

void
gy_Repository_extract(void *obj, char * name)
{
  gy_Repository * r = (gy_Repository *) obj;
  GError * err;

  /// push output
  gy_Typelib * tl = ypush_gy_Typelib();

  tl -> namespace = p_strcpy(name);
  tl -> repo      = r -> repo,
  tl -> typelib   = g_irepository_require (r->repo,
					   name,
					   NULL,
					   0,
					   &err);
  if (!tl->typelib) y_error(err->message);
}

gy_Repository* yget_gy_Repository(int iarg) {
  return (gy_Repository*) yget_obj(iarg, &gy_Repository_obj);
}

gy_Repository* ypush_gy_Repository() {
  return (gy_Repository*) ypush_obj(&gy_Repository_obj, sizeof(gy_Repository));
}


/// GIBASEINFO

void gy_Object_free(void *obj);
void gy_Object_print(void *obj);
void gy_Object_eval(void *obj, int argc);
void gy_Object_extract(void *, char *);
static y_userobj_t gy_Object_obj =
  {"gy_Object",
   &gy_Object_free,
   &gy_Object_print,
   &gy_Object_eval,
   &gy_Object_extract,
   NULL  //&uo_ops
  };

void gy_Object_free(void *obj) {
  gy_Object* o = (gy_Object*) obj;
  if (o->info) g_base_info_unref(o->info);
  if (o->object) {
    // I don't know how reference counting works here...
    // if (GI_IS_STRUCT_INFO(o->info)) g_free(o->object);
    if (GI_IS_OBJECT_INFO(o->info)) g_object_unref(o->object);
  }
}

void gy_Object_print(void *obj) {
  gy_Object* o = (gy_Object*) obj;
  if (o->object) {
    printf("%p", o->object);
    y_print(" is pointer to ", 0);
  }
  y_print("gy object name: ", 0);
  if (!o->info) return;
  y_print(g_base_info_get_name(o->info), 0);
  y_print(", type: ", 0);
  y_print(g_info_type_to_string(g_base_info_get_type (o->info)), 0);
  y_print(", namespace: ", 0);
  y_print(g_base_info_get_namespace (o->info), 0);
}

void
gy_Object_extract(void *obj, char * name)
{
  gy_Object * o = (gy_Object *) obj;

  if (GI_IS_ENUM_INFO(o->info)) {
    
    gint64 wtype=-1;
    gint nc = g_enum_info_get_n_values (o->info);
    GIValueInfo * ci ;
    gint i;
    gboolean tfound=0;
    for (i=0; i<nc; ++i) {
      ci = g_enum_info_get_value(o->info, i);
      if (!strcmp(g_base_info_get_name (ci), name)) {
	wtype=g_value_info_get_value (ci);
	tfound=1;
	break;
      }
    }
    if (tfound) ypush_long(wtype);
    else y_errorq("No such enum value: %s", name);
    return;
  }
  if (GI_IS_OBJECT_INFO(o->info)) {

    printf("Looking for symbol %s in %s\n",
	   name,
	   g_base_info_get_name(o->info));

    GIBaseInfo * info = g_object_info_find_method (o->info, name);

    GIBaseInfo * cur = o->info, *next;
    g_base_info_ref(cur);
    while (!info &&
	   (next = g_object_info_get_parent(cur))) {
      g_base_info_unref(cur);
      cur = next;
      printf("Looking for symbol %s in parent %s\n",
	     name,
	     g_base_info_get_name(cur));
      info = g_object_info_find_method (cur, name);
    }
    if (info) {
      printf("Symbol %s found in %s\n",
	     name,
	     g_base_info_get_name(cur));
      g_base_info_unref(cur);
      gy_Object * out = ypush_gy_Object();
      out->info = info;
      out->repo = o->repo;
      if (g_function_info_get_flags (info) & GI_FUNCTION_IS_METHOD) {
	// a method needs an object!
	out->object=o->object;
	g_object_ref(o->object);
      }
      return;
    }

    static const char const * sigprefix = "signal_";
    int sigprefixlen=7;

    if (!strncmp(name, sigprefix, sigprefixlen)) {
      name += sigprefixlen;

      printf("Looking for signal %s in %s\n",
	     name,
	     g_base_info_get_name(o->info));
  

      gint nc = g_object_info_get_n_signals (o->info);

      printf("%s has %d signals\n", g_base_info_get_name(o->info), nc);

      GISignalInfo * ci=NULL ;
      gint i;

      for (i=0; i<nc; ++i) {
	ci = g_object_info_get_signal(o->info, i);
	if (!strcmp(g_base_info_get_name (ci), name)) {
	  info=ci;
	  break;
	}
	g_base_info_unref(ci);
      }
      if (info) {
	gy_Object * out = ypush_gy_Object();
	out -> info = info;
	out->repo = o->repo;
	return;
      }
      y_errorq("Signal %s not found", name);
    }

    if (!info) {
      y_error("No such method");
    }
  }
}

void gy_Argument_getany(GIArgument * arg, GITypeInfo * info, int iarg) {
  //gint alen;
  //GIArrayType atype;
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  switch(type) {
  case GI_TYPE_TAG_BOOLEAN:
    arg->v_boolean=yarg_true(iarg);
    break;
  case GI_TYPE_TAG_UINT8:
    arg->v_uint8=(gint8)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_INT32:
    arg->v_int32=(gint32)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_DOUBLE:
    arg->v_double=ygets_d(iarg);
    break;
  case GI_TYPE_TAG_UTF8:
    arg->v_string=ygets_q(iarg);
    fprintf(stderr, "argument: %s\n", arg->v_string);
    break;
  case GI_TYPE_TAG_ARRAY:
    //alen=g_type_info_get_array_length(info);
    //atype=g_type_info_get_array_type (info);
    switch (yarg_number(iarg)) {
    case 0:
      if (yarg_string(iarg)) arg->v_pointer=ygeta_q(iarg, 0, 0);
      else if (yarg_nil(iarg)) arg->v_pointer=0;
      else if (yarg_typeid(iarg)==Y_POINTER) arg->v_pointer=ygets_p(iarg);
      else y_error("Unimplemented GIArgument array type");
      break;
    case 1:
      arg->v_pointer=ygeta_l(iarg, 0, 0);
      break;
    case 2:
      arg->v_pointer=ygeta_d(iarg, 0, 0);
      break;
    case 3:
      arg->v_pointer=ygeta_z(iarg, 0, 0);
      break;
    default:
      y_error("Unimplemented GIArgument array type");
    }
    break;
  case GI_TYPE_TAG_INTERFACE:
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_STRUCT:
      {
	GType g_type=
	  g_registered_type_info_get_g_type ( (GIRegisteredTypeInfo *) itrf);
	if (yarg_nil(iarg)) {
	  arg->v_pointer = NULL;
	  break;
	}
	if (g_type_is_a (g_type, G_TYPE_VALUE)) {
	  GValue val=G_VALUE_INIT;
	  // should check type passed from yorick!
	  GObject * obj = yget_gy_Object(iarg)->object;
	  g_value_init (&val, G_TYPE_OBJECT );
	  g_value_set_object(&val, obj);
	  arg->v_pointer = &val;
	  break;
	}
      }
      arg->v_pointer=yget_gy_Object(iarg)->object;
      break;
    case GI_INFO_TYPE_ENUM:
      switch (g_enum_info_get_storage_type (itrf)) {
      case GI_TYPE_TAG_INT32:
	arg->v_int32=(gint32)ygets_l(iarg);
	break;
      case GI_TYPE_TAG_UINT32:
	arg->v_uint32=(guint32)ygets_l(iarg);
	break;
      case GI_TYPE_TAG_INT64:
	arg->v_int64=ygets_l(iarg);
	break;
      default:
	y_errorn("Unimplemented GIArgument enum storage %ld",
		 g_enum_info_get_storage_type (itrf));
      }
      break;
    case GI_INFO_TYPE_OBJECT:
      if (yarg_nil(iarg)) arg->v_pointer=NULL;
      else arg->v_pointer=yget_gy_Object(iarg)->object;
      break;
    default:
      y_errorn("Unimplemented GIArgument interface type %ld",
	      g_base_info_get_type (itrf));
    }
    g_base_info_unref(itrf);
    break;
    
  default:
    y_errorq("Unimplemented GIArgument type: %s",
	     g_type_tag_to_string(type));
  }
}

void gy_Argument_pushany(GIArgument * arg, GITypeInfo * info, gy_Object* o) {
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  gy_Object * outObject=NULL;

  // const char * nspace, * name_with_namespace, * name;
  fprintf (stderr, "ici\n");
  switch(type) {
  case GI_TYPE_TAG_VOID: 
    fprintf (stderr, "la\n");
    if (arg->v_pointer) {
      fprintf(stderr, "not nil\n");
      outObject = ypush_gy_Object();
      outObject -> repo= o -> repo;
      outObject -> object = arg -> v_pointer;

      if (G_IS_OBJECT(outObject -> object)) {
	g_object_ref(outObject -> object);
	outObject->info =
	  g_irepository_find_by_gtype(o -> repo,
				      G_OBJECT_TYPE(outObject->object));
	if (!outObject->info) {
	  y_warn("unable to find object type !");
	  outObject -> info = info;
	  g_base_info_ref(info);
	}
      } else {
	outObject -> info = info;
	g_base_info_ref(info);
      }

    } else    ypush_nil();
    break;
  case GI_TYPE_TAG_BOOLEAN:
    ypush_long(arg->v_boolean);
    break;
  case GI_TYPE_TAG_UINT32:
    ypush_long(arg->v_int32);
    break;
  case GI_TYPE_TAG_DOUBLE:
    fprintf(stderr, "push double... ");
    ypush_double(arg->v_double);
    fprintf(stderr, "%g\n", arg->v_double);
    break;
  case GI_TYPE_TAG_UTF8:
    *ypush_q(0) = p_strcpy(arg->v_string);
    break;
  case GI_TYPE_TAG_INTERFACE:
    fprintf(stderr, "Out argument is interface\n");
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_ENUM:
    fprintf(stderr, "Out argument is enum\n");
      switch (g_enum_info_get_storage_type (itrf)) {
      case GI_TYPE_TAG_INT32:
	ypush_long(arg->v_int32);
	fprintf(stderr, "%d\n", arg->v_int32);
	break;
      case GI_TYPE_TAG_UINT32:
	ypush_long(arg->v_uint32);
	break;
      case GI_TYPE_TAG_INT64:
	ypush_long(arg->v_int64);
	break;
      default:
	y_errorn("Unimplemented output GIArgument enum storage %ld",
		 g_enum_info_get_storage_type (itrf));
      }
    case GI_INFO_TYPE_OBJECT:
      if (!arg -> v_pointer) ypush_nil();
      outObject = ypush_gy_Object();
      outObject -> repo= o -> repo;
      outObject -> object = arg -> v_pointer;
      if (!outObject->object)
	y_warn("object is NULL!");
      g_object_ref(outObject -> object);

      if (G_IS_OBJECT(outObject -> object)) {
	outObject->info =
	  g_irepository_find_by_gtype(o -> repo,
				      G_OBJECT_TYPE(outObject->object));
	if (!outObject->info) {
	  y_warn("unable to find object type !");
	  outObject -> info = info;
	  g_base_info_ref(info);
	}
      } else {
	outObject -> info = info;
	g_base_info_ref(info);
      }

      break;
    default:
      y_errorn("Unimplemented output GIArgument interface type %ld",
	       g_base_info_get_type (itrf));
    }
    break;
  default:
    y_errorq("Unimplemented output GIArgument type: %s",
	     g_type_tag_to_string(type));
  }
}
void
gy_Object_eval(void *obj, int argc)
{
  fprintf(stderr, "in gy_Object_eval\n");
  gy_Object* o = (gy_Object*) obj;
  GError * err = NULL;

  if (GI_IS_STRUCT_INFO(o->info)){
    gy_Object* out = ypush_gy_Object(0);
    if(!o->object)
      out->object=g_malloc0(g_struct_info_get_size (o->info));
    else
      out->object=o->object;
    out->repo=o->repo;
    out->info=o->info;
    g_base_info_ref(o->info);

    int iarg = argc; // last is newly pushed reference
    long index;
    char * name;
    gint i, n;
    GIBaseInfo * cur=NULL;
    GIArgument rarg;
    GITypeInfo * ti;
    gboolean getting=0;

    if (argc==1 && yarg_nil(iarg)) return;

    int lim=0;
    while (iarg > lim) { // 0 is output
      index=yarg_key(iarg);
      fprintf (stderr, "index: %ld\n", index);
      if (index<0) {
	getting=1;
	index=yget_ref(iarg);
	if (index<0) name = ygets_q(iarg);
	else name=yfind_name(index);
      } else {
	getting=0;
	name=yfind_name(index);
      }
      fprintf(stderr, "field: %s\n",name);
      n = g_struct_info_get_n_fields(o->info);
      for (i=0; i<n; ++i) {
	fprintf (stderr, "i=%d/%d\n", i, n);
	cur = g_struct_info_get_field (o->info, i);
	fprintf(stderr, "comparing %s with %s\n", name, g_base_info_get_name(cur));
	if (!strcmp(name, g_base_info_get_name(cur))) {
	  GY_DEBUG("found it\n");
	  ti = g_field_info_get_type(cur);
	  if (getting) { //y_error("Getting");
	    GY_DEBUG("getting\n");
	    long idx = yget_ref(iarg-1);

	    fprintf(stderr, "Getting field... ");
	    gboolean success=g_field_info_get_field(cur, o->object, &rarg);
	    fprintf(stderr, "done.\n");

	    if (!success)
	      y_error("get field failed");

	    gy_Argument_pushany(&rarg, ti, o);
	    yput_global(idx, 0);
	    ++lim;
	    //	    yarg_drop(0);
	  } else {
	    gy_Argument_getany(&rarg, ti, --iarg);
	    if (!g_field_info_set_field(cur, out->object, &rarg))
	      y_error("set field failed");
	  }
	  break;
	}
      }
      if (i==n) y_error("field not found!");
      --iarg;
    }
    
    return;
  }

  if (!GI_IS_CALLABLE_INFO(o->info))
    y_error("Object is not callable");

  gint n_args = g_callable_info_get_n_args(o->info);
  if ((argc != n_args) && !(n_args==0 && argc==1 && yarg_nil(0)))
    y_errorn("function takes %ld arguments", n_args);

  GIArgument * in_args=g_new0(GIArgument,n_args+1);
  GIArgument * out_args=g_new0(GIArgument,n_args);

  GIArgInfo arginfo;
  gint n_in=0, n_out=0, i;

  if (GI_IS_FUNCTION_INFO(o->info) &&
      (g_function_info_get_flags (o->info) & GI_FUNCTION_IS_METHOD)) {
    printf ("Object address: %p\n", o->object);
    printf("Is object: %d\n", G_IS_OBJECT(o->object));
    printf("Object type name: %s\n", G_OBJECT_TYPE_NAME(o->object));
    if (!o -> object) y_error("NULL pointer");
    in_args[0].v_pointer= o -> object;
    ++n_in;
  }

  for (i=0; i<n_args;++i) {
    g_callable_info_load_arg (o->info, i, &arginfo);

    GITypeInfo * argtype = g_arg_info_get_type(&arginfo);

    switch (g_arg_info_get_direction(&arginfo)) {
    case GI_DIRECTION_IN:
      gy_Argument_getany(in_args+n_in,
			 argtype,
			 argc-i-1);
      ++n_in;
      break;
    case GI_DIRECTION_OUT:
      gy_Argument_getany(out_args+n_out,
			 argtype,
			 argc-i-1);
      ++n_out;
      break;
    case GI_DIRECTION_INOUT:
      gy_Argument_getany(in_args+n_in,
			 argtype,
			 argc-i-1);
      ++n_in;
      gy_Argument_getany(out_args+n_out,
			 argtype,
			 argc-i-1);
      ++n_out;
      break;
    default:
      y_error("unknown GI_DIRECTION");
    }

    g_base_info_unref(argtype);
  }

  GIArgument retval;

  fenv_t fenv_in;
  if (feholdexcept(&fenv_in)) y_error("fenv error");


  struct sigaction act, *oldact=NULL;
  act.sa_handler=&gy_sa_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  sigaction(SIGABRT, &act, oldact);
  sigaction(SIGSEGV, &act, oldact);

  gboolean success = g_function_info_invoke (o->info,
					     in_args,
					     n_in,
					     out_args,
					     n_out,
					     &retval,
					     &err);
  sigaction(SIGABRT, oldact, NULL);

  fesetenv(&fenv_in);
  if (!success) {
    fprintf(stderr, "here\n");
    y_error(err->message);
  }

  printf("Function %s sucessfully called\n", g_base_info_get_name(o->info));

  if (n_out)
    y_warn("unimplemented: positional out arguments");

  GITypeInfo * retinfo = g_callable_info_get_return_type(o->info);

  gy_Argument_pushany(&retval, retinfo, o);

  /*
  if (g_function_info_get_flags (o->info) & GI_FUNCTION_IS_CONSTRUCTOR) {
    gy_Object*out = yget_gy_Object(0);
    g_base_info_unref(out->info);
    const char * nspace = g_base_info_get_namespace (o->info);
    printf("Namespace: %s\n", nspace);
    const char * name_with_namespace = G_OBJECT_TYPE_NAME(out->object);
    printf("Name with namespace: %s\n", name_with_namespace);
    const char * name = name_with_namespace + strlen(nspace);
    printf("Name without namespace: %s\n", name);

    out->info = g_irepository_find_by_name(NULL,
					   nspace,
					   name);

    g_base_info_ref(out->info);
  }
  */
  g_base_info_unref(retinfo); 
  g_free (in_args);
  g_free (out_args);

}

gy_Object* yget_gy_Object(int iarg) {
  return (gy_Object*) yget_obj(iarg, &gy_Object_obj);
}

gy_Object* ypush_gy_Object() {
  return (gy_Object*) ypush_obj(&gy_Object_obj, sizeof(gy_Object));
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
Y_gy_require(int argc)
{
  GError * err;

  /// process input
  ystring_t namespace = ygets_q(argc-1);

  /// push output
  gy_Typelib * tl = ypush_gy_Typelib();

  tl -> namespace = p_strcpy(namespace);
  tl -> repo      = NULL,
  tl -> typelib   = g_irepository_require (NULL,
					   namespace,
					   NULL,
					   0,
					   &err);
  if (!tl->typelib) y_error(err->message);
}

void
Y_gy_list_namespace(int argc) {
  if (argc!=1) y_error("gy_list_namespace takes exactly 1 argument");
  char * name =NULL;
  gint i, ninfos;
  GError * err=NULL;
  GITypelib * typelib=NULL;
  gy_Typelib * tl =NULL;
  GIRepository * repo=NULL;
  if (yarg_string(0)) {
    name=ygets_q(0);
    typelib=g_irepository_require (NULL,
				   name,
				   NULL,
				   0,
				   &err);
    if (!typelib) y_error(err->message);
  } else {
    tl = yget_gy_Typelib(0);
    name = tl -> namespace;
    repo = tl -> repo;
  }

  ninfos = g_irepository_get_n_infos (repo, name);
  printf("Namespace %s has %d infos\n", name, ninfos);
  GIBaseInfo * info;
  for (i=0; i<ninfos; ++i) {
    info = g_irepository_get_info(repo, name, i);
    printf("  Info type: %s, name: %s\n",
	   g_info_type_to_string(g_base_info_get_type (info)),
	   g_base_info_get_name (info));
    g_base_info_unref(info);
  }
}

void
Y_gy_list_object(int argc) {
  gy_Object * o = yget_gy_Object(0);
    printf("gy object name: %s, type: %s, namespace: %s\n",
	   g_base_info_get_name(o->info),
	   g_info_type_to_string(g_base_info_get_type (o->info)),
	   g_base_info_get_namespace (o->info));
  if (o->object) {
    printf("with object at %p\n", o->object);
    printf("Object type: %s\n", G_OBJECT_TYPE_NAME(o->object));
  }
  if (GI_IS_STRUCT_INFO(o->info)) {
    gint i, n = g_struct_info_get_n_fields(o->info);
    GIBaseInfo * cur=NULL;
    for (i=0; i<n; ++i) {
      cur = g_struct_info_get_field (o->info, i);
      printf("Field #%d=%s\n", i, g_base_info_get_name(cur));
      g_base_info_unref(cur);
    }
    n = g_struct_info_get_n_methods(o->info);
    for (i=0; i<n; ++i) {
      cur = g_struct_info_get_method (o->info, i);
      printf("Field #%d=%s\n", i, g_base_info_get_name(cur));
      g_base_info_unref(cur);
    }
    
  } else if (GI_IS_ENUM_INFO(o->info)) {
    gint nc = g_enum_info_get_n_values (o->info);
    GIValueInfo * ci ;
    gint i;
    for (i=0; i<nc; ++i) {
      ci = g_enum_info_get_value(o->info, i);
      printf("Enum name: %s\n", g_base_info_get_name (ci));
    }
  } else if (GI_IS_OBJECT_INFO(o->info)) {

    printf("Available vfuncs:\n");
    int i, n = g_object_info_get_n_vfuncs (o->info);
    printf("Object has %d vfuncs(s)\n", n);
    GIFunctionInfo * gmi;
    for (i=0; i<n; ++i) {
      gmi=g_object_info_get_vfunc(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }

    printf("Available methods:\n");
    n = g_object_info_get_n_methods (o->info);
    printf("Object has %d methods(s)\n", n);
    for (i=0; i<n; ++i) {
      gmi=g_object_info_get_method(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }

    printf("Available signals:\n");
    n = g_object_info_get_n_signals (o->info);
    printf("Object has %d signals(s)\n", n);
    for (i=0; i<n; ++i) {
      gmi=g_object_info_get_signal(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }

    if (g_object_info_get_fundamental (o->info))
      printf("Object is fundamental\n");
    else {
      gmi = g_object_info_get_parent(o->info);
      if (gmi) {
	printf("Object parent: %s\n", g_base_info_get_name(gmi));
	g_base_info_unref(gmi);
      } else printf("Object has no parent\n");
    }
  }
}

void gy_callback(GObject* obj, void* data, ...) {
  const char * cmd = g_object_get_data(obj, "gy_callback");
  printf("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  g_object_set_data(obj, "gy_data", data);
  fprintf(stderr, "Event received: %d\n", ((GdkEventAny*) data )-> type);
  long dims[2]={1,1};
  *ypush_q(dims) = p_strcpy(cmd);
  yexec_include(0,1);
  yarg_drop(0);
}

void
Y_gy_signal_connect(int argc) {
  gy_Object * o = yget_gy_Object(argc-1);
  ystring_t sig = ygets_q(argc-2);
  ystring_t cmd = p_strcpy(ygets_q(argc-3));
  // this will work as long as object is the first parameter
  // in the callback signature.
  g_object_set_data(o->object, "gy_callback", cmd);
  g_signal_connect (o -> object,
		    sig,
		    G_CALLBACK(&gy_callback),
		    NULL);
  ypush_nil();
}

//// hack, should be done from gi
void
Y_gy_xid(int argc) {
  gy_Object * o = yget_gy_Object(0);
  GObject * ptr = o->object;
  if (!G_IS_OBJECT(ptr)) y_error ("Not an object");
  GValue val = G_VALUE_INIT;
  g_value_init(&val, G_TYPE_OBJECT);
  g_object_get_property(ptr, "window", &val);
  GdkWindow * win = g_value_get_object(&val);
  if (!win) y_error("Cannot get Gdk window");
  ypush_long(GDK_WINDOW_XID(win));
}

void
Y_gy_GdkEventButton(int argc) {
  gy_Object * o = yget_gy_Object(0);
  GObject * ptr = o->object;
  if (!G_IS_OBJECT(ptr)) y_error ("Not an object");
  gy_Object * out = ypush_gy_Object();
  out -> repo = o -> repo;
  out -> info = g_irepository_find_by_name(out->repo,
					   "Gdk",
					   "EventButton");
  out -> object = g_object_get_data(ptr, "gy_data");

}
