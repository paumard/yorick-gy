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


   PROVIDED APPLICATIONS
    gy comes with a few sample GUI utilities:
    
     - gyterm is a command line in a Gtk window. It is useful for
       keeping a Yorick command line while another GUI is running.

     - gycmap is a wrapper around cmap.

     - gywindow is a wrapper for Yorick windows (work in progress).

   DETAILS
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

    Please use gy_setlocale() in any public code, else Gtk will set
    LC_NUMERIC the user locale which will break Yorick in countries
    where the decimal separator is not the English dot.

   EXAMPLE:
    Gtk=gy.Gtk;
    Gtk.init_check(0,);
    gy_setlocale("C");
    win = Gtk.Window.new(Gtk.WindowType.toplevel);
    button = Gtk.Button.new_with_label("Hello World!");
    win.add(button);
    func hello(widget, event, data) {
      "\"Hello World!\"";
    }
    gy_signal_connect, button, "clicked", hello;
    func winhide(widget, event, data) {
      noop, win.hide();
      noop, Gtk.main_quit();
    }
    gy_signal_connect(win, "delete-event", winhide);
    noop, win.show_all();
    noop, Gtk.main();
    
   SEE ALSO: gyterm, gycmap, gywindow
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
             receives the signal. HANDLER will be called like:
             include, ["noop, " + handler + "(par1, ..., parn)"], 1
             where par1 to parn are the parameters described in the C
             documentation for SIGNAL. HANDLER may be a string or a
             Yorick function.
             
   EXAMPLE:
    See gy.

   SEE ALSO: gy
*/



/// gyterm, a command line in a widget

func __gyterm_init(void) {
  require,  "string.i";
  extern __gyterm_initialized, __gyterm_win;
  Gtk=gy.Gtk;
  noop, Gtk.init_check(0,);
  gy_setlocale;
  __gyterm_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, __gyterm_win.set_title("Yorick command line");
  gy_gtk_window_suspend, __gyterm_win;
  __gyterm_entry = Gtk.Entry.new();
  gy_gtk_entry_include, __gyterm_entry;
  noop, __gyterm_win.add(__gyterm_entry);
  noop, __gyterm_entry.set_width_chars(80);
  __gyterm_initialized=1;
}  

func __gyterm_idler(void) {
  noop, gy.Gtk.main();
}

func __gyterm_entry_activated(widget, user_data) {
  gy_gtk_idleonce;
  cmd=widget.get_text();
  noop, widget.set_text("");
  if (cmd != "") include, strchar("if (catch(-1)) {return;} "+cmd), 1;
}

func __gyterm_key_pressed(widget, event) {
  ev = gy.Gdk.EventKey(event);
  ev, keyval, keyval;
  if (keyval!=gy.Gdk.KEY_Return) return;
  gy_gtk_idleonce;
  cmd=widget.get_text();
  include, strchar("if (catch(-1)) {return;} "+cmd), 1;
}

func gy_gtk_entry_include(widget) {
/* DOCUMENT gy_gtk_entry_include, entry_widget
   
    Makes entry_widget mimick gyterm behavior.

   EXAMPLE
    entry=gy.Gtk.Entry.new();
    gy_gtk_entry_include, entry;

   SEE ALSO: gy, gyterm, gy_gtk_window_suspend
 */
  gy_signal_connect, widget, "activate", __gyterm_entry_activated;
  noop, widget.set_placeholder_text("Yorick command");
  noop, widget.set_tooltip_text("Yorick command");
}

func gy_gtk_window_suspend(window)
/* DOCUMENT gy_gtk_window_suspend, window
   
    Connect a standard handler to the delete event of WINDOW.
    
   EXAMPLE
    win=gy.Gtk.Window.new();
    gy_gtk_window_suspend, win;

   SEE ALSO: gy, gyterm, gy_gtk_entry_include
 */
{
  gy_signal_connect, window, "delete-event", __gyterm_suspend;
}

func __gyterm_suspend(widget, event) {
  noop, widget.hide();
  noop, gy.Gtk.main_quit();
}

func gyterm(cmd)
/* DOCUMENT gyterm
   
     Open a window containing a single line in which arbitrary Yorick
     commands can be typed.

     If you want to keep a command line around while launching another
     gy-based, blocking GUI, simpy launch it from gyterm.

     If you want to embed gyterm in another GUI, see
     gy_gtk_entry_include.

   SEE ALSO: gy, gy_gtk_entry_include, gycmap, gywindow
 */
{
  extern __gyterm_initialized, __gyterm_win;
  if (!__gyterm_initialized) __gyterm_init;
  noop, __gyterm_win.show_all();
  if (cmd) include, [cmd], 1; 
  else noop, gy.Gtk.main();
}

/// gycmap: a GUI for cmap
func __gycmap_init(void) {
  require, "pathfun.i";
  extern __gycmap_initialized, __gycmap_builder, __gycmap_win, __gycmap_ebox,
    __gycmap_gist_img, __gycmap_msh_img, __gycmap_mpl_img, __gycmap_cmd,
    __gycmap_gpl_img, __gycmap_gmt_img, __gycmap_cur_img,
    __gycmap_div_img, __gycmap_seq_img, __gycmap_qual_img,
    __gycmap_cur_names, __gycmap_gist_names;

  gist_png = find_in_path("gist-cmap.png", takefirst=1,
                          path=pathform(_(get_cwd(),
                                          _(Y_SITES,
                                            Y_SITE)+"data/")));
  png_dir=dirname(gist_png);
  
  glade = find_in_path("gycmap.xml", takefirst=1,
                       path=pathform(_(get_cwd(),
                                       _(Y_SITES,
                                         Y_SITE)+"glade/")));
 
  
  noop, gy.Gtk.init_check(0,);
  gy_setlocale;
  __gycmap_gist_img = gy.Gtk.Image.new();
  noop, __gycmap_gist_img.set_from_file(gist_png);
  __gycmap_gist_names=
    ["gray", "yarg", "heat", "earth", "stern", "rainbow", "ncar"];

  __gycmap_msh_img = gy.Gtk.Image.new();
  noop, __gycmap_msh_img.set_from_file(png_dir+"/msh-cmap.png");

  __gycmap_mpl_img = gy.Gtk.Image.new();
  noop, __gycmap_mpl_img.set_from_file(png_dir+"/mpl-cmap.png");
   
  __gycmap_gpl_img = gy.Gtk.Image.new();
  noop, __gycmap_gpl_img.set_from_file(png_dir+"/gpl-cmap.png");

  __gycmap_gmt_img = gy.Gtk.Image.new();
  noop, __gycmap_gmt_img.set_from_file(png_dir+"/gmt-cmap.png");

  __gycmap_div_img = gy.Gtk.Image.new();
  noop, __gycmap_div_img.set_from_file(png_dir+"/cbc-div-cmap.png");

  __gycmap_seq_img = gy.Gtk.Image.new();
  noop, __gycmap_seq_img.set_from_file(png_dir+"/cbc-seq-cmap.png");

  __gycmap_qual_img = gy.Gtk.Image.new();
  noop, __gycmap_qual_img.set_from_file(png_dir+"/cb-qual-cmap.png");
  
  __gycmap_builder = gy.Gtk.Builder.new();
  noop, __gycmap_builder.add_from_file(glade);
  __gycmap_win = __gycmap_builder.get_object("window1");
  __gycmap_ebox = __gycmap_builder.get_object("eventbox");

  noop, __gycmap_ebox.add(__gycmap_gist_img);
  __gycmap_cur_img = __gycmap_gist_img;
  __gycmap_cur_names = __gycmap_gist_names;
  __gycmap_cmd=gistct;
  
  combo=__gycmap_builder.get_object("combobox");
  noop, combo.set_active_id("gist");
  gy_signal_connect, combo, "changed", __gycmap_combo_changed;
  
  gy_signal_connect, __gycmap_ebox, "button-press-event", __gycmap_callback;
  gy_gtk_window_suspend, __gycmap_win;
  noop, __gycmap_win.set_title("Yorick color table chooser");

  gy_gtk_entry_include, __gycmap_builder.get_object("entry");
  __gycmap_initialized=1;
}

func __gycmap_callback(widget, event) {
  extern __gycmap_cur_names;
  ev = gy.Gdk.EventButton(event);
  ev, x, x, y, y;
  __gycmap_cmd, __gycmap_cur_names(long(y/19)+1);
}

func __gycmap_combo_changed(widget, event) {
  extern __gycmap_cur_img, __gycmap_cur_names, __gycmap_cmd;

  lst = widget.get_active_id();
  noop, __gycmap_ebox.remove(__gycmap_cur_img);
  if (lst == "gist") {
    __gycmap_cur_img=__gycmap_gist_img;
    __gycmap_cur_names=__gycmap_gist_names;
    __gycmap_cmd=gistct;
  } else if (lst == "msh") {
    __gycmap_cur_img=__gycmap_msh_img;
    __gycmap_cur_names=
      ["coolwarm", "blutan", "ornpur", "grnred",
       "purple", "blue", "green", "red", "brown"];
    __gycmap_cmd=mshct;
  } else if (lst == "mpl") {
    __gycmap_cur_img=__gycmap_mpl_img;
    __gycmap_cur_names=
      ["binary", "gray", "bone", "pink", "copper", "winter",
       "spring", "summer", "autumn", "hot", "afmhot", "coolwarm",
       "cool", "rainbow", "terrain", "jet", "spectral", "hsv",
       "flag", "prism", "seismic", "bwr", "brg"];    
    __gycmap_cmd=mplct;
  } else if (lst == "gmt") {
    __gycmap_cur_img=__gycmap_gmt_img;
    __gycmap_cur_names=
      ["cool", "copper", "cyclic", "drywet", "gebco", "globe", "gray", "haxby",
       "hot", "jet", "nighttime", "no_green", "ocean", "paired", "panoply",
       "polar", "rainbow", "red2green", "relief", "sealand", "seis", "split",
       "topo", "wysiwyg"];
    __gycmap_cmd=gmtct;
  } else if (lst == "gpl") {
    __gycmap_cur_img=__gycmap_gpl_img;
    __gycmap_cur_names=
      ["ocean", "gnu_hot", "gnuplot", "gnuplot2",
       "gnuplot3", "gnuplot4", "gnuplot5"];
    __gycmap_cmd=cmap;
  } else if (lst == "cb-seq") {
    __gycmap_cur_img=__gycmap_seq_img;
    __gycmap_cur_names=
      ["Greys", "Purples", "Blues", "Greens", "Oranges", "Reds",
       "PuBu", "PuBuGn", "PuRd", "BuGn", "BuPu", "GnBu", "YlGn",
       "YlGnBu", "YlOrBr", "YlOrRd", "OrRd", "RdPu"];
    __gycmap_cmd=cmap;
  } else if (lst == "cb-div") {
    __gycmap_cur_img=__gycmap_div_img;
    __gycmap_cur_names=
      ["BrBG", "PRGn", "PiYG", "PuOr", "RdBu",
       "RdGy", "RdYlBu", "RdYlGn", "Spectral"];
    __gycmap_cmd=cmap;
  } else if (lst == "cb-qual") {
    __gycmap_cur_img=__gycmap_qual_img;
    __gycmap_cur_names=
      ["Set1", "Pastel1", "Dark2", "Set2", "Pastel2",
       "Set3", "Paired", "Accent"];
    __gycmap_cmd=cmap;
  } else error, "unrecognized colormap kind";
  noop, __gycmap_ebox.add(__gycmap_cur_img);
  noop, __gycmap_cur_img.show();
}

func gycmap(void)
/* DOCUMENT gycmap
   
    A graphical wrapper around the cmap family of functions, gycmap
    allows the user to interactively select a color table by viewing
    sample colorbars and clicking on the bar of her choice. If an
    image is displayed in the current Yorick graphic window, the new
    colormap is applied immediately. cmap_test can be used for
    displaying a test image.

    gycmap requires a recent version of Yorick (from git, as of
    2013-04).

   SEE ALSO: cmap, cmap_test
 */
{
  extern __gycmap_initialized, __gycmap_builder, __gycmap_win, __gycmap_ebox;
  if (!__gycmap_initialized) __gycmap_init;
  noop, __gycmap_win.show_all();
  noop, gy.Gtk.main();
}

//// gywindow: a yorick window wrapper

func __gywindow_on_error (void) {
  extern __gywindow_device;
  noop, __gywindow_device.ungrab(gy.Gdk.CURRENT_TIME);
}

func __gywindow_event_handler(widget, event) {
  extern __gywindow_win, __gywindow_xlabel, __gywindow_ylabel,
    __gywindow_xs0, __gywindow_ys0, __gywindow_device;

  after_error=__gywindow_on_error;
    
  ev = gy.Gdk.EventAny(event);
  ev, type, type;

  Gdk = gy.Gdk;
  Gtk = gy.Gtk;
  EventType=Gdk.EventType;
  
  if (type == EventType.map) {
    window, parent=gy_xid(widget), ypos=-24 ;
    if (Gtk.main_level()) gy_gtk_idleonce;
    return;
  }
  
  if (type == EventType.enter_notify) {
    __gywindow_device = Gdk.Device(Gtk.get_current_event_device());
    win = gy_gdk_window(widget);
    noop, __gywindow_device.grab(win, Gdk.GrabOwnership.none, 1,
                                 Gdk.EventMask.all_events_mask,
                                 ,
                                 Gdk.CURRENT_TIME);
    return;
  }

  if (type == EventType.leave_notify) {
    /*
    device = Gdk.Device(Gtk.get_current_event_device());
    noop, device.ungrab(Gdk.CURRENT_TIME);
    */
    return;
  }

  if (type == EventType.button_release ||
      type == EventType.button_press ||
      type == EventType.motion_notify) {
    ev = Gdk.EventButton(ev);
    ev, x, x, y, y, button, button;
    // ll in NDC: 0.1165 0.3545
    // ur in NDC: 0.6790 0.9170
    xndc=0.1165+(0.5625/450.)*(x-2);
    yndc=0.9170-(0.5625/450.)*(y-1);

    vp = viewport();
    lm = limits();

    if (xndc < vp(1)) xs = lm(1);
    else if (xndc>vp(2)) xs=lm(2);
    else xs = lm(1)+(xndc-vp(1))*(lm(2)-lm(1))/(vp(2)-vp(1));

    if (yndc < vp(3)) ys = lm(3);
    else if (yndc>vp(4)) ys=lm(4);
    else ys = lm(3)+(yndc-vp(3))*(lm(4)-lm(3))/(vp(4)-vp(3));
  }

  if (type == EventType.button_press) {
    if (xndc >= vp(1) && xndc <= vp(2)) __gywindow_xs0=xs;
    else __gywindow_xs0=[];
    if (yndc >= vp(3) && yndc <= vp(4)) __gywindow_ys0=ys;
    else __gywindow_ys0=[];
    return;
  }

  if (type == EventType.button_release) {
    lm2=lm;
    fact=1.;
    if (button==1) fact=2./3.;
    else if (button==3) fact=1.5;

    flags=long(lm(5));
    xlog = flags & 128;
    ylog = flags & 256;
    
    if (!is_void(__gywindow_xs0)) {
      if (xlog) {
        __gywindow_xs0=log10(__gywindow_xs0);
        xs=log10(xs);
        lm(1:2)=log10(lm(1:2));
        lm2(1)=__gywindow_xs0 - (xs-lm(1))*fact;
        lm2(2)=__gywindow_xs0 - (xs-lm(2))*fact;
        lm2(1:2)=10^(lm2(1:2));
      } else {
        lm2(1)=__gywindow_xs0 - (xs-lm(1))*fact;
        lm2(2)=__gywindow_xs0 - (xs-lm(2))*fact;
      }
      limits, lm2(1), lm2(2);
    }

    if (!is_void(__gywindow_ys0)) {
      lm2(3)=__gywindow_ys0 - (ys-lm(3))*fact;
      lm2(4)=__gywindow_ys0 - (ys-lm(4))*fact;
      range, lm2(3), lm2(4);
    }
    
    gy_gtk_idleonce;
    return;
  }

  if (type == EventType.motion_notify) {
    if (x<0 || y<0 || x>455 || y>455) {
      noop, __gywindow_device.ungrab(Gdk.CURRENT_TIME);
      return;
    }
    noop, __gywindow_xlabel.set_text(pr1(xs));
    noop, __gywindow_ylabel.set_text(pr1(ys));
    return;
  }
  
  //write, format="in __gywindow_realized. event: %d\n", type;
}

func gy_gtk_idleonce(void)
/* DOCUMENT gygtk_idleonce

     Display in Yorick graphic windows is updated only when Yorick is
     idle, which never occurs when a gy.Gtk.main loop is running. This
     procedure lets Yorick reach the idle state to update its graphics
     and restarts the Gtk main loop. You should call it from Gtk
     applications each time the graphics should be updated.

   SEE ALSO: gy, gyterm, gywindow
 */
{
  noop, gy.Gtk.main_quit();
  set_idler, __gyterm_idler;
}

func __gywindow_init(void) {
  extern __gywindow_win, __gywindow_xlabel, __gywindow_ylabel;
  Gtk=gy.Gtk;
  noop, Gtk.init_check(0,);
  gy_setlocale;
  __gywindow_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  gy_gtk_window_suspend, __gywindow_win;
  box=Gtk.Box.new(Gtk.Orientation.vertical, 0);
  noop, __gywindow_win.add(box);

  box2=Gtk.Box.new(Gtk.Orientation.horizontal, 0);
  noop, box.pack_start(box2, 0,0,0);

  label=Gtk.Label.new(" x = ");
  noop, box2.pack_start(label, 0, 0, 0);
  __gywindow_xlabel=Gtk.Label.new("");
  noop, box2.pack_start(__gywindow_xlabel, 0, 0, 0);
    
  label=Gtk.Label.new(" y = ");
  noop, box2.pack_start(label, 0, 0, 0);
  __gywindow_ylabel=Gtk.Label.new("");
  noop, box2.pack_start(__gywindow_ylabel, 0, 0, 0);

  da = Gtk.DrawingArea.new();
  gy_signal_connect, da, "event", __gywindow_event_handler;
  noop, da.add_events(gy.Gdk.EventMask.all_events_mask);
  noop, da.set_size_request(454, 453);
  noop, box.pack_start(da, 1, 1, 0);

  entry=Gtk.Entry.new();
  gy_gtk_entry_include, entry;
  noop, box.pack_start(entry, 1,1,0);

}

func gywindow(wid)
/* DOCUMENT gywindow
   *** work in progress ***

   When the Gtk main loop is running, the Yorick main loop is
   stuck. This prevents things like window zooming to happen.

   The goal of gywindow is to provide a Yorick window wrapper which
   would emulate Yorick's native behavior for Gtk-embedded windows.

   SEE ALSO: gyterm
*/
{
  if (is_void(__gywindow_win)) __gywindow_init;
  noop, __gywindow_win.show_all();
  noop, gy.Gtk.main();
}

/// hack: should be done from gi
extern gy_gdk_window;
/* DOCUMENT gdkwin = gy_gdk_window(gtkwidget)
   
     Get low-level Gdk "window" associated with a given Gtk widget.

     As of now, this is a compiled routnie as the interpreted
     interface is not sufficiently developed to get it directly.

     Once the interpreted inteface allows doing that, this function
     may or may not remain as an interpreted wrapper, depending on how
     complex it is.
     
   SEE ALSO: gy, gy_xid
 */

extern gy_xid;
/* DOCUMENT id=gy_xid(wdg)
   
     Get X11 window ID associated with widget WDG.
     This allows displaying a Yorick window inside a Gtk widget.

   EXAMPLE:
     builder=gy.Gtk.Builder.new(glade_file);
     ywin = builder.get_object(yorick_widget_name);
     func on_ywin_event(void) {
       gy_gtk_idleonce;
       window, parent=gy_xid(ywin);
     }
     gy_signal_connect, ywin, "event", on_ywin_event;
     
   SEE ALSO: gy, gy_gdk_window
 */
/*
{
  // For some reason, this interpreted version does not work:
  // the yorick window appears on top of the drawing area,
  // but detached, with its own window borders!
  gy.GdkX11.X11Window(gy_gdk_window(__gywindow_win)).get_xid();
}
*/


extern gy_debug;
/* DOCUMENT mode = gy_debug();
         or gy_debug, mode;
    Get or set gy debug mode.
   SEE ALSO: gy
*/

extern gy_setlocale;
/* DOCUMENT gy_setlocale, [category,] locale
         or locale=gy_setlocale()

     Get or set locale used in the Yorick process.

     Allways resets LC_NUMERIC to "C" as other values may break
     Yorick.

   SEE ALSO: gy
 */

//extern gy_thread;
