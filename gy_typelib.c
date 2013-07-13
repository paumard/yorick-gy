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

/// TYPELIB

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

int
yarg_gy_Typelib(int iarg)
{
  return yget_obj(iarg,0)==gy_Typelib_obj.type_name;
}

void
gy_Typelib_list(int argc) {
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
