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

  if (GI_IS_CONSTANT_INFO(o->info)) {
    GY_DEBUG("Extracted object is constant\n");
    GIArgument rarg;
    GITypeInfo * retinfo = g_constant_info_get_type(o->info);
    //gint retval = // useless so far: size of the constant
    g_constant_info_get_value(o->info, &rarg);
    yarg_drop(1);
    gy_Argument_pushany(&rarg, retinfo, o);
    g_base_info_unref(retinfo);
  }

}

gy_Typelib* yget_gy_Typelib(int iarg) {
  return (gy_Typelib*) yget_obj(iarg, &gy_Typelib_obj);
}

gy_Typelib* ypush_gy_Typelib() {
  return (gy_Typelib*) ypush_obj(&gy_Typelib_obj, sizeof(gy_Typelib));
}

/// GIBASEINFO

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
    if (o->info && GI_IS_OBJECT_INFO(o->info)) g_object_unref(o->object);
  }
}

void gy_Object_print(void *obj) {
  gy_Object* o = (gy_Object*) obj;
  char spointer[40] = {0};
  if (o->object) {
    sprintf(spointer, "%p", o->object);
    y_print(spointer, 0);
    y_print(" is pointer to ", 0);
  }
  y_print("gy object name: ", 0);
  if (!o->info) {
    y_print("(unknown)", 0);
    return;
  }
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

  if (!o->info) y_error("Object has no type information");
  
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

  gboolean isstruct = GI_IS_STRUCT_INFO(o->info),
    isobject = GI_IS_OBJECT_INFO(o->info),
    isitrf = GI_IS_INTERFACE_INFO(o->info);

  if (isstruct || isobject || isitrf) {
    
    GIBaseInfo * info =NULL;

    GY_DEBUG("Looking for symbol %s in %s\n",
	   name,
	   g_base_info_get_name(o->info));
    if (isobject)
      info = g_object_info_find_method (o->info, name);
    else if (isitrf)
      info = g_interface_info_find_method (o->info, name);
    else
      info = g_struct_info_find_method (o->info, name);

    GIBaseInfo * cur = o->info, *next;
    g_base_info_ref(cur);
    while (!info && isobject &&
	   (next = g_object_info_get_parent(cur))) {
      g_base_info_unref(cur);
      cur = next;
      GY_DEBUG("Looking for symbol %s in parent %s\n",
	     name,
	     g_base_info_get_name(cur));
      info = g_object_info_find_method (cur, name);
    }
    if (info) {
      GY_DEBUG("Symbol %s found in %s\n",
	     name,
	     g_base_info_get_name(cur));
      g_base_info_unref(cur);
      gy_Object * out = ypush_gy_Object();
      out->info = info;
      out->repo = o->repo;
      if (g_function_info_get_flags (info) & GI_FUNCTION_IS_METHOD) {
	// a method needs an object!
	out->object=o->object;
	if (isobject) g_object_ref(o->object);
      }
      return;
    }

    if (isobject) {
      static const char const * sigprefix = "signal_";
      int sigprefixlen=7;

      if (!strncmp(name, sigprefix, sigprefixlen)) {
	name += sigprefixlen;

	GY_DEBUG("Looking for signal %s in %s\n",
		 name,
		 g_base_info_get_name(o->info));
  

	gint nc = g_object_info_get_n_signals (o->info);

	GY_DEBUG("%s has %d signals\n", g_base_info_get_name(o->info), nc);

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
  case GI_TYPE_TAG_VOID:
    if (yarg_nil(iarg)) arg->v_pointer=NULL;
    else y_error("unimplemented void... type (?!)");
    break;
  case GI_TYPE_TAG_BOOLEAN:
    arg->v_boolean=yarg_true(iarg);
    break;
  case GI_TYPE_TAG_UINT8:
    arg->v_uint8=(gint8)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_INT32:
    arg->v_int32=(gint32)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_UINT32:
    arg->v_int32=(guint32)ygets_l(iarg);
    break;
  case GI_TYPE_TAG_DOUBLE:
    arg->v_double=ygets_d(iarg);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    arg->v_string=ygets_q(iarg);
    GY_DEBUG( "argument: %s\n", arg->v_string);
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
    case GI_INFO_TYPE_CALLBACK:
      arg->v_pointer=yget_gy_Object(iarg)->object;
      break;
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
    case GI_INFO_TYPE_FLAGS:
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
  switch(type) {
  case GI_TYPE_TAG_VOID:
    GY_DEBUG("Out argument is void\n")
    /*
    if (arg->v_pointer) {
      GY_DEBUG("Out argument is not (nil)\n");
      outObject = ypush_gy_Object();
      outObject -> repo= o -> repo;
      outObject -> object = arg -> v_pointer;

      if (G_IS_OBJECT(outObject -> object)) {
	GY_DEBUG("plut\n");
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
	GY_DEBUG("plat\n");
	outObject -> info = info;
	g_base_info_ref(info);
	GY_DEBUG("plut\n");
      }

    } else {
      GY_DEBUG("Out argument is (nil)\n");
    */
      ypush_nil();
    //}
    break;
  case GI_TYPE_TAG_BOOLEAN:
    ypush_long(arg->v_boolean);
    break;
  case GI_TYPE_TAG_INT8:
    ypush_long(arg->v_int8);
    break;
  case GI_TYPE_TAG_UINT8:
    ypush_long(arg->v_uint8);
    break;
  case GI_TYPE_TAG_INT16:
    ypush_long(arg->v_int16);
    break;
  case GI_TYPE_TAG_UINT16:
    ypush_long(arg->v_uint16);
    break;
  case GI_TYPE_TAG_INT32:
    ypush_long(arg->v_int32);
    break;
  case GI_TYPE_TAG_UINT32:
    ypush_long(arg->v_uint32);
    break;
  case GI_TYPE_TAG_INT64:
    ypush_long(arg->v_int64);
    break;
  case GI_TYPE_TAG_UINT64:
    ypush_long(arg->v_uint64);
    break;
  case GI_TYPE_TAG_DOUBLE:
    GY_DEBUG("push double... ");
    ypush_double(arg->v_double);
    GY_DEBUG("%g\n", arg->v_double);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    *ypush_q(0) = p_strcpy(arg->v_string);
    break;
  case GI_TYPE_TAG_INTERFACE:
    GY_DEBUG("Out argument is interface\n");
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_ENUM:
    case GI_INFO_TYPE_FLAGS:
    GY_DEBUG("Out argument is enum\n");
      switch (g_enum_info_get_storage_type (itrf)) {
      case GI_TYPE_TAG_INT32:
	ypush_long(arg->v_int32);
	GY_DEBUG("%d\n", arg->v_int32);
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
      break;
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
	  GY_DEBUG("unable to find object type !");
	  //outObject -> info = info;
	  //g_base_info_ref(info);
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

int
yarg_gy_Object(int iarg)
{
  return yget_obj(iarg,0)==gy_Object_obj.type_name;
}

void
gy_Object_eval(void *obj, int argc)
{
  GY_DEBUG("in gy_Object_eval\n");
  gy_Object* o = (gy_Object*) obj;
  GError * err = NULL;

  if (!o -> info)
    y_error("Object lacks type information. "
	    "Please cast it appropriately");

  if (GI_IS_STRUCT_INFO(o->info)){
    gy_Object* out = ypush_gy_Object(0);
    if(!o->object) {
      if (yarg_gy_Object(argc))
	out -> object = yget_gy_Object(argc--) -> object;
      else
	out -> object = g_malloc0(g_struct_info_get_size (o->info));
    } else
      out -> object = o->object;
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
      GY_DEBUG("index: %ld\n", index);
      if (index<0) {
	getting=1;
	index=yget_ref(iarg);
	if (index<0) name = ygets_q(iarg);
	else name=yfind_name(index);
      } else {
	getting=0;
	name=yfind_name(index);
      }
      GY_DEBUG("field: %s\n",name);
      n = g_struct_info_get_n_fields(o->info);
      for (i=0; i<n; ++i) {
	GY_DEBUG("i=%d/%d\n", i, n);
	cur = g_struct_info_get_field (o->info, i);
	GY_DEBUG("comparing %s with %s\n", name, g_base_info_get_name(cur));
	if (!strcmp(name, g_base_info_get_name(cur))) {
	  GY_DEBUG("found it\n");
	  ti = g_field_info_get_type(cur);
	  if (getting) { //y_error("Getting");
	    GY_DEBUG("getting\n");
	    long idx = yget_ref(iarg-1);

	    GY_DEBUG("Getting field... ");
	    gboolean success=g_field_info_get_field(cur, o->object, &rarg);
	    GY_DEBUG("done.\n");

	    if (!success)
	      y_error("get field failed");

	    gy_Argument_pushany(&rarg, ti, o);
	    GY_DEBUG("pushing result... ");
	    yput_global(idx, 0);
	    GY_DEBUG("done.\n");
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

  gboolean
    isobject = GI_IS_OBJECT_INFO(o->info),
    isitrf   = GI_IS_INTERFACE_INFO(o->info);

  if (isobject || isitrf) {
    gy_Object* out = ypush_gy_Object(0);
    if(!o->object) {
      if (yarg_gy_Object(argc))
	out -> object = yget_gy_Object(argc--) -> object;
      /* else if (GI_IS_OBJECT_INFO(o->info)) { */
      /* 	/\* find new() method *\/ */
      /* 	GY_DEBUG("Looking for symbol \"new\" in %s\n", */
      /* 		 g_base_info_get_name(o->info)); */
      /* 	GIFunctionInfo * newinfo = */
      /* 	  g_object_info_find_method (o->info, "new"); */
      /* 	if (!newinfo) */
      /* 	  y_errorq("\"new\" method not found for object type \"%s\"", */
      /* 		   g_base_info_get_name(o->info)); */
      /*  } */
      else y_error("Object is not callable");
    } else
      out -> object = o->object;
    out->info=o->info;
    g_base_info_ref(o->info);
    g_object_ref(out->object);
    out->repo=o->repo;

    /* try setting / getting properties */
    int iarg = argc; // last is newly pushed reference
    long index;
    char * name;
    gint i, n;
    GIPropertyInfo * cur=NULL;
    GITypeInfo * ti;
    gboolean getting=0;
    GParamFlags pf;

    if (argc==1 && yarg_nil(iarg)) return;

    int lim=0;
    while (iarg > lim) { // 0 is output
      index=yarg_key(iarg);
      GY_DEBUG("index: %ld\n", index);
      if (index<0) {
	getting=1;
	index=yget_ref(iarg);
	if (index<0) name = ygets_q(iarg);
	else name=yfind_name(index);
      } else {
	getting=0;
	name=yfind_name(index);
      }
      GY_DEBUG("property: %s\n",name);
      if (isobject) n = g_object_info_get_n_properties(o->info);
      else  n = g_interface_info_get_n_properties(o->info);
      GY_DEBUG("object has %d properties.\n", n);
      for (i=0; i<n; ++i) {
	GY_DEBUG("i=%d/%d\n", i, n);
	cur = isobject?
	  g_object_info_get_property (o->info, i):
	  g_interface_info_get_property (o->info, i);
	GY_DEBUG("comparing %s with %s\n", name, g_base_info_get_name(cur));
	if (!strcmp(name, g_base_info_get_name(cur))) {
	  GY_DEBUG("found it\n");
	  ti = g_property_info_get_type(cur);
	  pf = g_property_info_get_flags (cur);
	  GITypeTag tag = g_type_info_get_tag(ti);
	  if (getting) { //y_error("Getting");
	    if (!(pf & G_PARAM_READABLE)) y_error("property is not readable");
	    GY_DEBUG("getting\n");
	    long idx = yget_ref(iarg-1);

	    GY_DEBUG("Getting property... ");
	    GIBaseInfo * outinfo=NULL;
	    if (tag != GI_TYPE_TAG_INTERFACE)
	      y_error ("fix me: only properties of type object supported yet");

	    outinfo = g_type_info_get_interface (ti);
	    if (!GI_IS_OBJECT_INFO(outinfo)) {
	      g_base_info_unref(outinfo);
	      y_error ("fix me: only properties of type object supported yet");
	    }

	    GValue val=G_VALUE_INIT;
	    g_value_init(&val, G_TYPE_OBJECT);
	    g_object_get_property(o->object, name, &val);

	    GObject * prop=g_value_get_object(&val);
	    if (!prop) y_error("get property failed");

	    GY_DEBUG("pushing result... ");
	    ypush_check(1);
	    gy_Object * out = ypush_gy_Object();
	    yput_global(idx, 0);
	    yarg_swap(0, 1); // keep o on top
	    GY_DEBUG("done.\n");

	    out->info=outinfo;
	    out->object=prop;
	    out->repo=o->repo;
	    
	    ++lim;
	    //	    yarg_drop(0);
	  } else {
	    if (!(pf & G_PARAM_WRITABLE)) y_error("property is not writable");
	    else y_error("fix me: setting properties not implemented");
	  }
	  break;
	}
      }
      if (i==n) y_error("property not found!");
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
    GY_DEBUG("Object address: %p\n", o->object);
    GY_DEBUG("Is object: %d\n", G_IS_OBJECT(o->object));
    GY_DEBUG("Object type name: %s\n", G_OBJECT_TYPE_NAME(o->object));
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
    GY_DEBUG("here\n");
    y_error(err->message);
  }

  GY_DEBUG("Function %s successfully called\n", g_base_info_get_name(o->info));

  if (n_out)
    y_warn("unimplemented: positional out arguments");

  GITypeInfo * retinfo = g_callable_info_get_return_type(o->info);

  gy_Argument_pushany(&retval, retinfo, o);

  /*
  if (g_function_info_get_flags (o->info) & GI_FUNCTION_IS_CONSTRUCTOR) {
    gy_Object*out = yget_gy_Object(0);
    g_base_info_unref(out->info);
    const char * nspace = g_base_info_get_namespace (o->info);
    GY_DEBUG("Namespace: %s\n", nspace);
    const char * name_with_namespace = G_OBJECT_TYPE_NAME(out->object);
    GY_DEBUG("Name with namespace: %s\n", name_with_namespace);
    const char * name = name_with_namespace + strlen(nspace);
    GY_DEBUG("Name without namespace: %s\n", name);

    out->info = g_irepository_find_by_name(NULL,
					   nspace,
					   name);

    g_base_info_ref(out->info);
  }
  */
  GY_DEBUG("g_base_info_unref(retinfo)... ");
  g_base_info_unref(retinfo); 
  GY_DEBUG(" done.\n");
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
  if (!o->info) printf("object without type information.\n");
  printf("gy object name: %s, type: %s, namespace: %s\n",
	 g_base_info_get_name(o->info),
	 g_info_type_to_string(g_base_info_get_type (o->info)),
	 g_base_info_get_namespace (o->info));

  gboolean isobject=GI_IS_OBJECT_INFO(o->info);
  gboolean isstruct=GI_IS_STRUCT_INFO(o->info);
  gboolean isitrf=GI_IS_INTERFACE_INFO(o->info);
  gboolean isenum=GI_IS_ENUM_INFO(o->info);

  char * itype="n unknown";
  if (isobject) itype=" GObject";
  else if (isitrf) itype=" GInterface";
  else if (isstruct) itype=" C structure";
  else if (isenum) itype="n enumeration";

  printf("Object is a%s.\n", itype);

  gint i=0, n=0;

  if (o->object) {
    printf("with object at %p\n", o->object);
    if (isobject) printf("Object type: %s\n", G_OBJECT_TYPE_NAME(o->object));
  }

  /* values */
  if (isenum) {
    n = g_enum_info_get_n_values (o->info);
    GIValueInfo * ci ;
    printf ("Object has %d enum values\n", n);
    for (i=0; i<n; ++i) {
      ci = g_enum_info_get_value(o->info, i);
      printf("  Enum name: %s, value: %ld\n",
	     g_base_info_get_name (ci),
	     g_value_info_get_value(ci));
    }
  }

  /* fields */
  if (isstruct || isobject) {
    n = isstruct?
      g_struct_info_get_n_fields(o->info):
      g_object_info_get_n_fields(o->info);
    GIBaseInfo * cur=NULL;
    printf ("Object has %d fields\n", n);
    for (i=0; i<n; ++i) {
      cur = isstruct?
	g_struct_info_get_field (o->info, i):
	g_object_info_get_field (o->info, i);
      printf("  Field #%d=%s\n", i, g_base_info_get_name(cur));
      g_base_info_unref(cur);
    }
  }

  /* properties */
  if (isitrf || isobject) {
    n = isitrf?
      g_interface_info_get_n_properties(o->info):
      g_object_info_get_n_properties(o->info);
    GIPropertyInfo * cur=NULL;
    printf ("Object has %d properties\n", n);
    for (i=0; i<n; ++i) {
      cur = isitrf?
	g_interface_info_get_property (o->info, i):
	g_object_info_get_property (o->info, i);
      printf("  %s\n", g_base_info_get_name(cur));
      g_base_info_unref(cur);
    }
  }

  /* methods */
  if (isstruct || isobject || isitrf || isenum) {
    if (isstruct) n = g_struct_info_get_n_methods(o->info);
    else if (isobject) n = g_object_info_get_n_methods(o->info);
    else if (isitrf) n = g_interface_info_get_n_methods(o->info);
    else if (isenum) n = g_enum_info_get_n_methods(o->info);
    printf("Object has %d methods\n", n);
    GIBaseInfo * cur=NULL;
    for (i=0; i<n; ++i) {
      if (isstruct) cur = g_struct_info_get_method (o->info, i);
      else if (isobject) cur = g_object_info_get_method (o->info, i);
      else if (isitrf) cur = g_interface_info_get_method (o->info, i);
      else if (isenum) cur = g_enum_info_get_method (o->info, i);
      printf("  %s\n", g_base_info_get_name(cur));
      g_base_info_unref(cur);
    }
  }

  /* signals */
  if (isobject || isitrf) {
    n = isobject?
      g_object_info_get_n_signals (o->info):
      g_interface_info_get_n_signals (o->info);
    printf("Object has %d signals(s)\n", n);
    GIBaseInfo * gmi=NULL;
    for (i=0; i<n; ++i) {
      gmi=isobject?
	g_object_info_get_signal(o->info, i):
	g_interface_info_get_signal(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }
  }

  /* vfuncs */
  if (isobject || isitrf) {

    n = isobject?
      g_object_info_get_n_vfuncs (o->info):
      g_interface_info_get_n_vfuncs (o->info);
    printf("Object has %d vfunc(s)\n", n);
    GIFunctionInfo * gmi;
    for (i=0; i<n; ++i) {
      gmi=isobject?
	g_object_info_get_vfunc(o->info, i):
	g_interface_info_get_vfunc(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }
  }

  /* constants */
  if (isobject || isitrf) {

    n = isobject?
      g_object_info_get_n_constants (o->info):
      g_interface_info_get_n_constants (o->info);
    printf("Object has %d constant(s)\n", n);
    GIConstantInfo * gmi;
    for (i=0; i<n; ++i) {
      gmi=isobject?
	g_object_info_get_constant(o->info, i):
	g_interface_info_get_constant(o->info, i);
      printf("  %s\n", g_base_info_get_name (gmi));
      g_base_info_unref(gmi);
    }
  }

  if (isobject) {
    if (g_object_info_get_fundamental (o->info))
      printf("Object is fundamental\n");
    else {
      GIBaseInfo * gmi=NULL;
      gmi = g_object_info_get_parent(o->info);
      if (gmi) {
	printf("Object parent: %s\n", g_base_info_get_name(gmi));
	g_base_info_unref(gmi);
      } else printf("Object has no parent\n");
    }
  }
}

////  generic callbacks
static gboolean gy_callback_retval;

void gy_callback0(void* arg1, gy_signal_data* sd) {
  GY_DEBUG("in gy_callback0()\n");
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  //  void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(4);

  long idx1=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    idx1 = yget_global(var1, 0);

    gy_Object * o1 = ypush_gy_Object();
    yput_global(idx1, 0);

    o1 -> object = arg1;
    o1 -> repo = repo;
    g_object_ref(o1 -> object);
    o1 -> info =
	  g_irepository_find_by_gtype(o1 -> repo,
				      G_OBJECT_TYPE(o1 -> object));

    const char * fmt = "noop, %s (%s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)+strlen(var1)));
    sprintf(buf, fmt, cmd, var1);
    cmd=buf;
    ndrops+=1;
  }

  long dims[2]={1,1};
  *ypush_q(dims) = p_strcpy(cmd);
  ++ndrops;
  if (buf) p_free(buf);
  yexec_include(0,1);
  yarg_drop(ndrops);

}

gboolean gy_callback0_bool(void* arg1, gy_signal_data* sd) {
  gy_callback_retval=0;
  gy_callback0(arg1, sd) ;
  return gy_callback_retval;
}

void gy_callback1(void* arg1, void* arg2, gy_signal_data* sd) {
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  //void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(4);

  long idx1=0, idx2=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    const char * var2 = "__gy_callback_var2";
    idx1 = yget_global(var1, 0);
    idx2 = yget_global(var2, 0);

    gy_Object * o1 = ypush_gy_Object();
    yput_global(idx1, 0);
    gy_Object * o2 = ypush_gy_Object();
    yput_global(idx2, 0);

    o1 -> object = arg1;
    o1 -> repo = repo;
    g_object_ref(o1 -> object);
    o1 -> info =
	  g_irepository_find_by_gtype(o1 -> repo,
				      G_OBJECT_TYPE(o1 -> object));

    o2 -> object = arg2;
    o2 -> repo = repo;


    const char * fmt = "noop, %s (%s, %s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)+strlen(var1)+strlen(var2)));
    sprintf(buf, fmt, cmd, var1, var2);
    cmd=buf;
    ndrops+=2;
  }

  long dims[2]={1,1};
  *ypush_q(dims) = p_strcpy(cmd);
  ++ndrops;
  if (buf) p_free(buf);
  yexec_include(0,1);
  yarg_drop(ndrops);

}

gboolean gy_callback1_bool(void* arg1, void* arg2, gy_signal_data* sd) {
  gy_callback_retval=0;
  gy_callback1(arg1, arg2, sd) ;
  return gy_callback_retval;
}

void
Y_gy_return(int argc)
{
  gy_callback_retval=ygets_l(0);
}

void gy_callback2(void* arg1, void* arg2, void* arg3, gy_signal_data* sd) {
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  // void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(5);

  long idx1=0, idx2=0, idx3=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    const char * var2 = "__gy_callback_var2";
    const char * var3 = "__gy_callback_var3";
    idx1 = yget_global(var1, 0);
    idx2 = yget_global(var2, 0);
    idx3 = yget_global(var3, 0);

    gy_Object * o1 = ypush_gy_Object();
    yput_global(idx1, 0);
    gy_Object * o2 = ypush_gy_Object();
    yput_global(idx2, 0);
    gy_Object * o3 = ypush_gy_Object();
    yput_global(idx3, 0);

    o1 -> object = arg1;
    o1 -> repo = repo;
    g_object_ref(o1 -> object);
    o1 -> info =
	  g_irepository_find_by_gtype(o1 -> repo,
				      G_OBJECT_TYPE(o1 -> object));

    o2 -> object = arg2;
    o2 -> repo = repo;
    o3 -> object = arg3;
    o3 -> repo = repo;


    const char * fmt = "noop, %s (%s, %s, %s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)
			 +strlen(var1)+strlen(var2)+strlen(var3)));
    sprintf(buf, fmt, cmd, var1, var2, var3);
    cmd=buf;
    ndrops+=3;
  }

  long dims[2]={1,1};
  *ypush_q(dims) = p_strcpy(cmd);
  ++ndrops;
  if (buf) p_free(buf);
  yexec_include(0,1);
  yarg_drop(ndrops);

}

gboolean gy_callback2_bool(void* arg1, void* arg2, void*arg3,
			   gy_signal_data* sd) {
  gy_callback_retval=0;
  gy_callback2(arg1, arg2, arg3, sd) ;
  return gy_callback_retval;
}

///// end callbacks

void
__gy_signal_connect(GObject * object,
		    GIBaseInfo * info,
		    GIRepository * repo,
		    const gchar* sig,
		    const gchar * cmd);

void
Y_gy_signal_connect(int argc) {
  gy_Object * o = yget_gy_Object(argc-1);
  if (!o->info || !GI_IS_OBJECT_INFO(o->info) || ! o -> object )
    y_error("First argument but hold GObject derivative instance");

  if (!strcmp(G_OBJECT_TYPE_NAME(o->object), "GtkBuilder")) {
    long idx1 = yget_global("__gy_gtk_builder", 0);
    void* usage=yget_use(argc-1);
    ypush_use(usage);
    yput_global(idx1, 0);
    long dims[Y_DIMSIZE]={1,1};
    *ypush_q(dims)=p_strcpy("noop, __gy_gtk_builder"
			    ".connect_signals_full("
			    "gy_gtk_builder_connector(),)");
    yexec_include(0, 1);
    ypush_nil();
    return;
  }

  ystring_t sig = ygets_q(argc-2);
  ystring_t cmd = NULL;

  if (yarg_string(argc-3)) cmd = p_strcpy(ygets_q(argc-3));
  else if (yarg_func(argc-3)) {
    cmd = p_strcpy(yfind_name(yget_ref(argc-3)));
  } else y_error("callback must be string or function");

  __gy_signal_connect(o->object, o->info, o->repo, sig, cmd);

  ypush_nil();
}

void
__gy_signal_connect(GObject * object, GIBaseInfo * info, GIRepository * repo,
		    const gchar * sig, const gchar * cmd)
{
  GIBaseInfo * cur, *next;
  GISignalInfo * cbinfo=NULL;
  gint i, n;

  cur = info;
  g_base_info_ref(cur);
  while (!cbinfo && cur) {
    GY_DEBUG("%s\n", g_base_info_get_name(cur) );
    n= g_object_info_get_n_signals(cur);
    for (i=0; i<n; ++i) {
      cbinfo = g_object_info_get_signal(cur, i);
      if (!strcmp(g_base_info_get_name(cbinfo), sig))
	break;
      g_base_info_unref(cbinfo);
      cbinfo=NULL;
    }
    next = g_object_info_get_parent(cur);
    g_base_info_unref(cur);
    cur = next;
  }
  if (!cbinfo) y_errorq ("Object does not support signal \"%s\"", sig);

  gy_signal_data * sd = g_new0(gy_signal_data, 1);

  GY_DEBUG("%p type: %s, name: %s, is signal info: %d, is callable: %d\n",
	   cbinfo,
	   g_info_type_to_string(g_base_info_get_type(cbinfo)),
	   g_base_info_get_name(cbinfo),
	   GI_IS_SIGNAL_INFO(cbinfo),
	   GI_IS_CALLABLE_INFO(cbinfo));

  sd -> info = cbinfo;
  sd -> cmd = cmd;
  sd -> repo = repo;

  GCallback * voidcallbacks[]={(GCallback*)(&gy_callback0),
			       (GCallback*)(&gy_callback1),
			       (GCallback*)(&gy_callback2)};
  GCallback * gbooleancallbacks[]={(GCallback*)(&gy_callback0_bool),
				   (GCallback*)(&gy_callback1_bool),
				   (GCallback*)(&gy_callback2_bool)};

  GCallback * * callbacks=NULL;

  gint nargs = g_callable_info_get_n_args (cbinfo);
  GY_DEBUG("Callback takes %d arguments\n", nargs);
  
  GITypeInfo * retinfo = g_callable_info_get_return_type (cbinfo);
  GITypeTag    rettag  = g_type_info_get_tag(retinfo); 
  g_base_info_unref(retinfo);
  switch(rettag) {
  case GI_TYPE_TAG_VOID:
    callbacks=voidcallbacks;
    break;
  case GI_TYPE_TAG_BOOLEAN:
    callbacks=gbooleancallbacks;
    break;
  default:
    y_errorq("unimplemented output type for callback: %",
	     g_type_tag_to_string (rettag));
  }

  if (nargs>2) y_errorn("unimplemented: callback with %ld arguments", nargs);

  GY_DEBUG("Callback address: %p\n", callbacks[nargs]);

  g_signal_connect (object,
		    sig,
		    G_CALLBACK(callbacks[nargs]),
		    sd);
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

void
gyGtkBuilderConnectFunc(void *builder,
			GObject *object,
			const gchar *signal_name,
			const gchar *handler_name,
			GObject *connect_object,
			GConnectFlags flags,
			gpointer user_data)

{
  // builder is a GtkBuilder but we don't want to use gtk headers
  GIObjectInfo * info = g_irepository_find_by_gtype(NULL, // should use repo
						    G_OBJECT_TYPE(object));
  GY_DEBUG("autoconnecting %s to %s\n", signal_name, handler_name);
  // ! we may leak memory
  __gy_signal_connect(object, info, NULL, signal_name, p_strcpy(handler_name));
  g_base_info_unref(info);
}

void
Y_gy_gtk_builder_connector(int argc)
{
  ypush_gy_Object()->object = (void*)&gyGtkBuilderConnectFunc;
}
