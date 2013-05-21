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

GIPropertyInfo *
gy_base_info_find_property_info(GIBaseInfo * objectinfo, char * name)
{

  if (GI_IS_STRUCT_INFO(objectinfo)) return NULL;

  gboolean isobject = GI_IS_OBJECT_INFO(objectinfo);
  gint iprop, nprop = isobject?
    g_object_info_get_n_properties(objectinfo):
    g_interface_info_get_n_properties(objectinfo);

  GIPropertyInfo * cur=NULL;
  int hyphenize;
  char * oname = NULL;
  for (hyphenize=0; hyphenize<=1; ++hyphenize) {
    if (hyphenize) {
      GY_DEBUG("Property %s not found, trying to replace underscores with hyphens\n", name);
      oname = p_strcpy(name);
      g_strdelimit(name, "_", '-');
      if (!strcmp(name, oname)) {
	p_free(oname);
	return NULL;
      }
    }
	      
    for (iprop=0; iprop<nprop; ++iprop) {
      GY_DEBUG("i=%d/%d\n", iprop, nprop);
      cur = isobject?
	g_object_info_get_property (objectinfo, iprop):
	g_interface_info_get_property (objectinfo, iprop);
      GY_DEBUG("comparing %s with %s\n", name, g_base_info_get_name(cur));
      if (!strcmp(name, g_base_info_get_name(cur))) {
	GY_DEBUG("found it\n");
	p_free(oname);
	return cur;
      }
      g_base_info_unref(cur);
    }
  }
  strcpy(name, oname);
  p_free(oname);
  return NULL;
}

GIPropertyInfo *
gy_base_info_find_field_info(GIBaseInfo * objectinfo, char * name)
{

  if (GI_IS_INTERFACE_INFO(objectinfo)) return NULL;

  gboolean isobject = GI_IS_OBJECT_INFO(objectinfo);
  gint iprop, nprop = isobject?
    g_object_info_get_n_fields(objectinfo):
    g_struct_info_get_n_fields(objectinfo);

  GIFieldInfo * cur=NULL;
  int hyphenize;
  char * oname = NULL;
  for (hyphenize=0; hyphenize<=1; ++hyphenize) {
    if (hyphenize) {
      GY_DEBUG("Field %s not found, trying to replace underscores with hyphens\n", name);
      oname = p_strcpy(name);
      g_strdelimit(name, "_", '-');
      if (!strcmp(name, oname)) {
	p_free(oname);
	return NULL;
      }
    }
	      
    for (iprop=0; iprop<nprop; ++iprop) {
      GY_DEBUG("i=%d/%d\n", iprop, nprop);
      cur = isobject?
	g_object_info_get_field (objectinfo, iprop):
	g_struct_info_get_field (objectinfo, iprop);
      GY_DEBUG("comparing %s with %s\n", name, g_base_info_get_name(cur));
      if (!strcmp(name, g_base_info_get_name(cur))) {
	GY_DEBUG("found it\n");
	p_free(oname);
	return cur;
      }
      g_base_info_unref(cur);
    }
  }
  strcpy(name, oname);
  p_free(oname);
  return NULL;
}
