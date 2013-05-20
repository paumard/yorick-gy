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

void
gy_value_init(GValue * val, GITypeInfo * info)
{
  GY_DEBUG("in gy_value_init\n");
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  GY_DEBUG("Initializing GValue to %s\n", g_type_tag_to_string(type));
  switch (type) {
  case GI_TYPE_TAG_INTERFACE:
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_OBJECT:
      g_value_init(val, G_TYPE_OBJECT);
      break;
    case GI_INFO_TYPE_ENUM:
      g_value_init(val,  g_registered_type_info_get_g_type(itrf));
      GY_DEBUG("GValue is enum\n");
      break;
    default:
      y_errorn("Unimplemented GValue interface type %ld",
	       g_base_info_get_type (itrf));
    }
    g_base_info_unref(itrf);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    g_value_init(val, G_TYPE_STRING);
    GY_DEBUG("GValue is string\n");
    break;
  default:
    y_error("Unimplement property GValue type");
  }
  GY_DEBUG("out gy_value_init\n");
}

void
gy_value_set_iarg(GValue* val, GITypeInfo * info, int iarg)
{
  GY_DEBUG("in gy_value_set_iarg\n");
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  switch (type) {
  case GI_TYPE_TAG_INTERFACE:
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_ENUM:
      {
	g_value_set_enum (val, ygets_l(iarg));
	break;
      }
      break;
    default:
      y_errorn("Unimplemented GValue interface type %ld",
	       g_base_info_get_type (itrf));
    }
    g_base_info_unref(itrf);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    g_value_set_static_string (val, ygets_q(iarg));
    GY_DEBUG("GValue is string: \"%s\"\n", ygets_q(iarg));
    break;
  default:
    y_error("Unimplement property GValue type");
  }
  GY_DEBUG("out gy_iarg2gvalue\n");
}

void
gy_value_push(GValue * pval, GITypeInfo * info, gy_Object* o)
{
  GITypeTag tag = g_type_info_get_tag(info);
  switch (tag) {
  case GI_TYPE_TAG_INTERFACE:
    {
      GIBaseInfo * itrf = g_type_info_get_interface (info);
      switch(g_base_info_get_type (itrf)) {
      case GI_INFO_TYPE_ENUM:
	ypush_long(g_value_get_enum(pval));
	g_base_info_unref(itrf);
	break;
      case GI_INFO_TYPE_OBJECT:
	{
	  GObject * prop=g_value_get_object(pval);
	  g_object_ref_sink(prop);
	  if (!prop) {
	    g_base_info_unref(itrf);
	    y_error("get property failed");
	  }
	  GY_DEBUG("pushing result... ");
	  ypush_check(1);
	  gy_Object * out = ypush_gy_Object();

	  out->info=itrf;
	  out->object=prop;
	  out->repo=o->repo;
	}
	break;
      default:
	g_base_info_unref(itrf);
	y_error ("fix me: only properties of type object supported yet");
      }
      break;
    }
  }


}
