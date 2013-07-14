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

void gy_callback0(void* arg1, gy_signal_data* sd) {
  GY_DEBUG("in gy_callback0()\n");
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  void * data = sd -> data;
  //  void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(4);

  long idx1=0, idxud=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    const char * varud = "__gy_callback_userdata";
    idx1 = yget_global(var1, 0);
    idxud = yget_global(varud, 0);

    gy_Object * o1 = ypush_gy_Object();
    yput_global(idx1, 0);

    o1 -> object = arg1;
    o1 -> repo = repo;
    g_object_ref(o1 -> object);
    o1 -> info =
	  g_irepository_find_by_gtype(o1 -> repo,
				      G_OBJECT_TYPE(o1 -> object));

    gy_Object * oud = ypush_gy_Object();
    yput_global(idxud, 0);
    oud -> object = data;
    oud -> repo = repo;

    const char * fmt = "__gy_callback_retval = %s (%s, %s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)+strlen(var1)+strlen(varud)));
    sprintf(buf, fmt, cmd, var1, varud);
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

inline gboolean gy_callback_retbool() {
  long idx=yget_global("__gy_callback_retval", 0);
  ypush_check(1);
  ypush_global(idx);
  long retval=0;
  if (yarg_number(0)) retval=ygets_l(0);
  yarg_drop(1);
  return retval;
}

gboolean gy_callback0_bool(void* arg1, gy_signal_data* sd) {
  gy_callback0(arg1, sd) ;
  return gy_callback_retbool();
}

void gy_callback1(void* arg1, void* arg2, gy_signal_data* sd) {
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  void * data = sd -> data;
  //void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(4);

  long idx1=0, idx2=0, idxud=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    const char * var2 = "__gy_callback_var2";
    const char * varud = "__gy_callback_userdata";
    idx1 = yget_global(var1, 0);
    idx2 = yget_global(var2, 0);
    idxud = yget_global(varud, 0);

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

    gy_Object * oud = ypush_gy_Object();
    yput_global(idxud, 0);
    oud -> object = data;
    oud -> repo = repo;

    const char * fmt = "__gy_callback_retval = %s (%s, %s, %s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)+strlen(var1)+strlen(var2)
			 +strlen(varud)));
    sprintf(buf, fmt, cmd, var1, var2, varud);
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
  gy_callback1(arg1, arg2, sd) ;
  return gy_callback_retbool();
}

void gy_callback2(void* arg1, void* arg2, void* arg3, gy_signal_data* sd) {
  const char * cmd = sd -> cmd;
  GISignalInfo * cbinfo = sd -> info;
  GIRepository * repo = sd -> repo;
  void * data = sd -> data;
  // void * udata = sd -> data;
  GY_DEBUG("Callback called with pointer %p: \"%s\"\n", cmd, (char*)cmd);
  char*buf=NULL;
  int ndrops=0;

  ypush_check(5);

  long idx1=0, idx2=0, idx3=0, idxud=0;

  if (cbinfo) {
    const char * var1 = "__gy_callback_var1";
    const char * var2 = "__gy_callback_var2";
    const char * var3 = "__gy_callback_var3";
    const char * varud = "__gy_callback_userdata";
    idx1 = yget_global(var1, 0);
    idx2 = yget_global(var2, 0);
    idx3 = yget_global(var3, 0);
    idxud = yget_global(varud, 0);

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


    gy_Object * oud = ypush_gy_Object();
    yput_global(idxud, 0);
    oud -> object = data;
    oud -> repo = repo;

    const char * fmt = "__gy_callback_retval = %s (%s, %s, %s, %s)";
    char * buf=p_malloc(sizeof(char)*
			(strlen(fmt)+strlen(cmd)
			 +strlen(var1)+strlen(var2)+strlen(var3)
			 +strlen(varud)));
    sprintf(buf, fmt, cmd, var1, var2, var3, varud);
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
  gy_callback2(arg1, arg2, arg3, sd) ;
  return gy_callback_retbool();
}

///// end callbacks

void
__gy_signal_connect(GObject * object,
		    GIBaseInfo * info,
		    GIRepository * repo,
		    const gchar* sig,
		    const gchar * cmd,
		    void * data);

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
			    "__gy_gtk_builder_connector(),)");
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

  void* data;
  if (argc>=4) data = yget_gy_Object(argc-4)->object; 

  __gy_signal_connect(o->object, o->info, o->repo, sig, cmd, data);

  ypush_nil();
}

void
__gy_signal_connect(GObject * object, GIBaseInfo * info, GIRepository * repo,
		    const gchar * sig, const gchar * cmd, void * data)
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
  sd -> data = data;

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
  __gy_signal_connect(object, info, NULL, signal_name, p_strcpy(handler_name),
		      user_data);
  g_base_info_unref(info);
}

void
Y___gy_gtk_builder_connector(int argc)
{
  ypush_gy_Object()->object = (void*)&gyGtkBuilderConnectFunc;
}
