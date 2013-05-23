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
    
  case GI_TYPE_TAG_GLIST:
  case GI_TYPE_TAG_GSLIST:
    if (yarg_nil(iarg)) arg->v_pointer=NULL;
    else arg->v_pointer=yget_gy_Object(iarg)->object;
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
    case GI_INFO_TYPE_STRUCT:
    case GI_INFO_TYPE_OBJECT:
      if (!arg -> v_pointer) ypush_nil();
      outObject = ypush_gy_Object();
      outObject -> repo= o -> repo;
      outObject -> object = arg -> v_pointer;
      if (!outObject->object)
	y_warn("object is NULL!");
      if (g_base_info_get_type (itrf) == GI_INFO_TYPE_OBJECT) {
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
      }
      outObject -> info =
	g_irepository_find_by_gtype(o -> repo,
				    g_registered_type_info_get_g_type(itrf));
      g_base_info_ref(outObject->info);
      break;
    default:
      y_errorn("Unimplemented output GIArgument interface type %ld",
	       g_base_info_get_type (itrf));
    }
    break;
  case GI_TYPE_TAG_GLIST:
  case GI_TYPE_TAG_GSLIST:
    outObject = ypush_gy_Object();
    outObject -> repo= o -> repo;
    outObject -> object = arg -> v_pointer;
    outObject -> info = info;
    g_base_info_ref(info);
    //printf("%s\n", g_base_info_get_name(info));
    break;
  default:
    y_errorq("Unimplemented output GIArgument type: %s",
	     g_type_tag_to_string(type));
  }
}
