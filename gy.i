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

     - gyhelloworld is a trivial example.

    gyterm and gywindow can be easily embedded in custom applications
    (indeed, gyterm *is* embedded in both gycmap and gywindow). See
    gy_gtk_ycmd and gy_gtk_ywindow.

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
    To load a specific version of a namespace (good idea for public
    code):
      Gtk = gy.require("Gtk", "3.0");

    You can use gy_list_namespace to list the symbols inside this
    namespace:
      gy_list_namespace, Gtk;
    Objects are typically instanciated using a "new" method:
      button = gy.Gtk.Button.new();
    Object members can be listed with gy_list_object:
      gy_list_object, button;
    Callbacks can be connected to objects using gy_signal_connect.

    gy simply exposes conforming library content to Yorick. See the
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
    // Load namespace, checking version
    Gtk = gy.require("Gtk", "3.0");

    // Initialize Gtk
    Gtk.init_check(0,);

    // Gtk.init messes with the local, reset at least LC_NUMERIC
    gy_setlocale;

    // Create widget hierarchy
    win = Gtk.Window.new(Gtk.WindowType.toplevel);
    button = Gtk.Button.new_with_label("Hello World!");
    win.add(button);

    // Write a callback
    func hello(widget, event, data) {
      "\"Hello World!\"";
    }

    // Connect callback to button event
    gy_signal_connect, button, "clicked", hello;

    // Connect standard "delete" event to window, show window,
    // count it among the managed windows, and start Gtk main loop
    gy_gtk_main, win;
    
   SEE ALSO: gyterm, gycmap, gywindow
 */

extern gy_init;
/* DOCUMENT gy=gy_init()
    Initialize gy.
 */
gy=gy_init();

extern gy_list_namespace;
/* DOCUMENT gy_list_namespace, NAMESPACE
   
    List symbols in NAMESPACE.  Beware: the list can contain thousands
    of entries.

   EXAMPLES:
    gy_list_namespace, "Gtk"
    gy_list_namespace, gy.require("Gtk", "3.0")
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
         or gy_connect_signal, builder
   
    Connect signal to signal handler.
    
    The handler must accept all the parameters described in the C
    documentation for the signal.

   ARGUMENTS:
    builder: if first argument is a Gtk Builder object, the signals
             information it contains will be used to automatically
             connect GObject signals to Yorick interpreted functions.
             Whether or not the Yorick functions actually exists is
             not checked.
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
  Gtk=gy.require("Gtk");
  noop, Gtk.init_check(0,);
  gy_setlocale;
  __gyterm_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, __gyterm_win.set_title("Yorick command line");
  entry = gy_gtk_ycmd(1);
  noop, __gyterm_win.add(entry);
  noop, entry.set_width_chars(80);
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
    noop, Gtk.Editable(widget).set_position(-1);
    return 1;
  }
  if (keyval==Gdk.KEY_Down) {
    __gyterm_idx=(__gyterm_idx==__gyterm_max)?1:__gyterm_idx+1;
    noop, widget.set_text(__gyterm_history(__gyterm_idx));
    noop, Gtk.Editable(widget).set_position(-1);
    return 1;
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
    return 1;
  }

  return 0;
}

func gy_gtk_ycmd(noexpander)
/* DOCUMENT widget = gy_gtk_ycmd (noexpander)
   
    Create a Gtk widget to type Yorick commands, similar to
    gyterm. Unless NOEXPANDER is specified and evaluates to true, the
    Gtk entry is put in an expander.

   SEE ALSO: gy, gyentry, gy_gtk_ycmd_connect, gy_gtk_ywindow
 */
{
  Gtk=gy.require("Gtk", "3.0");
  entry = Gtk.Entry.new();
  gy_gtk_ycmd_connect, entry;
  if (noexpander) return entry;
  exp = Gtk.Expander.new("<small><span style=\"italic\" size=\"smaller\">Yorick command</span></small>");
  exp.add(entry);
  exp.set_use_markup(1);
  exp.set_resize_toplevel(1);
  return exp;
}

func gy_gtk_ycmd_connect(widget) {
/* DOCUMENT gy_gtk_ycmd_connect, entry_widget
   
    Makes entry_widget mimick gyterm behavior.

   EXAMPLE
    entry=gy.Gtk.Entry.new();
    gy_gtk_ycmd_connect, entry;

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
    noop, gy.require("Gtk", "3.0").init(0,);
    gy_setlocale;
    win=gy.Gtk.Window.new(gy.Gtk.WindowType.toplevel);
    gy_gtk_main, win;

   SEE ALSO: gy, gyterm, gy_gtk_ycmd_connect
 */
{
  gy_signal_connect, win, "delete-event", __gyterm_suspend;
  //  gy_signal_connect, win, "destroy", __gyterm_destroy;
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
  return 1;
}

func gyterm(cmd)
/* DOCUMENT gyterm
   
     Open a window containing a single line in which arbitrary Yorick
     commands can be typed.

     If you want to keep a command line around while launching another
     gy-based, blocking GUI, simpy launch it from gyterm.

     If you want to embed gyterm in another GUI, see
     gy_gtk_ycmd_connect.

   SEE ALSO: gy, gy_gtk_ycmd_connect, gycmap, gywindow
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

  Gtk=gy.require("Gtk", "3.0");
  
  gist_png = find_in_path("gist-cmap.png", takefirst=1,
                          path=pathform(_(get_cwd(),
                                          _(Y_SITES,
                                            Y_SITE)+"data/")));
  png_dir=dirname(gist_png);
  
  glade = find_in_path("gycmap.xml", takefirst=1,
                       path=pathform(_(get_cwd(),
                                       _(Y_SITES,
                                         Y_SITE)+"glade/")));
 
  
  noop, Gtk.init_check(0,);
  gy_setlocale;
  __gycmap_gist_img = Gtk.Image.new();
  noop, __gycmap_gist_img.set_from_file(gist_png);
  __gycmap_gist_names=
    ["gray", "yarg", "heat", "earth", "stern", "rainbow", "ncar"];

  __gycmap_msh_img = Gtk.Image.new();
  noop, __gycmap_msh_img.set_from_file(png_dir+"/msh-cmap.png");

  __gycmap_mpl_img = Gtk.Image.new();
  noop, __gycmap_mpl_img.set_from_file(png_dir+"/mpl-cmap.png");
   
  __gycmap_gpl_img = Gtk.Image.new();
  noop, __gycmap_gpl_img.set_from_file(png_dir+"/gpl-cmap.png");

  __gycmap_gmt_img = Gtk.Image.new();
  noop, __gycmap_gmt_img.set_from_file(png_dir+"/gmt-cmap.png");

  __gycmap_div_img = Gtk.Image.new();
  noop, __gycmap_div_img.set_from_file(png_dir+"/cbc-div-cmap.png");

  __gycmap_seq_img = Gtk.Image.new();
  noop, __gycmap_seq_img.set_from_file(png_dir+"/cbc-seq-cmap.png");

  __gycmap_qual_img = Gtk.Image.new();
  noop, __gycmap_qual_img.set_from_file(png_dir+"/cb-qual-cmap.png");
  
  __gycmap_builder = Gtk.Builder.new();
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

  noop, __gycmap_builder.get_object("box1").add(gy_gtk_ycmd());

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

func gywinkill(yid)
/* DOCUMENT gywinkill, yid
     Forget all about gywindow YID.
   SEE ALSO gywindow
 */
{
  extern __gywindow;
  tmp = save();
  n = __gywindow(*);
  for (i=1; i<=n; ++i) {
    if (__gywindow( (nothing=i) ).yid==yid) {
      winkill, yid;
      gy_gtk_destroy, __gywindow( (nothing=i) ).win;
    } else save, tmp, "", __gywindow( (nothing=i) );
  }
  __gywindow = tmp;
}

func gy_gtk_destroy(win)
/* DOCUMENT gy_gtk_destroy, win

     Hide window WIN and remove it from the lsit of managed windows.

   SEE ALSO: gy_gtk_main
 */
{
  extern __gygtk_windows;
  __gyterm_suspend, win; 
  idx = where (__gygtk_windows(1,)!=gy_id(win));
  if (numberof(idx)) __gygtk_windows = __gygtk_windows(,idx);
  else __gygtk_windows = [];
  // noop, win.destroy();
}

func gy_gtk_ywindow_reinit(yid, dpi=, style=)
/* DOCUMENT gy_gtk_ywindow_reinit, yid
   
     RE-initialize Yorick window YID attached to a gy widget, for
     instance to change DPI or STYLE.

   KEYWORDS: dpi, style.
   SEE ALSO: gy, gywindow, gy_gtk_ywindow
 */
{
  cur = __gywindow_find_by_yid(yid);
  if (!is_void(dpi)) {
    if (dpi==0) dpi=75;
    save, cur, dpi;
  }
  if (!is_void(style)) {
    if (style==0) style="work.gs";
    save, cur, style;
  }
  winkill, cur.yid;
  window, cur.yid, parent=cur.xid, ypos=-24, dpi=cur.dpi,
    width=long(8.5*cur.dpi), height=long(11*cur.dpi), style=cur.style;
}

func __gywindow_event_handler(widget, event) {
  extern __gywindow, __gywindow_xs0, __gywindow_ys0, __gywindow_device;
  local curwin, win;
  
  after_error=__gywindow_on_error;

  cur = __gywindow_find_by_xid(gy_xid(widget));
  if (is_void(cur)) return;

  Gtk = gy.require("Gtk", "3.0");
  Gdk = gy.Gdk;
  EventType=Gdk.EventType;

  ev = Gdk.EventAny(event);
  ev, type, type;
  
  if (type == EventType.map && !cur.realized) {
    window, cur.yid, parent=gy_xid(widget), ypos=-24, dpi=cur.dpi,
      width=long(8.5*cur.dpi), height=long(11*cur.dpi), style=cur.style;
    save, cur, realized=1;

    sw = widget.get_parent().get_parent();

    xcenter = long(4.25*cur.dpi);
    
    hadjustment = sw.get_hadjustment();
    noop, hadjustment.set_value(xcenter-hadjustment.get_page_size()/2);
  
    vadjustment = sw.get_vadjustment();
    noop, vadjustment.set_value(xcenter-vadjustment.get_page_size()/2);

    save, cur, hadjustment, vadjustment;

    if (Gtk.main_level()) gy_gtk_idleonce;
    return;
  }

  if (!cur.realized) return;

  pix2ndc=72.27/cur.dpi*0.0013;
  
  if (type == EventType.enter_notify) {
    __gywindow_device = Gdk.Device(Gtk.get_current_event_device());
    noop, Gtk.Widget(widget)(window, win);
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
        
    xndc=pix2ndc*(x-2);
    yndc=11.*72.27*0.0013-pix2ndc*(y-1);

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
    if (x<cur.hadjustment.get_value() ||
        y<cur.vadjustment.get_value() ||
        x>cur.hadjustment.get_value()+cur.hadjustment.get_page_size() ||
        y>cur.vadjustment.get_value()+cur.vadjustment.get_page_size()) {
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

func gy_gtk_ywindow_connect(&yid, win, da, xylabel, dpi=, style=)
/* DOCUMENT gy_gtk_ywindow_connect, yid, win, da, xylabel
   
    Connect widgets to embed a Yorick window in a Gtk DrawingArea (see
    gywindow for a trivial example). For a lower level function, see
    gy_gtk_ywindow.

     If YID is nil, a new ID is taken and YID is set to this value.

   ARGUMENTS
    yid: Yorick window ID to embed
    win: the toplevel gy.Gtk.Window widget
    da:  the gy.Gtk.DrawingArea in which the yorick window will be embedded.
    xylabel: gy.Gtk.Entry widget in which to report mouse motion.

   SEE ALSO: gy, gywindow, gy_gtk_ywindow
 */
{
  extern __gywindow;
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id();
  if (is_void(yid)) error, "unable to find free id";
  
  if (is_void(dpi)) dpi=75;
  gy_signal_connect, da, "event", __gywindow_event_handler;
  gy_signal_connect, da, "configure-event", __gywindow_redraw;
  gy_signal_connect, da, "draw", __gywindow_redraw;
  save, __gywindow, "", save(yid, xid=[], win, da, xylabel,
                             realized=0, dpi=dpi, style=style);
}


func __gywindow_redraw(widg, event, userdata) {
  gy_gtk_idleonce;
  return 1;
}

func gy_gtk_ywindow(&yid, dpi=, width=, height=, style=)
/* DOCUMENT widget = gy_gtk_ywindow(yid)

     Initialize a Gtk widget embedding Yorick window number YID. The
     widget is scrollable and provides mouse position reading and
     zoom/pan capabilities. The widget is connected using
     gy_gtk_ywindow_connect.

     If YID is nil, a new ID is taken and YID is set to this value.

   KEYWORDS: dpi, width, height, style.
   SEE ALSO: gy, gywindow, gyterm, gycmap, gy_gtk_ywindow_connect
 */
{
  extern __gywindow;
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id();
  if (is_void(yid)) error, "unable to find free id";
  
  Gtk = gy.require("Gtk", "3.0");
  box = Gtk.Box.new(Gtk.Orientation.vertical, 0);

  box2=Gtk.Box.new(Gtk.Orientation.horizontal, 0);
  noop, box.pack_start(box2, 0,0,0);

  xylabel=Gtk.Label.new("");
  noop, box2.pack_start(xylabel, 0, 0, 0);


  sw = Gtk.ScrolledWindow.new(,);
  noop, sw.set_size_request(width,height); 
  noop, box.pack_start(sw, 1, 1, 0);

  tmp = Gtk.Viewport.new(,);
  noop, sw.add(tmp);
  
  da = Gtk.DrawingArea.new();
  noop, da.add_events(gy.Gdk.EventMask.all_events_mask);
  noop, da.set_size_request(long(8.5*dpi),long(11*dpi));
  noop, tmp.add(da);

  gy_gtk_ywindow_connect, yid, win, da, xylabel, dpi=dpi, style=style;

  return box;
}

func __gywindow_init(&yid, dpi=, width=, height=, style=) {
  extern __gywindow, adj;
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id();
  if (is_void(yid)) error, "unable to find free id";
  if (is_void(dpi)) dpi=75;
  if (is_void(width)) width=long(6*dpi);
  if (is_void(height)) height=long(6*dpi);
  Gtk = gy.require("Gtk", "3.0");
  noop, Gtk.init_check(0,);
  gy_setlocale;
  win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, win.set_title("Yorick "+pr1(yid));
  box=Gtk.Box.new(Gtk.Orientation.vertical, 0);
  noop, win.add(box);

  
  noop, box.add(gy_gtk_ywindow(yid,
                               dpi=dpi, width=width, height=height,
                               style=style));

  noop, box.pack_start(gy_gtk_ycmd(), 0,1,0);
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

func gy_gtk_ywindow_free_id(void)
/* DOCUMENT yid = gy_gtk_ywindow_free_id();
   
     Find Yorick window ID not yet used by a gywindow. It is not
     guaranteed that this id is not use by a non-GTK Yorick window.

   SEE ALSO: gywindow
*/
{
  extern __gywindow;
  n = __gywindow(*);
  free = array(1, 64);
  for (i=1; i<=n; ++i) free(__gywindow( (nothing=i) ).yid+1)=0;
  ids = where(free);
  if (!numberof(ids)) return;
  return ids(0)-1;
}

func gywindow(&yid, freeid=, dpi=, width=, height=, style=)
/* DOCUMENT gywindow, yid

    When the Gtk main loop is running, the Yorick main loop is
    stuck. This prevents things like window zooming to
    happen. gywindow embeds Yorick window YID in a Gtk window and
    emulates zooming etc. If YID already exists, it is first killed
    and its content erased.

    If you are interested in embedding a Yorick window in your own
    application, have a look at gy_gtk_ywindow.

   KEYWORDS:
    freeid: if true, a new ID is foound using gy_gtk_ywindow_free_id.
    dpi, width, height, style: see window

   SEE ALSO: gyterm, gy_gtk_ywindow, gy_gtk_ywindow_free_id, window,
             gywinkill
*/
{
  if (freeid) {
    yid = gy_gtk_ywindow_free_id();
    if (is_void(yid)) error, "unable to find free id";
  }
  if (is_void(yid)) {
    yid = current_window();
    if (yid<0) yid=0;
  }
  if (is_void(__gywindow_find_by_yid(yid)))
    {
      winkill, yid;
      __gywindow_init, yid,
        dpi=dpi, width=width, height=height, style=style;
    }
  gy_gtk_main, __gywindow_find_by_yid(yid).win;
}

//// error message

func gyerror(msg)
/* DOCUMENT gyerror, msg
     Display error message in a Gtk window.
 */
{
  aspect = 1.61803398875; // nombre d'or
  Gtk = gy.require("Gtk", "3.0");
  noop, Gtk.init(0, );
  gy_setlocale;
  win = Gtk.Dialog.new();
  noop, win.set_title("Yorick error");
  noop, win.set_size_request(long(200*aspect), 200);
  hbox = Gtk.Box.new(Gtk.Orientation.horizontal, 0);
  vbox = Gtk.Box.new(Gtk.Orientation.vertical, 0);
  noop, hbox.add(Gtk.Label.new("   "));
  noop, hbox.add(Gtk.Image.new_from_stock(Gtk.STOCK_DIALOG_ERROR,
                                          Gtk.IconSize.dialog));
  noop, hbox.add(Gtk.Label.new("   "));
  noop, hbox.add(vbox);
  noop, hbox.add(Gtk.Label.new("   "));
  noop, vbox.add(Gtk.Label.new("Yorick error:"));
  noop, vbox.add(Gtk.Label.new(msg));
  noop, win.get_content_area().add(Gtk.Label.new(""));
  noop, win.get_content_area().add(hbox);
  noop, win.get_content_area().add(Gtk.Label.new(""));
  noop, win.get_content_area().set_homogeneous(1);
  noop, win.add_button(Gtk.STOCK_OK, Gtk.ResponseType.ok);
  noop, win.show_all();
  noop, win.run();
  noop, win.destroy();
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


 extern gy_id;
 /* DOCUMENT id = gy_id(object)
    
      Get unique id of gy object. Two variables may hold the same
      underlying object: the id is unique.

    SEE ALSO: gy
  */

extern gy_gtk_builder_connector;
