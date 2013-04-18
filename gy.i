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

    gyterm and gywindow can be easily embedded in custom applications
    (indeed, gyterm *is* embedded in both gycmap and gywindow).

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
    limitation, see gyterm. On the other hand, that means that
    callbacks are called almost synchronously, so applications are
    easier to code.

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
    gy_gtk_main, win;
    
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

    Connect signal to signal handler.
    
    The handler must accept all the parameters described in the C
    documentation for the signal. Callbacks which need to return a
    value must use gy_return to do so.

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
  extern __gyterm_initialized, __gyterm_win, __gyterm_entry;
  Gtk=gy.Gtk;
  noop, Gtk.init_check(0,);
  gy_setlocale;
  __gyterm_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, __gyterm_win.set_title("Yorick command line");
  __gyterm_entry = Gtk.Entry.new();
  gy_gtk_entry_include, __gyterm_entry;
  noop, __gyterm_win.add(__gyterm_entry);
  noop, __gyterm_entry.set_width_chars(80);
  __gyterm_initialized=1;
}  

func __gyterm_idler(void) {
  noop, gy.Gtk.main();
}

if (is_void(__gyterm_history_size)) __gyterm_history_size=100;
__gyterm_history=array(string, __gyterm_history_size);
__gyterm_idx=__gyterm_cur=1;
__gyterm_max=0;

func __gyterm_key_pressed(widget, event) {
  extern __gyterm_history, __gyterm_cur, __gyterm_max, __gyterm_idx;

  Gdk=gy.Gdk;
  Gtk=gy.Gtk;
  ev = Gdk.EventKey(event);
  ev, keyval, keyval;
  if (keyval==Gdk.KEY_Up) {
    if (__gyterm_idx==__gyterm_cur) {
      __gyterm_history(__gyterm_cur)=widget.get_text();
      if (__gyterm_max<__gyterm_cur) __gyterm_max=__gyterm_cur;
    }
    __gyterm_idx=(__gyterm_idx==1)?__gyterm_max:__gyterm_idx-1;
    noop, widget.set_text(__gyterm_history(__gyterm_idx));
    gy_return, 1;
    return;
  }
  if (keyval==Gdk.KEY_Down) {
    __gyterm_idx=(__gyterm_idx==__gyterm_max)?1:__gyterm_idx+1;
    noop, widget.set_text(__gyterm_history(__gyterm_idx));
    gy_return, 1;
    return;
  }

  if (keyval==Gdk.KEY_Return) {
    gy_gtk_idleonce;
    cmd=widget.get_text();
    noop, widget.set_text("");
    __gyterm_history(__gyterm_cur)=cmd;
    __gyterm_cur=(__gyterm_cur<numberof(__gyterm_history))?__gyterm_cur+1:1;
    __gyterm_max=min(numberof(__gyterm_history), __gyterm_max+1);
    __gyterm_idx=__gyterm_cur;
    write, format="> %s\n", cmd;
    include, strchar("if (catch(-1)) {gyerror, catch_message; return;} "+cmd), 1;
    gy_return, 1;
    return;
  }

  gy_return, 0;
  return;
}

func gy_gtk_entry_include(widget) {
/* DOCUMENT gy_gtk_entry_include, entry_widget
   
    Makes entry_widget mimick gyterm behavior.

   EXAMPLE
    entry=gy.Gtk.Entry.new();
    gy_gtk_entry_include, entry;

   SEE ALSO: gy, gyterm, gy_gtk_window_suspend, gy_gtk_main
 */
  gy_signal_connect, widget, "key-press-event", __gyterm_key_pressed;
  noop, widget.set_placeholder_text("Yorick command");
  noop, widget.set_tooltip_text("Yorick command");
}

func gy_gtk_window_suspend(win)
/* DOCUMENT gy_gtk_window_suspend, window
   
    Connect a standard handler to the delete event of WINDOW.

    Windows using gy_gtk_window_suspend should do so through
    gy_gtk_main.
    
   EXAMPLE
    noop, gy.Gtk.init(0,);
    gy_setlocale;
    win=gy.Gtk.Window.new(gy.Gtk.WindowType.toplevel);
    gy_gtk_main, win;

   SEE ALSO: gy, gyterm, gy_gtk_entry_include
 */
{
  gy_signal_connect, win, "delete-event", __gyterm_suspend;
  gy_signal_connect, win, "destroy", __gyterm_destroy;
}

func __gyterm_destroy(widget) {
  write, "destroy called on "+ pr1(widget)+"\n";
}

func __gyterm_suspend(widget, event) {
  extern __gygtk_windows;
  idx = where (__gygtk_windows(1,)==gy_id(widget));
  if (!numberof(idx)) error("window is not managed");
  __gygtk_windows(2,idx) = 0;
  noop, widget.hide();
  if (noneof(__gygtk_windows(2,))) noop, gy.Gtk.main_quit();
  gy_return, 1;
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
  gy_gtk_main, __gyterm_win;
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
  gy_gtk_main, __gycmap_win;
}

//// gywindow: a yorick window wrapper

func __gywindow_on_error (void) {
  extern __gywindow_device;
  noop, __gywindow_device.ungrab(gy.Gdk.CURRENT_TIME);
}

func __gywindow_event_handler(widget, event) {
  extern __gywindow, __gywindow_xs0, __gywindow_ys0, __gywindow_device;
  local curwin, win;
  
  after_error=__gywindow_on_error;

  cur = __gywindow_find_by_xid(gy_xid(widget));
  if (is_void(cur)) return;

  
  ev = gy.Gdk.EventAny(event);
  ev, type, type;

  Gdk = gy.Gdk;
  Gtk = gy.Gtk;
  EventType=Gdk.EventType;
  
  if (type == EventType.map) {
    window, cur.yid, parent=gy_xid(widget), ypos=-24 ;
    save, cur, realized=1;
    if (Gtk.main_level()) gy_gtk_idleonce;
    return;
  }

  if (!cur.realized) return;
  
  if (type == EventType.enter_notify) {
    __gywindow_device = Gdk.Device(Gtk.get_current_event_device());
    noop, gy.Gtk.Widget(widget)(window, win);
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
    curwin = current_window();
    window, cur.yid;
    
    ev = Gdk.EventButton(ev);
    ev, x, x, y, y, button, button, state, state;

    shft = state & Gdk.ModifierType.shift_mask;
    meta = state & Gdk.ModifierType.mod1_mask;
    if (shft && !meta && button==1) button=2;
    if (meta && !shft && button==1) button=3;
        
    // ll in NDC: 0.1165 0.3545
    // ur in NDC: 0.6790 0.9170
    xndc=0.1165+(0.5625/450.)*(x-2);
    yndc=0.9170-(0.5625/450.)*(y-1);

    vp = viewport();
    lm = limits();

    flags=long(lm(5));
    xlog = flags & 128;
    ylog = flags & 256;

    if (xndc < vp(1)) xs = lm(1);
    else if (xndc>vp(2)) xs=lm(2);
    else {
      if (xlog)
        xs = lm(1) * (lm(2)/lm(1))^((xndc-vp(1))/(vp(2)-vp(1)));
      else
        xs = lm(1)+(xndc-vp(1))*(lm(2)-lm(1))/(vp(2)-vp(1));
    }

    if (yndc < vp(3)) ys = lm(3);
    else if (yndc>vp(4)) ys=lm(4);
    else {
      if (xlog)
        ys = lm(3) * (lm(4)/lm(3))^((yndc-vp(3))/(vp(4)-vp(3)));
      else
        ys = lm(3)+(yndc-vp(3))*(lm(4)-lm(3))/(vp(4)-vp(3));
    }
  }

  if (type == EventType.button_press) {
    if (xndc >= vp(1) && xndc <= vp(2)) __gywindow_xs0=xs;
    else __gywindow_xs0=[];
    if (yndc >= vp(3) && yndc <= vp(4)) __gywindow_ys0=ys;
    else __gywindow_ys0=[];
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
        lm2(1:2)=__gywindow_xs0 / (xs/lm(1:2))^fact;
      } else {
        lm2(1:2)=__gywindow_xs0 - (xs-lm(1:2))*fact;
      }
      limits, lm2(1), lm2(2);
    }

    if (!is_void(__gywindow_ys0)) {
      if (ylog) {
        lm2(3:4)=__gywindow_ys0 / (ys/lm(3:4))^fact;
      } else {
        lm2(3:4)=__gywindow_ys0 - (ys-lm(3:4))*fact;
      }
      range, lm2(3), lm2(4);
    }
    
    gy_gtk_idleonce;
  }

  if (!is_void(curwin) && curwin>=0) window, curwin;
  
  if (type == EventType.motion_notify) {
    if (x<0 || y<0 || x>455 || y>455) {
      noop, __gywindow_device.ungrab(Gdk.CURRENT_TIME);
      return;
    }
    noop, cur.xylabel.set_text("("+pr1(xs)+", "+pr1(ys)+")");
    return;
  }
  
  //write, format="in __gywindow_realized. event: %d\n", type;
}

__gygtk_windows=[];

func gy_gtk_main(win)
/* DOCUMENT gy_gtk_main, toplevel_window
   
     Show window and ensure the Gtk main loop is running, while
     keeping track of the number of windows open.
       
     Windows open this way should use gy_gtk_window_suspend to close the
     window.

   SEE ALSO: gy_gtk_window_suspend
*/ 
     
{
  extern __gygtk_windows;
  id = gy_id(win);
  if (is_void(__gygtk_windows) || noneof(__gygtk_windows(1,)==id)) {
    grow, __gygtk_windows, [[id, 0]];
    gy_gtk_window_suspend, win;
  }
  idx=where(__gygtk_windows(1,)==id)(1);
  if (__gygtk_windows(2,idx)) return;
  __gygtk_windows(2,idx)=1;
  noop, win.show_all();
  if (sum(__gygtk_windows(2,))==1) noop, gy.Gtk.main();
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

if (is_void(__gywindow)) __gywindow=save();

func gy_gtk_window_connect(yid, win, da, xylabel)
/* DOCUMENT gy_gtk_window_connect, yid, win, da, xylabel
   
    Connect widgets to embed a Yorick window in a Gtk DrawingArea (see
    gywindow for a trivial example).

   ARGUMENTS
    yid: Yorick window ID to embed
    win: the toplevel gy.Gtk.Window widget
    da:  the gy.Gtk.DrawingArea in which the yorick window will be embedded.
    xylabel: gy.Gtk.Entry widget in which to report mouse motion.

   SEE ALSO: gy, gywindow
 */
{
  extern __gywindow;
  gy_signal_connect, da, "event", __gywindow_event_handler;
  save, __gywindow, "", save(yid, xid=[], win, da, xylabel, realized=0);
}

func __gywindow_init(yid) {
  extern __gywindow;
  Gtk=gy.Gtk;
  noop, Gtk.init_check(0,);
  gy_setlocale;
  win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, win.set_title("Yorick "+pr1(yid));
  box=Gtk.Box.new(Gtk.Orientation.vertical, 0);
  noop, win.add(box);

  box2=Gtk.Box.new(Gtk.Orientation.horizontal, 0);
  noop, box.pack_start(box2, 0,0,0);

  xylabel=Gtk.Label.new("");
  noop, box2.pack_start(xylabel, 0, 0, 0);
    
  da = Gtk.DrawingArea.new();
  noop, da.add_events(gy.Gdk.EventMask.all_events_mask);
  noop, da.set_size_request(454, 453);
  noop, box.pack_start(da, 1, 1, 0);

  entry=Gtk.Entry.new();
  gy_gtk_entry_include, entry;
  noop, box.pack_start(entry, 1,1,0);
  
  gy_gtk_window_connect, yid, win, da, xylabel;
}

func __gywindow_find_by_xid(xid)
{
  extern __gywindow;
  if (is_void(__gywindow)) return;
  nwins = __gywindow(*);
  for (oid=1; oid<=nwins; ++oid) {
    cur=__gywindow( (nothing=oid) );
    cxid=cur.xid;
    if (is_void(cxid)) {
      cxid=gy_xid(cur.da);
      if (!is_void(cxid)) save, cur, xid=cxid;
    }
    if (cxid==xid) return cur;
  }
}

func __gywindow_find_by_yid(yid)
{
  extern __gywindow;
  if (is_void(__gywindow)) return;
  nwins = __gywindow(*);
  for (oid=1; oid<=nwins; ++oid) {
    if ((cur=__gywindow( (nothing=oid) )).yid==yid) return cur;
  }
}

func gywindow(yid)
/* DOCUMENT gywindow, yid

    When the Gtk main loop is running, the Yorick main loop is
    stuck. This prevents things like window zooming to
    happen. gywindow embeds Yorick window YID in a Gtk window and
    emulates zooming etc. If YID already exists, it is first killed
    and its content erased.

    If you are interested in embedding a Yorick window in your own
    application, have a look at gy_window_connect.

   SEE ALSO: gyterm, gy_window_connect
*/
{
  if (is_void(yid)) {
    yid = current_window();
    if (yid<0) yid=0;
  }
  if (is_void(__gywindow_find_by_yid(yid)))
    {
      winkill, yid;
      __gywindow_init, yid;
    }
  gy_gtk_main, __gywindow_find_by_yid(yid).win;
}

//// error message

func __gyerror_init(void) {
  extern __gyerror_win, __gyerror_msgarea, __gyerror_initialized;
  noop, gy.Gtk.init(0, );
  gy_setlocale;
  __gyerror_win = gy.Gtk.Window.new(gy.Gtk.WindowType.toplevel);
  __gyerror_msgarea=gy.Gtk.Label.new("");
  noop, __gyerror_win.add(__gyerror_msgarea);
  __gyerror_initialized=1;
}

func gyerror(msg)
/* DOCUMENT gyerror, msg
     Display error message in a Gtk window.
 */
{
  if (!__gyerror_initialized) __gyerror_init;
  noop, __gyerror_msgarea.set_text(msg);
  gy_gtk_main,__gyerror_win;
}

//// utilities

func gy_xid(wdg)
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

{
  local gdkwin;
  noop, gy.Gtk.Widget(wdg)(window, gdkwin);
  return gy.GdkX11.X11Window(gdkwin).get_xid();
}

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

extern gy_return;
/* DOCUMENT gy_return, retval

     Return value from callback.

     Some callbacks must return a value. Use gy_callback to do it.

   EXAMPLE:
     func mycallback(widget, event) {
       do_stuff;
       gy_return, 1;
     }

   SEE ALSO: gy, gy_signal_connect
 */

 extern gy_id;
 /* DOCUMENT id = gy_id(object)
    
      Get unique id of gy object. Two variables may hold the same
      underlying object: the id is unique.

    SEE ALSO: gy
  */
