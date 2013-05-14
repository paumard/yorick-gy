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
gy_iarg2gvalue(GITypeInfo * info, int iarg, GValue* val)
{
  GITypeTag type = g_type_info_get_tag(info);
  GIBaseInfo * itrf;
  switch (type) {
  case GI_TYPE_TAG_INTERFACE:
    itrf = g_type_info_get_interface(info);
    switch(g_base_info_get_type (itrf)) {
    case GI_INFO_TYPE_ENUM:
      {
	const GEnumValue * ev=NULL;
	const gchar * name = g_base_info_get_name(info);
	GY_DEBUG("info name: %s\n", name);
	GType gt = g_type_module_register_enum ( "toto", name, ev);
	g_value_init(val, gt);
	g_value_set_enum (val, ygets_l(iarg));
      break;
      }
      break;
    default:
      y_errorn("Unimplemented GIArgument interface type %ld",
	       g_base_info_get_type (itrf));
    }
    g_base_info_unref(itrf);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    g_value_init(val, G_TYPE_STRING);
    g_value_set_static_string (val, ygets_q(iarg));
    GY_DEBUG("GValue is string: \"%s\"\n", ygets_q(iarg));
    break;
  default:
    y_error("Unimplement property GValue type");
  }
}
