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
  if (o->object) {
    // I don't know how reference counting works here...
    // if (GI_IS_STRUCT_INFO(o->info)) g_free(o->object);
    if (o->info && GI_IS_OBJECT_INFO(o->info)) {
      GY_DEBUG("Unref'ing GObject %p with refcount %d... ",
	       o->object, o->object->ref_count);
      g_object_unref(o->object);
      o->object=NULL;
      GY_DEBUG("done.\n");
    } else {
      if (gy_debug()) {
	fprintf(stderr,"Object %p not unref'ed\n", o->object);
	if (o->info && GI_IS_TYPE_INFO(o->info)) {
	  fprintf(stderr, "Object is ");
	  GITypeTag type = g_type_info_get_tag(o->info);
	  switch(type) {
	  case GI_TYPE_TAG_GLIST:
	    fprintf(stderr, "double linked list", 0);
	    break;
	  case GI_TYPE_TAG_GSLIST:
	    fprintf(stderr, "single linked list", 0);
	    break;
	  default:
	    fprintf(stderr, "unhandled TypeInfo");
	  }
	  fprintf(stderr,
		  "gy object name: %s, type: %s, namespace: %s\n",
		  g_base_info_get_name(o->info),
		  g_info_type_to_string(g_base_info_get_type (o->info)),
		  g_base_info_get_namespace (o->info));
	}
      }
    }
  }
  if (o->info) g_base_info_unref(o->info);
}

void gy_Object_print(void *obj) {
  gy_Object* o = (gy_Object*) obj;
  char spointer[40] = {0};
  if (o->object) {
    sprintf(spointer, "%p", o->object);
    y_print(spointer, 0);
    y_print(" is pointer to ", 0);
  }
  if (!o->info) {
    y_print("unknown type object", 0);
    return;
  }
  
  if (GI_IS_TYPE_INFO(o->info)) {
    GITypeTag type = g_type_info_get_tag(o->info);
    switch(type) {
    case GI_TYPE_TAG_GLIST:
      y_print("double linked list", 0);
      break;
    case GI_TYPE_TAG_GSLIST:
      y_print("single linked list", 0);
      break;
    default:
      y_error("unhandled TypeInfo");
    }
    return;
  }

  y_print("gy object name: ", 0);
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
  
  if (GI_IS_TYPE_INFO(o->info)) {
    GITypeTag type = g_type_info_get_tag(o->info);
    switch(type) {
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
      {
	int action = 0;
#define GYLIST_ACTION_DATA 1
#define GYLIST_ACTION_NEXT 2
#define GYLIST_ACTION_PREV 3
#define GYLIST_ACTION_SIZE 4
	if (!strcmp(name,"data")) action=GYLIST_ACTION_DATA;
	else if (!strcmp(name, "next")) action =GYLIST_ACTION_NEXT;
	else if (!strcmp(name, "prev")) action =GYLIST_ACTION_PREV;
	else if (!strcmp(name, "size")) action =GYLIST_ACTION_SIZE;
	else y_errorq("Unknown action for G(S)List: %s", name);
	
	if (!o->object && action !=GYLIST_ACTION_SIZE)
	  y_error("G(S)List is nil");
	GY_DEBUG("Extracting member %s from G(S)List\n", name);
	if (o->object) {
	  GY_DEBUG("   data=%p\n"
		   "   next=%p\n", o->object, ((GList*)o->object)->next);
	  if (type==GI_TYPE_TAG_GLIST)
	    GY_DEBUG("   prev=%p\n", ((GList*)o->object)->prev);
	}

	if (action == GYLIST_ACTION_SIZE) {
	  GList* lst = o->object;
	  if (!lst) {
	    ypush_long(0);
	    return;
	  }
	  long sz=1;
	  while (lst=lst->next) ++sz;
	  ypush_long(sz);
	  return;
	}

	if (action == GYLIST_ACTION_PREV && type == GI_TYPE_TAG_GSLIST)
	  y_error("Single-linked list: no prev");

	if ( (action == GYLIST_ACTION_NEXT && !((GList*) o->object) -> next) ||
	     (action == GYLIST_ACTION_PREV && !((GList*) o->object) -> prev) ) {
	  ypush_nil();
	  return;
	}

	GITypeInfo * itrf = g_type_info_get_interface(g_type_info_get_param_type(o->info, 0));

	gy_Object * out = ypush_gy_Object();
	out -> repo = o -> repo;
	if (action == GYLIST_ACTION_DATA) {
	  out -> info = itrf;
	  out -> object = ((GList*) o->object) -> data;
	  if (g_base_info_get_type (itrf) == GI_INFO_TYPE_OBJECT) {
	    g_object_ref(out -> object);
	  }
	} else {
	  out -> info = o -> info;
	  if (action==GYLIST_ACTION_NEXT)
	    out -> object = ((GList*) o->object) -> next;
	  else if (action==GYLIST_ACTION_PREV)
	    out -> object = ((GList*) o->object) -> prev;
	}
	g_base_info_ref(out -> info);
	return;
      }
      break;
    default:
      y_error("Don't know how to extract members from that kind of objects");
    } 
  }

  if (GI_IS_ENUM_INFO(o->info)) {
    
    gint64 wtype=-1;
    gint nc = g_enum_info_get_n_values (o->info);
    GIValueInfo * ci ;
    gint i;
    gboolean tfound=0;
    char * name_dn = p_strtolower(name);
    if (!strcmp(name, name_dn)) {
      p_free(name_dn);
      name_dn=NULL;
    }
    for (i=0; i<nc; ++i) {
      ci = g_enum_info_get_value(o->info, i);
      if (!strcmp(g_base_info_get_name (ci), name) ||
	  (name_dn && !strcmp(g_base_info_get_name (ci), name_dn)) ) {
	wtype=g_value_info_get_value (ci);
	tfound=1;
	g_base_info_unref(ci);
	break;
      }
      g_base_info_unref(ci);
    }
    p_free(name_dn);
    if (tfound) ypush_long(wtype);
    else y_errorq("No such enum value: %s", name);
    return;
  }

  gboolean isstruct = GI_IS_STRUCT_INFO(o->info),
    isobject = GI_IS_OBJECT_INFO(o->info),
    isitrf = GI_IS_INTERFACE_INFO(o->info);


  /// Look for method
  GIBaseInfo * info =NULL;

  if (isstruct || isobject || isitrf) {

    GY_DEBUG("Looking for method %s in %s\n",
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
      GY_DEBUG("Looking for method %s in parent %s\n",
	     name,
	     g_base_info_get_name(cur));
      info = g_object_info_find_method (cur, name);
    }
    if (info) {
      GY_DEBUG("Method %s found in %s\n",
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
  }

  /// Look for property
  if (isobject || isitrf) {
    GY_DEBUG("Looking for property %s in %s\n",
	     name,
	     g_base_info_get_name(o->info));
    ystring_t name2 = p_strcpy(name);
    GIPropertyInfo * cur = gy_base_info_find_property_info(o->info, name2);
    if (cur) {
      GValue val=G_VALUE_INIT;
      GITypeInfo * ti = g_property_info_get_type(cur);
      g_value_init(&val,
		   g_object_class_find_property(G_OBJECT_GET_CLASS(o->object),
						name2)->value_type);
      g_object_get_property(o->object, name2, &val);
      gy_value_push(&val, ti, o);
      g_base_info_unref(ti);

      p_free(name2);
      return;
    }
    p_free(name2);
  }

  /// Look for field
  if (isobject || isstruct) {
    ystring_t name2 = p_strcpy(name);
    GIBaseInfo * cur = gy_base_info_find_field_info(o->info, name2);
    if (cur) {
      GITypeInfo * ti = g_field_info_get_type(cur);
      GIArgument rarg;
      gboolean success=g_field_info_get_field(cur, o->object, &rarg);
      if (!success)
	y_error("get field failed");

      gy_Argument_pushany(&rarg, ti, o);

      g_base_info_unref(ti);
      p_free(name2);
      return;
    }
    p_free(name2);
  }

  /// Look for signal
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

    if (!info) {
      y_error("No such member");
    }
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

  gboolean
    isobject = GI_IS_OBJECT_INFO(o->info),
    isstruct = GI_IS_STRUCT_INFO(o->info),
    isitrf   = GI_IS_INTERFACE_INFO(o->info);

  if (GI_IS_TYPE_INFO(o->info)) {
    GITypeTag type = g_type_info_get_tag(o->info);
    if (type !=GI_TYPE_TAG_GLIST && type !=GI_TYPE_TAG_GSLIST)
      y_error("Unimplemented");
    if (!o->object) y_error("G(S)List is nil");
    GList* lst = o->object;
    long idx = ygets_l(argc-1)-1;
    gy_Object * out = ypush_gy_Object();
    if (type !=GI_TYPE_TAG_GLIST)
      out->object=g_list_nth_data (lst, idx);
    else if (type !=GI_TYPE_TAG_GSLIST)
      out->object=g_slist_nth_data ((GSList*)lst, idx);
    if (!out->object) y_error("index out of range");
    
    GITypeInfo * itrf =
      g_type_info_get_interface(g_type_info_get_param_type(o->info, 0));
    out -> repo = o -> repo;
    out -> info = itrf;
    if (g_base_info_get_type (itrf) == GI_INFO_TYPE_OBJECT) {
      g_object_ref(out -> object);
    }
    g_base_info_ref(out -> info);
    return;
  }

  if (isobject || isitrf || isstruct) {
    GY_DEBUG("Pushing gy_Object return value\n");
    gy_Object* out = ypush_gy_Object(0);

    out->info=o->info;
    g_base_info_ref(o->info);
    out->repo=o->repo;

    if(!o->object) {
      if (yarg_gy_Object(argc)) {
	GY_DEBUG("This is a cast operation\n");
	gy_Object * ino = yget_gy_Object(argc--);
	GY_DEBUG("here\n");
	out -> object = ino -> object;
	GY_DEBUG("here\n");
	if ((ino->info && GI_IS_OBJECT_INFO(ino->info)) ||
	    GI_IS_OBJECT_INFO(out->info)) {
	  GY_DEBUG("This is an object, referencing\n");
	  g_object_ref(out->object);
	}
	GY_DEBUG("Cast done\n");
      } else if (isobject) {
	GY_DEBUG("Instanciating GObject\n");
	/* instanciate GObject, interpret arguments as properties */
	guint n_parameters = argc/2;
	GParameter *parameters=NULL;
	if (n_parameters) {
	  parameters=g_new0(GParameter, n_parameters);
	  guint p;
	  int iarg=argc;
	  long index;
	  GIPropertyInfo * cur;
	  GITypeInfo * ti;

	  for (p=0; p<n_parameters; ++p) {
	    index=yarg_key(iarg);
	    GY_DEBUG("index=%ld\n", index);
	    if (index<0) parameters[p].name = ygets_q(iarg);
	    else parameters[p].name=yfind_name(index);

	    cur = gy_base_info_find_property_info(o->info, parameters[p].name);
	    GY_DEBUG("Property info:%p\n", cur);
	    if (!cur) y_errorq("No such porperty in object: \"%s\"", parameters[p].name);
	    --iarg;
	    GY_DEBUG("property name=\"%s\"\n", parameters[p].name);
	    ti  = g_property_info_get_type(cur);
	    g_base_info_unref(cur);
	    gy_value_init(&(parameters[p].value), ti);
	    gy_value_set_iarg(&(parameters[p].value), ti, iarg);
	    --iarg;
	    g_base_info_unref(ti);
	  }
	}
	out -> object =
	  g_object_newv(g_registered_type_info_get_g_type(o->info),
			n_parameters,
			parameters);
	g_free(parameters);
	GY_DEBUG("Newly created object has refcount=%d and %d floating ref\n",
		 out->object->ref_count, g_object_is_floating(out->object));
	if (G_IS_INITIALLY_UNOWNED(out->object)) {
	  g_object_ref_sink(out->object);
	}
	if (!out->object->ref_count) g_object_ref(out->object);
	GY_DEBUG("Newly created object has refcount=%d\n",
		 out->object->ref_count);
	return;
      } else if (isstruct) {
	GY_DEBUG("Instanciating C struct\n");
	/* instanciate struct, arguments will be parsed later */
	out -> object = g_malloc0(g_struct_info_get_size (o->info));
      }
      else y_error("Object is not callable");
    } else {
      out -> object = o->object;
      if (GI_IS_OBJECT_INFO(o->info) || GI_IS_OBJECT_INFO(out->info))
	g_object_ref(out->object);
    }

    /* try setting / getting properties */
    int iarg = argc; // last is newly pushed reference
    long index;
    char * name;
    GIBaseInfo * cur=NULL;
    GITypeInfo * ti;
    gboolean getting=0;
    GParamFlags pf;
    GIArgument rarg;

    if (argc==1 && yarg_nil(iarg)) return;

    int lim=0;
    while (iarg > lim) { // 0 is output
      index=yarg_key(iarg);
      GY_DEBUG("Key iarg: %d, index: %ld\n", iarg, index);
      if (index<0) {
	getting=1;
	index=yget_ref(iarg);
	if (index<0) name = ygets_q(iarg);
	else name=yfind_name(index);
	GY_DEBUG("Getting member %s\n", name);
      } else {
	getting=0;
	name=yfind_name(index);
	GY_DEBUG("Setting member %s\n", name);
      }

      if ( (cur = gy_base_info_find_property_info(o->info, name)) ) {
	/* NAME is property */ 
	GY_DEBUG("Canonical property name: %s\n",name);
	ti = g_property_info_get_type(cur);
	pf = g_property_info_get_flags (cur);

	GValue val=G_VALUE_INIT;
	g_value_init(&val,
		     g_object_class_find_property(G_OBJECT_GET_CLASS(o->object),
						  name)->value_type);

	iarg--;
	if (getting) {
	  if (!(pf & G_PARAM_READABLE)) y_error("property is not readable");
	  GY_DEBUG("Getting property %s", name);
	  long idx=yget_ref(iarg);
	  GY_DEBUG("Output variable iarg: %d, index: %ld\n", iarg, idx);
	  g_object_get_property(o->object, name, &val);
	  gy_value_push(&val, ti, o);
	  yput_global(idx, 0);
	  yarg_drop(1);
	  GY_DEBUG("done.\n");
	} else {
	  if (!(pf & G_PARAM_WRITABLE)) y_error("property is not writable");
	  GY_DEBUG("Setting property %s\n", name);
	  gy_value_set_iarg(&val, ti, iarg);
	  g_object_set_property(o->object, name, &val);
	}
	g_base_info_unref(ti);
      } else if ( (cur = gy_base_info_find_field_info(o->info, name)) ) {
	/* NAME is field */
	GY_DEBUG("Member %s is a Field.\n", name);
	ti = g_field_info_get_type(cur);
	if (getting) { //y_error("Getting");
	  GY_DEBUG("getting\n");
	  long idx = yget_ref(iarg-1);
	  gboolean success=g_field_info_get_field(cur, o->object, &rarg);
	  if (!success)
	    y_error("get field failed");
	  gy_Argument_pushany(&rarg, ti, o);
	  yput_global(idx, 0);
	  yarg_drop(1);
	} else {
	  gy_Argument_getany(&rarg, ti, --iarg);
	  if (!g_field_info_set_field(cur, out->object, &rarg))
	    y_error("set field failed");
	}
	g_base_info_unref(ti);
      } else y_errorq("%s is neither property not field", name);
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
    GY_DEBUG("Getting argument %d\n", i);
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

  GY_DEBUG("Calling function %s... ", g_base_info_get_name(o->info));

  gboolean success = g_function_info_invoke (o->info,
					     in_args,
					     n_in,
					     out_args,
					     n_out,
					     &retval,
					     &err);
  GY_DEBUG("done.\n");

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

void
gy_Object_list(int argc) {
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

void
Y_gy_object_free(int argc)
{
  g_free(yget_gy_Object(0)->object);
}
