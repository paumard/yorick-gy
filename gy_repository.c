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

static y_userobj_t gy_Repository_obj =
  {"gy_Repository",
   NULL, //&gy_Repository_free,
   &gy_Repository_print,
   &gy_Repository_eval,
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
  y_print("gy_Repository with loaded namespaces: ", 1);
  for (;*nspcs;++nspcs) {
    y_print(*nspcs, 1);
    y_print(" ", 1);
  }
}

void
gy_Repository_extract(void *obj, char * name)
{
  gy_Repository * r = (gy_Repository *) obj;
  GError * err;

  if (!strcmp(name, "require") ||
      !strcmp(name, "require_private") ||
      !strcmp(name, "get_search_path") ||
      !strcmp(name, "prepend_search_path")||
      !strcmp(name, "is_registered")||
      !strcmp(name, "get_version")||
      !strcmp(name, "enumerate_versions")
      ) {
    gy_Repository* out = ypush_gy_Repository();
    out->repo = r->repo;
    out->method=name;
    return;
  }

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

void
gy_Repository_eval(void *obj, int argc)
{
  gy_Repository* r = (gy_Repository*) obj;
  if (!r->method) y_error("Object is not callable");

  if (!strcmp(r->method, "require")) {
    GError * err=NULL;

    /// process input
    ystring_t namespace = ygets_q(argc-1);
    ystring_t version = NULL;
    if (argc>=2) version=ygets_q(argc-2);
    GIRepositoryLoadFlags flags=0;
    if (argc>=3) flags=ygets_l(argc-3);

    /// push output
    gy_Typelib * tl = ypush_gy_Typelib();

    tl -> namespace = p_strcpy(namespace);
    tl -> repo      = r->repo;
    tl -> typelib   = g_irepository_require
      (r->repo, namespace, version, flags, &err);
    if (!tl->typelib) y_error(err->message);
   
    return;
  }

  if (!strcmp(r->method, "require_private")) {
    GError * err=NULL;

    /// process input
    ystring_t dir = ygets_q(argc-1);
    ystring_t namespace = ygets_q(argc-2);
    ystring_t version = NULL;
    if (argc>=2) version=ygets_q(argc-3);
    GIRepositoryLoadFlags flags=0;
    if (argc>=3) flags=ygets_l(argc-4);

    /// push output
    gy_Typelib * tl = ypush_gy_Typelib();

    tl -> namespace = p_strcpy(namespace);
    tl -> repo      = r->repo;
    tl -> typelib   = g_irepository_require_private
      (r->repo, dir, namespace, version, flags, &err);
    if (!tl->typelib) y_error(err->message);
   
    return;
  }

  if (!strcmp(r->method, "get_search_path")) {
    GSList * lst = g_irepository_get_search_path();
    GSList * cur = 0;
    long count =0;
    
    for (cur=lst; cur; cur=cur->next) ++count;

    GY_DEBUG("%ld elements in path\n", count);
    long dims[Y_DIMSIZE]={1, count};
    ystring_t * pth = ypush_q(dims);

    for (cur=lst, count=0; cur; cur=cur->next, ++count) {
      pth[count]=p_strcpy(cur->data);
    }

    return;
  }

  if (!strcmp(r->method, "prepend_search_path")) {
    g_irepository_prepend_search_path(ygets_q(argc-1));
    ypush_gy_Repository() -> repo = r -> repo;
    return;
  }

  if (!strcmp(r->method, "is_registered")) {
    ystring_t namespace = ygets_q(argc-1);
    ystring_t version = NULL;
    if (argc>=2) version=ygets_q(argc-2);
    ypush_long(g_irepository_is_registered (r->repo,
					    namespace,
					    version));
    return;
  }

  if (!strcmp(r->method, "get_version")) {
    ystring_t namespace ;
    if (yarg_string(argc-1)) namespace = ygets_q(argc-1);
    else namespace = yget_gy_Typelib(argc-1)->namespace;

    *ypush_q(0) = 
      p_strcpy(g_irepository_get_version (r->repo, namespace));
    return;
  }

  if (!strcmp(r->method, "enumerate_versions")) {
    ystring_t namespace ;
    if (yarg_string(argc-1)) namespace = ygets_q(argc-1);
    else namespace = yget_gy_Typelib(argc-1)->namespace;

    GSList * lst = g_irepository_enumerate_versions(r->repo, namespace);
    GSList * cur = 0;
    long count =0;
    
    for (cur=lst; cur; cur=cur->next) ++count;

    GY_DEBUG("%ld versions of %s\n", count, namespace);
    long dims[Y_DIMSIZE]={1, count};
    ystring_t * pth = ypush_q(dims);

    for (cur=lst, count=0; cur; cur=cur->next, ++count) {
      pth[count]=p_strcpy(cur->data);
    }

    return;
  }

  y_error("Unknown repository method");
}


gy_Repository* yget_gy_Repository(int iarg) {
  return (gy_Repository*) yget_obj(iarg, &gy_Repository_obj);
}

gy_Repository* ypush_gy_Repository() {
  gy_Repository* out = ypush_obj(&gy_Repository_obj, sizeof(gy_Repository));
  out->method=0;
  return out;
}

int
yarg_gy_Repository(int iarg)
{
  return yget_obj(iarg,0)==gy_Repository_obj.type_name;
}
