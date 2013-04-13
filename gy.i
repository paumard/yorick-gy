plug_in, "gy";
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

local gy;
/* DOCUMENT gy

    gy is a Yorick plug-in around GObject-introspection. It can
    notably be used to create Gtk GUIs from within Yorick.

    Any library providing gobject-introspection can be accessed
    through this plug-in, as long as the gobject-introspection
    repository (GIR) files are installed on the system: under certain
    Linux systems, the corresponding package may be called
    "gir1.0-<libname>" or something similar. The notable example is
    Gtk.

    To load the GIR bindings for a given library, simply append the
    library's namespace to gy:
      Gtk = gy.Gtk;
    You can use gy_list_namespace to list the symbols inside this
    namespace:
      gy_list_namespace, Gtk;
    Objects are typically instanciated using a "new" method:
      button = gy.Gtk.Button.new();
    Object members can be listed with gy_list_object:
      gy_list_object, button;
    Callbacks can be connected to objects using gy_signal_connect.

    gy simply exposes conforming librariy content to Yorick. See the
    relevant library C API documentation for more details, for
    instance:
      https://developer.gnome.org/gtk3/stable/
    
   NOTE CONCERNING GTK:
    As of now, Gtk GUIs are always blocking, meaning you can't use the
    Yorick prompt whil a GUI is running. To accomodate for this
    limitation, see gyterm.

    Please use gy.Gtk.disable_setlocale() in any public code, else Gtk
    will install the user locale which will break Yorick in countries
    where the decimal separator is not the English dot.

   EXAMPLE:
    Gtk=gy.Gtk;
    Gtk.disable_setlocale();
    Gtk.init_check(0,);
    win = Gtk.Window.new(Gtk.WindowType.toplevel);
    button = Gtk.Button.new_with_label("Hello World!");
    win.add(button);
    gy_signal_connect, button, "clicked", "\"Hello World!\"";
    func winhide(void) {
    noop, win.hide();
    noop, Gtk.main_quit();
    }
    gy_signal_connect(win, "delete_event", "noop, winhide()");
    noop, win.show_all();
    noop, Gtk.main();
    
   SEE ALSO: gyterm
 */

extern gy_init;
/* DOCUMENT gy=gy_init()
    Initialize gy.
 */
gy=gy_init();

extern gy_require;
/* DOCUMENT var = gy_require("NAMESPACE")
         or var = gy.NAMESPACE

    Load typelib corresponding to NAMESPCE in the repository.

   EXAMPLE:
    Gtk=gy.Gtk

   SEE ALSO: gy
 */

extern gy_list_namespace;
/* DOCUMENT gy_list_namespace, NAMESPACE
   
    List symbols in NAMESPACE.  Beware: the list can contain thousands
    of entries.

   EXAMPLES:
    gy_list_namespace, "Gtk"
    gy_list_namespace, gy.Gtk
    
   SEE ALSO: gy
 */

extern gy_list_object;
/* DOCUMENT gy_list_object, OBJECT
    List symbols in OBJECT.
   EXAMPLE:
    gy_list_object, gy.Gtk.Window
   SEE ALSO: gy
 */

extern gy_signal_connect;
/* DOCUMENT gy_connect_signal, object, signal, handler
    Connect signal to signal handler

   ARGUMENTS:
    object:  a gy object supporting signals, e.g. an instance of
             gy.Gtk.Entry.
    signal:  the signal name, e.g. "activated".
    handler: the Yorick command to be executed when the object
             receives the signal.
             
   EXAMPLE:
    See gy.

   SEE ALSO: gy
*/



/// gyterm, a command line in a widget

func __gyterm_init(void) {
  require,  "string.i";
  extern __gyterm_initialized, __gyterm_win, __gyterm_entry;
  Gtk=gy.Gtk;
  Gtk.disable_setlocale();
  Gtk.init_check(0,);
  __gyterm_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  gy_signal_connect(__gyterm_win, "delete_event", "noop, __gyterm_suspend()");
  __gyterm_entry = Gtk.Entry.new();
  gy_signal_connect, __gyterm_entry, "activate", "__gyterm_entry_activated";
  noop, __gyterm_win.add(__gyterm_entry);
  __gyterm_initialized=1;
}  

func __gyterm_idler(void) {
  gy.Gtk.main();
}

func __gyterm_entry_activated(void) {
  noop, gy.Gtk.main_quit();
  set_idler, __gyterm_idler;
  cmd=__gyterm_entry.get_text();
  include, strchar("if (catch(-1)) {return;} "+cmd), 1;
}

func __gyterm_suspend(void) {
  noop, __gyterm_win.hide();
  noop, gy.Gtk.main_quit();
}

func gyterm(cmd)
/* DOCUMENT gyterm
   
     Open a window containing a single line in which arbitrary Yorick
     commands can be typed.

     If you want to keep a command line around while launching another
     gy-based, blocking GUI, simpy launch it from gyterm.

   SEE ALSO: gy
 */
{
  extern __gyterm_initialized;
  if (!__gyterm_initialized) __gyterm_init;
  noop, __gyterm_win.show_all();
  if (cmd) __gyterm_entry.set_text(cmd);
  noop, gy.Gtk.main();
}

/// hack: should be done from gi
extern gy_xid;
/* DOCUMENT id=gy_xid(wdg)
   
     Get X11 window ID associated with widget WDG.
     This allows displaying a Yorick window inside a Gtk widget.

   EXAMPLE:
     builder=gy.Gtk.Builder.new(glade_file);
     ywin = builder.get_object(yorick_widget_name);
     func on_ywin_event(void) {
       noop, Gtk.main_quit();
       set_idler, __gyterm_idler;
       window, parent=gy_xid(ywin);
     }
     gy_signal_connect, ywin, "event", "on_ywin_event";
     
   SEE ALSO: gy, gy_signal_connect, window
 */

extern gy_data;
/* DOCUMENT data = gy_data(object)
   
     Get data set by the C callback on object. Often the GdkEvent if
     object is a GtkWidget. This is a temporary hack.
     
 */
