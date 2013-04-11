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
    along with Gyoto.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gobject-introspection-1.0/girepository.h>
#include "yapi.h"
#include "pstdlib.h"

#include <stdio.h>
#include <fenv.h>
#include <string.h>

typedef struct _gy_Object {
  GIBaseInfo * info;
  GObject * object;
} gy_Object;
gy_Object* yget_gy_Object(int);
gy_Object* ypush_gy_Object();

/// TYPELIB

typedef struct _gy_Typelib {
  GITypelib * typelib;
  gchar * namespace;
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
}

void gy_Typelib_print(void *obj){
  y_print(((gy_Typelib*)obj)->namespace, 0);
}


void
gy_Typelib_extract(void *obj, char * name)
{
  gy_Typelib * tl = (gy_Typelib *) obj;
  GIBaseInfo * info = g_irepository_find_by_name(NULL,
						 tl->namespace,
						 name);
  
  if (!info) y_error("No such member");
  gy_Object * o = ypush_gy_Object();
  o->info = info;
}

gy_Typelib* yget_gy_Typelib(int iarg) {
  return (gy_Typelib*) yget_obj(iarg, &gy_Typelib_obj);
}

gy_Typelib* ypush_gy_Typelib() {
  return (gy_Typelib*) ypush_obj(&gy_Typelib_obj, sizeof(gy_Typelib));
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
  if (o->object) g_object_unref(o->object);
}

void gy_Object_print(void *obj) {
  gy_Object* o = (gy_Object*) obj;
  y_print("gy object name: ", 0);
  y_print(g_base_info_get_name(o->info), 0);
  y_print(", type: ", 0);
  y_print(g_info_type_to_string(g_base_info_get_type (o->info)), 0);
  y_print(", namespace: ", 0);
  y_print(g_base_info_get_namespace (o->info), 0);
  if (o->object) {
    y_print("with object at ", 0);
    printf("%p", o->object);
  }
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
	   strcmp(g_base_info_get_name(cur), "InitiallyUnowned")) {
      next = g_object_info_get_parent(cur);
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

      gboolean tfound=0;
      for (i=0; i<nc; ++i) {
	ci = g_object_info_get_signal(o->info, i);
	printf("Checking against %s... ", g_base_info_get_name (ci));
	if (!strcmp(g_base_info_get_name (ci), name)) {
	  printf("yes.\n");
	  info=ci;
	  break;
	}
	printf("no.\n");
	g_base_info_unref(ci);
      }
      if (info) {
	printf("Pushing object... ");
	gy_Object * out = ypush_gy_Object();
	printf("putting value... ");
	out -> info = info;
	printf("done.\n");
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
  gint alen;
  GIArrayType atype;
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  switch(type) {
  case GI_TYPE_TAG_BOOLEAN:
    arg->v_boolean=yarg_true(iarg);
    break;
  case GI_TYPE_TAG_INT32:
    arg->v_int32=(gint32)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_UTF8:
    arg->v_string=ygets_q(iarg);
    break;
  case GI_TYPE_TAG_ARRAY:
    alen=g_type_info_get_array_length(info);
    atype=g_type_info_get_array_type (info);
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
    case GI_INFO_TYPE_ENUM:
      switch (g_enum_info_get_storage_type (itrf)) {
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
      arg->v_pointer=yget_gy_Object(iarg)->object;
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

void gy_Argument_pushany(GIArgument * arg, GITypeInfo * info) {
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  gy_Object * outObject=NULL;

  // const char * nspace, * name_with_namespace, * name;

  switch(type) {
  case GI_TYPE_TAG_VOID: 
    ypush_nil();
    break;
  case GI_TYPE_TAG_UINT32:
    ypush_long(arg->v_int32);
    break;
  case GI_TYPE_TAG_INTERFACE:
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_ENUM:
      switch (g_enum_info_get_storage_type (itrf)) {
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
      outObject = ypush_gy_Object();

      outObject -> object = arg -> v_pointer;
      g_object_ref(outObject -> object);
      printf("v_pointer: %p\n", arg -> v_pointer);

      if (G_IS_OBJECT(outObject -> object)) {
	outObject->info =
	  g_irepository_find_by_gtype(NULL,
				      G_OBJECT_TYPE(outObject->object));
      } else {
	outObject -> info = info;
	g_base_info_ref(info);
      }

      printf("object info name: %s\n", g_base_info_get_name(outObject->info));

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
  gy_Object* o = (gy_Object*) obj;
  GError * err = NULL;
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
  gboolean success = g_function_info_invoke (o->info,
					     in_args,
					     n_in,
					     out_args,
					     n_out,
					     &retval,
					     &err);
  fesetenv(&fenv_in);
  if (!success) y_error(err->message);

  printf("Function %s sucessfully called\n", g_base_info_get_name(o->info));

  if (n_out)
    y_warn("unimplemented: positional out arguments");

  GITypeInfo * retinfo = g_callable_info_get_return_type(o->info);

  gy_Argument_pushany(&retval, retinfo);

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
  g_type_init();
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
  tl -> typelib   = g_irepository_require (NULL,
					   namespace,
					   NULL,
					   0,
					   &err);
  if (!tl->typelib) y_error(err->message);
}

Y_gy_list_namespace(int argc) {
  char * name = ygets_q(0);
  gint i, ninfos;
  ninfos = g_irepository_get_n_infos (NULL, "Gtk");
  printf("Gtk has %d infos\n", ninfos);
  GIBaseInfo * info;
  for (i=0; i<ninfos; ++i) {
    info = g_irepository_get_info(NULL, "Gtk", i);
    printf("Info type: %s, name: %s\n",
	   g_info_type_to_string(g_base_info_get_type (info)),
	   g_base_info_get_name (info));
    g_base_info_unref(info);
  }
}

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
  if (GI_IS_ENUM_INFO(o->info)) {
    gint64 wtype=-1;
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
    else if (!strcmp(g_base_info_get_name(o->info), "InitiallyUnowned"))
      printf("Object is InitiallyUnowned\n");
    else {
      gmi = g_object_info_get_parent(o->info);
      if (gmi) {
	printf("Object parent: %s\n", g_base_info_get_name(gmi));
	g_base_info_unref(gmi);
      } else printf("Object has no parent\n");
    }
  }
}

void gy_callback(GObject* obj, ...) {
  const char * cmd = g_object_get_data(obj, "gy_callback");
  printf("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
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
