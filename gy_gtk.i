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

#include "gy.i"

local gy_gtk_i;
/* DOCUMENT #include "gy_gtk.i"

    gy is a Yorick plug-in around GObject-introspection. It can
    notably be used to create Gtk GUIs from within Yorick. gy_gtk.i is
    based on gy.i and adds convenience functions such as standard
    event handlers for writing Gtk based graphical user interfaces
    (GUI).


   PROVIDED APPLICATIONS
    gy_gtk comes with a few sample GUI utilities:
    
     - gyterm is a command line in a Gtk window. It is useful for
       keeping a Yorick command line while another GUI is running.

     - gycmap is a wrapper around cmap.

     - gywindow is a wrapper for Yorick windows (work in progress).

     - gyhelloworld is a trivial example.

    gyterm and gywindow can be easily embedded in custom applications
    (indeed, gyterm *is* embedded in both gycmap and gywindow). See
    gy_gtk_ycmd and gy_gtk_ywindow.

   SIDE EFFECTS:
    #include "gy_gtk.i" has the following side effect:
     - the users locale is set, excluding LC_NUMERIC, using
       gy_setlocale.
     - mouse() is replaced by gy_gtk_mouse().
   
    Whenever a GUI is running:
     - gy_gtk_idler() runs repeatedly using adter(). Some functions
       (such as mouse()) cannot run as long as this loop is running.
     - after_error is set to a function which redirects errors to a
       dialog and makes some clean-up.
    See gy_gtk_main().

   EXAMPLE:
    The gy source code comes with a few example scripts. Here comes a
    basic, commented helloworld:
    
    // Create widget hierarchy
    win = Gtk.Window();
    button = Gtk.Button(label="Hello World!");
    noop, win.add(button);

    // Write a callback
    func hello(widget, event, data) {
      "Hello World!";
    }

    // Connect callback to button event
    gy_signal_connect, button, "clicked", hello;

    // Show window
    gy_gtk_main, win, on_delete=1;
    
   SEE ALSO: gy_i, gyterm, gycmap, gywindow, gy_gtk_main
             and the C documentation for the Gtk+ 3 API.
 */

extern Gtk, Gdk, GLib, GdkPixbuf;
/* DOCUMENT Gtk, Gdk, GdkPixbuf, GLib
     gy namespaces.
   SEE ALSO: gy_gtk_i
 */
Gtk = gy.require("Gtk", "3.0");
GdkX11 = gy.require("GdkX11", "3.0");
Gdk = gy.Gdk;
GLib = gy.GLib;
GdkPixbuf = gy.GdkPixbuf;

func __gyterm_init
{
  require,  "string.i";
  extern __gyterm_initialized, __gyterm_win;
  __gyterm_win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, __gyterm_win.set_title("Yorick command line");
  entry = gy_gtk_ycmd(1);
  noop, __gyterm_win.add(entry);
  noop, entry.set_width_chars(80);
  __gyterm_initialized=1;
}  

if (is_void(__gyterm_history_size)) __gyterm_history_size=100;
__gyterm_history=array(string, __gyterm_history_size);
__gyterm_idx=__gyterm_cur=1;
__gyterm_max=0;

func __gyterm_key_pressed(widget, event, udata) {
  extern __gyterm_history, __gyterm_cur, __gyterm_max, __gyterm_idx;

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
    cmd=widget.get_text();
    noop, widget.set_text("");
    __gyterm_history(__gyterm_cur)=cmd;
    __gyterm_cur=(__gyterm_cur<numberof(__gyterm_history))?__gyterm_cur+1:1;
    __gyterm_max=min(numberof(__gyterm_history), __gyterm_max+1);
    __gyterm_idx=__gyterm_cur;
    write, format="> %s\n", cmd;
    include, strchar(cmd), 1;
    return 1;
  }

  return 0;
}

func gy_gtk_ycmd(noexpander)
/* DOCUMENT widget = gy_gtk_ycmd (noexpander)
   
    Create a Gtk widget to type Yorick commands, similar to
    gyterm. Unless NOEXPANDER is specified and evaluates to true, the
    Gtk entry is put in an expander.

   SEE ALSO: gy_i, gyentry, gy_gtk_ycmd_connect, gy_gtk_ywindow
 */
{
  entry = Gtk.Entry.new();
  noop, entry.set_vexpand(0);
  gy_gtk_ycmd_connect, entry;
  if (noexpander) return entry;
  exp = Gtk.Expander.new("<small><span style=\"italic\" size=\"smaller\">Yorick command</span></small>");
  noop, exp.add(entry);
  noop, exp.set_use_markup(1);
  noop, exp.set_resize_toplevel(1);
  noop, exp.set_vexpand(0);
  return exp;
}

func gy_gtk_ycmd_connect(widget) {
/* DOCUMENT gy_gtk_ycmd_connect, entry_widget
   
    Makes entry_widget mimick gyterm behavior.

   EXAMPLE
    entry=Gtk.Entry.new();
    gy_gtk_ycmd_connect, entry;

   SEE ALSO: gy_i, gyterm, gy_gtk_main
 */
  gy_signal_connect, widget, "key-press-event", __gyterm_key_pressed;
  noop, widget.set_placeholder_text("Yorick command");
  noop, widget.set_tooltip_text("Yorick command");
}

func gy_gtk_hide_on_delete(wdg, evt, udata)
/* DOCUMENT gy_gtk_destroy_on_delete, window, [evt, data]

     If evt is void, set-up gy_gtk_hide_on_delete as a callback for
     the "delete-event" Gtk Window event.

     If evt is not void, act as a callback: hide the window. In
     addition, stops the event loop if there are no visible windows
     remaining.
     
   SEE ALSO: gy_gtk_destroy_on_delete, gy_gtk_idler_maybe_stop,
             gy_gtk_main, gy_gtk_i
 */
{
  if (is_void(evt))
      gy_signal_connect, wdg, "delete-event", gy_gtk_hide_on_delete;
  noop, wdg.hide();
  gy_gtk_idler_maybe_stop;
  return 1;
}

func gy_gtk_suspend (wdg, evt, udata)
/* DOCUMENT deprecated: gy_gtk_suspend, wdg
     This was used when gy_gtk_main used to call gy.Gtk.main().
   SEE ALSO: gy_gtk_hide_on_delete, gy_gtk_destroy_on_delete
 */
{
  extern __gy_gtk_suspend_warned;
  if (!__gy_gtk_suspend_warned) {
    __gy_gtk_suspend_warned=1;
    write, "WARNING: gy_gtk_suspend is deprecated, please upgrade to the new API\n";
  }
  if (is_void(evt)) evt=1;
  return gy_gtk_hide_on_delete(wdg, evt, udata);
}


func gy_gtk_destroy_on_delete(wdg, evt, udata)
/* DOCUMENT gy_gtk_destroy_on_delete, window, [evt, data]

     If evt is void, set-up gy_gtk_destroy_on_delete as a callback for
     the "delete-event" Gtk Window event.

     If evt is not void, act as a callback: destroy the window. In
     addition, stops the event loop if there are no visible windows
     remaining.
     
   SEE ALSO: gy_gtk_hide_on_delete, gy_gtk_idler_maybe_stop,
             gy_gtk_main, gy_gtk_i
 */
{
  if (is_void(evt))
      gy_signal_connect, wdg, "delete-event", gy_gtk_destroy_on_delete;
  noop, wdg.destroy();
  gy_gtk_idler_maybe_stop;
  return 1;
}

func gy_gtk_idler_maybe_stop
/* DOCUMENT gy_gtk_idler_maybe_stop
     Stop the event handler if there remains no window visible.
     This should always be called in your delete-event handler.
   SEE ALSO: gy_gtk_main, gy_gtk_hide_on_delete, gy_gtk_destroy_on_delete
 */
{
  extern after_error;

  gy_gtk_idler; // perform pending actions such as hiding a window!

  tlvs = Gtk.Window.list_toplevels();
  res = 0;
  if (tlvs.size) {
    for (;!is_void(tlvs);tlvs=tlvs.next) res |= tlvs.data.visible();
  }
  if (!res) {
    after_error=[];
    gy_gtk_idler, 0;
  }
}


func gy_gtk_main(win, on_delete=)
/* DOCUMENT gy_gtk_main, win, on_delete=CALLBACK;

    Show Gtk.Window win and set-up gy_gtk.i event loop:
    
    1. Set-up after_error handler,
    2. Optionally connect delete-event callback,
    3. Show win;
    4. Launch event handler loop.

    It is (probably) best to stop the event loop when no windows are
    visible. To achieve this, the delete-event callback of any window
    (and any other callback which hides or detroys a window) should
    call gy_gtk_idler_maybe_stop.

   KEYWORDS:
    on_delete:
      the name (string) of a Yorick function suitable as a
      delete-event handler:
        func on_delete(widget, event, data) {
          // hide or destroy
          gy_gtk_idler_maybe_stop;
          return 1;
        }
      on_delete=1 is a short-hand for on_delete="gy_gtk_hide_on_delete";
      on_delete=2 is a short-hand for on_delete="gy_gtk_destroy_on_delete";

    
   SEE ALSO: gy_gtk_i, gy_gtk_hide_on_delete, gy_gtk_destroy_on_delete
 */
{
  extern after_error;
  after_error=__gy_gtk_on_error;
  if (!is_void(win)) {
    if (is_numerical(on_delete)) {
      if      (on_delete==0) on_delete=[];
      else if (on_delete==1) on_delete="gy_gtk_hide_on_delete";
      else if (on_delete==2) on_delete="gy_gtk_destroy_on_delete";
    }
    if (!is_void(on_delete)) gy_signal_connect, win, "delete-event", on_delete;
    noop, win.show_all();
  }
  gy_gtk_idler,1;
}

func gyterm(cmd)
/* DOCUMENT gyterm
     Open a window containing a single line in which arbitrary Yorick
     commands can be typed. If you want to embed gyterm in another GUI, see
     gy_gtk_ycmd and gy_gtk_ycmd_connect.
   SEE ALSO: gy_i, gy_gtk_ycmd, gy_gtk_ycmd_connect, gycmap, gywindow
 */
{
  extern __gyterm_initialized, __gyterm_win;
  local on_delete;
  if (!__gyterm_initialized) {
    __gyterm_init;
    on_delete=1;
  }
  gy_gtk_main, __gyterm_win, on_delete=on_delete;
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
  if (__gycmap_old_yorick)
    noop, combo.set_sensitive(0);
  else
    gy_signal_connect, combo, "changed", __gycmap_combo_changed;
  
  gy_signal_connect, __gycmap_ebox, "button-press-event", __gycmap_callback;
  noop, __gycmap_win.set_title("Yorick color table chooser");

  noop, __gycmap_builder.get_object("box1").add(gy_gtk_ycmd());

  //  gy_gtk_window_hide_on_delete, __gycmap_win;
  
  __gycmap_initialized=1;

}

func __gycmap_gistct(name) {
  palette, name+".gp";
}

if (!is_func(gistct)) {
  __gycmap_old_yorick=1;
  gistct=__gycmap_gistct;
 }

func __gycmap_callback(widget, event, udata) {
  extern __gycmap_cur_names;
  ev = Gdk.EventButton(event);
  ev, x, x, y, y;
  name= __gycmap_cur_names(long(y/19)+1);
  if (is_void(__gycmap.callback))
    __gycmap_cmd, __gycmap_cur_names(long(y/19)+1);
  else
    noop, __gycmap.callback(__gycmap_cmd, name);
}

func __gycmap_combo_changed(widget, event, udata) {
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

__gycmap=save();

func gycmap(callback)
/* DOCUMENT gycmap
         or gycmap, callback
   
    A graphical wrapper around the cmap family of functions, gycmap
    allows the user to interactively select a color table by viewing
    sample colorbars and clicking on the bar of her choice. If an
    image is displayed in the current Yorick graphic window, the new
    colormap is applied immediately. cmap_test can be used for
    displaying a test image.

    If CALLBACK is specified, it is called as:
      callback, fonction, colormap;
    each time a new colormap is selected instead of simply setting the
    color map.

    gycmap requires a recent version of Yorick (from git, as of
    2013-04).

   SEE ALSO: cmap, cmap_test
 */
{
  extern __gycmap_initialized, __gycmap_builder, __gycmap_win, __gycmap_ebox,
    __gycmap;
  local on_delete;
  save, __gycmap, callback;
  if (!__gycmap_initialized) {
    __gycmap_init;
    on_delete=1;
  }
  gy_gtk_main, __gycmap_win, on_delete=on_delete;
}

//// gywindow: a yorick window wrapper

func __gywindow_on_error
{
  extern __gywindow_device;
  __gywindow_ungrab;
}

func gy_gtk_allowgrab(allow)
/* DOCUMENT gy_gtk_allowgrab, mode
     mode = 0: forbid grabbing the mouse.
     mose = 1: allow grabbing.
   SEE ALSO: gywindow
 */
{
  extern __gy_gtk_allowgrab;
  __gy_gtk_allowgrab=allow;
  if (!allow) __gywindow_ungrab;
}
gy_gtk_allowgrab, 1;

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

func gy_gtk_ywindow_reinit(yid, dpi=, style=)
/* DOCUMENT gy_gtk_ywindow_reinit, yid
   
     RE-initialize Yorick window YID attached to a gy widget, for
     instance to change DPI or STYLE.

   KEYWORDS: dpi, style.
   SEE ALSO: gy_i, gywindow, gy_gtk_ywindow
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
  noop, cur.da.set_size_request(long(8.5*dpi),long(11*dpi));
}

func __gywindow_event_handler(widget, event, udata) {
  extern __gywindow, __gywindow_xs0, __gywindow_ys0, __gywindow_device;
  local curwin, win;
  
  cur = __gywindow_find_by_xid(gy_gtk_xid(widget));
  if (is_void(cur)) return;

  EventType=Gdk.EventType;

  ev = Gdk.EventAny(event);
  type = ev.type;
  
  if (type == EventType.map && !cur.realized) {
    window, cur.yid, parent=gy_gtk_xid(widget), ypos=-24, dpi=cur.dpi,
      width=long(8.5*cur.dpi), height=long(11*cur.dpi), style=cur.style;
    save, cur, realized=1;
    sw = widget.get_parent().get_parent();
    hadjustment = sw.get_hadjustment();
    vadjustment = sw.get_vadjustment();
    save, cur, sw, hadjustment, vadjustment;
    xcenter = long(4.25*cur.dpi);
    noop, cur.hadjustment.set_value(xcenter-cur.hadjustment.get_page_size()/2);
    noop, cur.vadjustment.set_value(xcenter-cur.vadjustment.get_page_size()/2);
    if (cur.on_realize) cur.on_realize;
    return;
  }

  if (!cur.realized) return;

  if (type == EventType.configure) {
    xcenter = long(4.25*cur.dpi);
    noop, cur.sw.set_size_request(-1,-1);
    noop, cur.hadjustment.set_value(xcenter-cur.hadjustment.get_page_size()/2);
    noop, cur.vadjustment.set_value(xcenter-cur.vadjustment.get_page_size()/2);
    if (cur.on_configure) cur.on_configure;
    return;
  }

  if (!cur.grab) return;

  pix2ndc=72.27/cur.dpi*0.0013;
  
  if (type == EventType.enter_notify && __gy_gtk_allowgrab) {
    __gywindow_device = Gdk.Device(Gtk.get_current_event_device());
    noop, Gtk.Widget(widget)(window, win);
    noop, __gywindow_device.grab(win, Gdk.GrabOwnership.none, 1,
                                 Gdk.EventMask.all_events_mask,
                                 ,
                                 Gdk.CURRENT_TIME);
    return;
  }

  if (type == EventType.leave_notify && Gdk.EventCrossing(ev).mode != Gdk.CrossingMode.grab) {
    __gywindow_ungrab;
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
    __gywindow_xs0=xs;
    __gywindow_ys0=ys;
  }

  if (type == EventType.button_release) {

    lm = limits();
    flags=long(lm(5));

    if (is_func(cur.mouse_handler)) {
      noop, cur.mouse_handler(cur.yid,
                              __gywindow_xs0, __gywindow_ys0,
                              xs, ys, button, flags);
      if (!is_void(curwin) && curwin>=0) window, curwin;
      return;
    }

    lm2=lm;
    fact=1.;
    if (button==1) fact=2./3.;
    else if (button==3) fact=1.5;

    xlog = flags & 128;
    ylog = flags & 256;

    if (noneof(__gywindow_xs0 == lm(1:2))) {
      if (xlog) {
        lm2(1:2)=__gywindow_xs0 / (xs/lm(1:2))^fact;
      } else {
        lm2(1:2)=__gywindow_xs0 - (xs-lm(1:2))*fact;
      }
      limits, lm2(1), lm2(2);
    }

    if (noneof(__gywindow_ys0 == lm(3:4))) {
      if (ylog) {
        lm2(3:4)=__gywindow_ys0 / (ys/lm(3:4))^fact;
      } else {
        lm2(3:4)=__gywindow_ys0 - (ys-lm(3:4))*fact;
      }
      range, lm2(3), lm2(4);
    }
    
  }

  if (!is_void(curwin) && curwin>=0) window, curwin;
  
  if (type == EventType.motion_notify) {
    if (x<cur.hadjustment.get_value() ||
        y<cur.vadjustment.get_value() ||
        x>cur.hadjustment.get_value()+cur.hadjustment.get_page_size() ||
        y>cur.vadjustment.get_value()+cur.vadjustment.get_page_size()) {
      __gywindow_ungrab;
      return;
    }
    noop, cur.slabel.set_text("?");
    noop, cur.xlabel.set_text(swrite(xs));
    noop, cur.ylabel.set_text(swrite(ys));
    return;
  }
  
  //write, format="in __gywindow_realized. event: %d\n", type;
}

/// ywindows
if (is_void(__gywindow)) __gywindow=save();

func gy_gtk_ywindow_connect(&yid, win, da, slabel, xlabel, ylabel, dpi=, style=,
                            on_realize=, on_configure=, grab=)
/* DOCUMENT gy_gtk_ywindow_connect, yid, win, da, slabel, xlabel, ylabel
   
    Connect widgets to embed a Yorick window in a Gtk DrawingArea (see
    gywindow for a trivial example). For a lower level function, see
    gy_gtk_ywindow.

     If YID is nil, a new ID is taken and YID is set to this value.

   ARGUMENTS
    yid: Yorick window ID to embed
    win: the toplevel Gtk.Window widget
    da:  the Gtk.DrawingArea in which the yorick window will be embedded.
    s|x|ylabel: Gtk.Label widgets in which to report mouse motion.

   KEYWORDS
    on_realize: function to call when the window is realized
    on_configure: function to call whenever the function is configured
          (i.e. resized)
    grab: 1 if window should grab the mouse, necessary to use
          gy_gtk_ywindow_mouse_handler, otherwise discouraged
    
   SEE ALSO: gy_i, gywindow, gy_gtk_ywindow, gy_gtk_ywindow_mouse_handler
 */
{
  extern __gywindow;
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id(-1);
  if (is_void(yid)) error, "unable to find free id";
  
  if (is_void(dpi)) dpi=75;
  gy_signal_connect, da, "event", __gywindow_event_handler;
  save, __gywindow, "", save(yid, xid=[], win, da, slabel, xlabel, ylabel,
                             realized=0, dpi, style,
                             mouse_handler=[],
                             on_realize, on_configure, grab);
}


func gy_gtk_ywindow_mouse_handler(yid, handler)
/* DOCUMENT gy_gtk_ywindow_mouse_handler, yid, handler

     Attach application-specific mouse event handler Yorick window
     created with gy_gtk_ywindow (this includes gywindow windows).

     The handler will be passed only press-release events and allows
     similar actions to what mouse() provides for regular Yorick
     windows.

   ARGUMENTS:
     yid: the Yorick window ID number, first argument of
          gy_gtk_window.
     handler: Yorick function with prototype
                func mouse_toto(yid, x0, y0, x1, y1, button, flags);
              where x0, y0, x1 and y1 are the window coordinates of
              the button press and release events, button is the
              button which was pressed, flags is limits()(5).
      
   SEE ALSO: gy_i, gy_gtk_ywindow, gywindow, mouse, limits
   
 */
{
  save, __gywindow_find_by_yid(yid), mouse_handler=handler, grab=1;
}

func gy_gtk_ywindow(&yid, dpi=, width=, height=, style=,
                    on_realize=, on_configure=, grab=)
/* DOCUMENT widget = gy_gtk_ywindow(yid)

     Initialize a Gtk widget embedding Yorick window number YID. The
     widget is scrollable and provides mouse position reading and
     zoom/pan capabilities. The widget is connected using
     gy_gtk_ywindow_connect.

     If YID is nil, a new ID is taken and YID is set to this value.

   KEYWORDS: dpi, width, height, style: see window
             on_realize, on_configure, grab: see gy_gtk_window_connect
   SEE ALSO: gy_i, gywindow, window, gy_gtk_ywindow_connect
 */
{
  extern __gywindow;
  
  if (is_void(dpi)) dpi=75;
  //if (is_void(width)) width=long(6*dpi);
  //if (is_void(height)) height=long(6*dpi);
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id(-1);
  if (is_void(yid)) error, "unable to find free id";
  
  box = Gtk.Box.new(Gtk.Orientation.vertical, 0);

  box2=Gtk.Box.new(Gtk.Orientation.horizontal, 0);
  noop, box.pack_start(box2, 0,0,0);

  noop, box2.pack_start(Gtk.Label(label="System: "),0,0,0);

  slabel=Gtk.Label(label="0", max_width_chars=1, width_chars=1);
  noop, box2.pack_start(slabel, 0, 0, 0);

  noop, box2.pack_start(Gtk.Label(label="( "),0,0,0);

  xlabel=Gtk.Label(label="", max_width_chars=15, width_chars=15);
  noop, box2.pack_start(xlabel, 0, 0, 0);

  noop, box2.pack_start(Gtk.Label(label=", "),0,0,0);

  ylabel=Gtk.Label(label="", max_width_chars=15, width_chars=15);
  noop, box2.pack_start(ylabel, 0, 0, 0);

  noop, box2.pack_start(Gtk.Label(label=")"),0,0,0);

  sw = Gtk.ScrolledWindow.new(,);
  if (!is_void(width) && !is_void(height))
    noop, sw.set_size_request(width,height); 
  noop, box.pack_start(sw, 1, 1, 0);

  tmp = Gtk.Viewport.new(,);
  noop, sw.add(tmp);
  
  da = Gtk.DrawingArea.new();
  noop, da.add_events(Gdk.EventMask.all_events_mask);
  noop, da.set_size_request(long(8.5*dpi),long(11*dpi));
  noop, tmp.add(da);

  gy_gtk_ywindow_connect, yid, win, da, slabel, xlabel, ylabel, dpi=dpi, style=style,
    on_realize=on_realize, on_configure=on_configure, grab=grab;

  return box;
}

func __gywindow_cmap(wdg, data)
{
  gycmap;
  return 1;
}

func __gywindow_save(wdg, data)
{
  require, "pathfun.i";
  extern result;
  
  win = Gtk.FileChooserDialog();
  fcfc = Gtk.FileChooser(win);
  noop, fcfc.set_action(Gtk.FileChooserAction.save);
  noop, fcfc.set_do_overwrite_confirmation(1);
  noop, fcfc.set_create_folders(1);
  noop, win.add_button(Gtk.STOCK_SAVE, Gtk.ResponseType.ok);
  noop, win.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.cancel);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*.[pP][dD][fF]");
  noop, filter.add_pattern("*.[eE][pP][sS]");
  noop, filter.add_pattern("*.[jJ][pP][eE][gG]");
  noop, filter.add_pattern("*.[jJ][pP][gG]");
  noop, filter.add_pattern("*.[jJ][fF][iI][fF]");
  noop, filter.add_pattern("*.[pP][nN][gG]");
  noop, filter.set_name("All supported files");
  noop, fcfc.add_filter(filter);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*.[pP][dD][fF]");
  noop, filter.set_name("PDF documents");
  noop, fcfc.add_filter(filter);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*.[eE][pP][sS]");
  noop, filter.set_name("EPS documents");
  noop, fcfc.add_filter(filter);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*.[jJ][pP][eE][gG]");
  noop, filter.add_pattern("*.[jJ][pP][gG]");
  noop, filter.add_pattern("*.[jJ][fF][iI][fF]");
  noop, filter.set_name("JPEG images");
  noop, fcfc.add_filter(filter);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*.[pP][nN][gG]");
  noop, filter.set_name("PNG images");
  noop, fcfc.add_filter(filter);

  filter = Gtk.FileFilter();
  noop, filter.add_pattern("*");
  noop, filter.set_name("All files");
  noop, fcfc.add_filter(filter);
  
  noop, win.show_all();
  
  result = win.run();
  
  fname=fcfc.get_filename();
  noop, win.hide();
  noop, win.destroy();
  
  if (result != Gtk.ResponseType.ok || fname == string(0) ) return 1;

  yid = __gywindow_find_by_xid(gy_gtk_xid(data)).yid;
  
  bname=basename(fname);
  ext=pathsplit(bname,delim=".");
  if (numberof(ext)==1) {
    gyerror, "Export failed: no extension in file name.";
    return 1;
  }
  format=ext(0);
  
  if (anyof(format==["JPEG","jpeg","jpg","jfif"])) fformat="jpeg";
  else if (anyof(format==["PNG","png"])) fformat="png";
  else if (anyof(format==["EPS","eps"])) fformat="eps";
  else if (anyof(format==["PDF","pdf"])) fformat="pdf";
  else {
    gyerror, "Could not recognize extension \"" + format +"\".";
    return 1;
  }
  symb=symbol_def(fformat);
  if (!is_func(symb)) {
    gyerror, "This Yorick lacks the function \"" + fformat + "\".";
    return 1;
  }
  if (catch(-1)) {gyerror, catch_message; return 1;}


  prev = current_window();
  window, yid;
  symb, fname;
  window, prev;
  
  return 1;
}

func __gywindow_init(&yid, dpi=, width=, height=, style=,
                     on_realize=, on_configure=, grab=)
{
  extern __gywindow, adj;
  if (is_void(yid)) yid=gy_gtk_ywindow_free_id(-1);
  if (is_void(yid)) error, "unable to find free id";
  win = Gtk.Window.new(Gtk.WindowType.toplevel);
  noop, win.set_default_size(450, 488);
  noop, win.set_title("Yorick "+pr1(yid));
  box=Gtk.Box.new(Gtk.Orientation.vertical, 0);
  noop, win.add(box);

  noop, box.pack_start(gy_gtk_ywindow
                       (yid,
                        dpi=dpi, width=width, height=height, style=style,
                        on_realize=on_realize, on_configure=on_configure, grab=grab),
                       1, 1, 0);

  cur = __gywindow_find_by_yid(yid);

  tb = Gtk.Toolbar.new();
  tbox=Gtk.Box.new(Gtk.Orientation.vertical, 0);
  
  exp = Gtk.Expander.new("<small><span style=\"italic\" size=\"smaller\">Tools</span></small>");
  noop, exp.add(tbox);
  noop, exp.set_use_markup(1);
  noop, exp.set_resize_toplevel(1);
  noop, box.pack_start(exp, 0,0,0);

  noop, tbox.pack_start(gy_gtk_ycmd(1), 0, 0, 0);
  noop, tbox.pack_start(tb, 0, 0, 0);

  cb = Gtk.ComboBoxText.new();
  noop, cb.insert(-1, "0.5625", "9:16");
  noop, cb.insert(-1, "0.6180339887498947915", "2:(1+√5)");
  noop, cb.insert(-1, "0.625", "10:16");
  noop, cb.insert(-1, "0.75", "3:4");
  noop, cb.insert(-1, "1", "1:1");
  noop, cb.insert(-1, "1.33333333333333333", "4:3");
  noop, cb.insert(-1, "1.6", "16:10");
  noop, cb.insert(-1, "1.6180339887498949025", "(1+√5):2");
  noop, cb.insert(-1, "1.77777777777777777", "16:9");
  noop, cb.get_child().set_size_request(2, -1);
  noop, cb.set_tooltip_text("Aspect ratio");
  gy_signal_connect, cb, "changed", __gywindow_ratio, cur.da;
  item = Gtk.ToolItem.new();
  noop, item.add(cb);
  noop, tb.insert(item, -1);
  
  cb = Gtk.ComboBoxText.new();
  noop, cb.insert(-1, "50", "50 dpi");
  noop, cb.insert(-1, "75", "75 dpi");
  noop, cb.insert(-1, "100", "100 dpi");
  noop, cb.insert(-1, "150", "150 dpi");
  noop, cb.get_child().set_size_request(2, -1);
  noop, cb.set_tooltip_text("Resolution (dots per inch)");
  gy_signal_connect, cb, "changed", __gywindow_dpi, cur.da;
  item = Gtk.ToolItem.new();
  noop, item.add(cb);
  noop, tb.insert(item, -1);
  
  cb = Gtk.ComboBoxText.new();
  path = pathsplit(GISTPATH);
  for (i=1; i<=numberof(path); ++i) {
    dir=path(i);
    files=lsdir(dir);
    if (is_string(files)) {
      ind=where(strglob( "*.gs", files));
      if (!numberof(ind)) continue;
      files=files(ind);
      for (j=1; j<=numberof(files); ++j) {
        noop, cb.insert(-1, files(j), strpart(files(j), :-3));
      }
    }
  }
  noop, cb.set_tooltip_text("Plot style");
  gy_signal_connect, cb, "changed", __gywindow_style, cur.da;
  item = Gtk.ToolItem.new();
  noop, item.add(cb);
  noop, tb.insert(item, -1);
  
  but = Gtk.ToolButton.new_from_stock(Gtk.STOCK_SAVE);
  noop, but.set_tooltip_text("Export plot");
  noop, tb.insert(but, -1);
  gy_signal_connect, but, "clicked", __gywindow_save, cur.da;
  
  but = Gtk.ToolButton.new_from_stock(Gtk.STOCK_SELECT_COLOR);
  noop, but.set_tooltip_text("Choose color map");
  noop, tb.insert(but, -1);
  gy_signal_connect, but, "clicked", __gywindow_cmap, cur.da;
}

func __gywindow_ratio(widget, data)
{

  yid = __gywindow_find_by_xid(gy_gtk_xid(data)).yid;
  
  ratio = 0.;
  sread, widget.get_active_id(), ratio;
  inch2ndc=72.27*0.0013;

  
  prev = current_window();
  window, yid;

  get_style, land, sys, leg, cleg;

  ovp = sys(plsys()).viewport;
  ow = ovp(2)-ovp(1);
  oh = ovp(4)-ovp(3);
  ol = max(ow, oh);
  xc = 0.5*(ovp(2)+ovp(1));
  yc = 0.5*(ovp(4)+ovp(3));

  if (ratio >= 1) {
    w = ol; h = ol / ratio;
  } else {
    h = ol; w = ol*ratio;
  }
  xc;
  yc;
  sys(plsys()).viewport = [xc,xc,yc,yc] + [-w,w,-h,h] * 0.5;
  set_style, land, sys, leg, cleg;

  window, prev;
}

func __gywindow_dpi(widget, data)
{

  cur = __gywindow_find_by_xid(gy_gtk_xid(data));
  
  dpi = 0;
  sread, widget.get_active_id(), dpi;
  
  prev = current_window();

  gy_gtk_ywindow_reinit, cur.yid, dpi=dpi, style=cur.style;
  
  window, prev;
}

func __gywindow_style(widget, data)
{

  cur = __gywindow_find_by_xid(gy_gtk_xid(data));
  
  prev = current_window();

  gy_gtk_ywindow_reinit, cur.yid, dpi=cur.dpi, style=widget.get_active_id();
  
  window, prev;
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
      cxid=gy_gtk_xid(cur.da);
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

func gy_gtk_ywindow_free_id(start)
/* DOCUMENT yid = gy_gtk_ywindow_free_id( start );
   
     Find a free Yorick window ID.

     A window ID is considered free if the window is not realized
     (which is checked using window_geometry()) and it has not yet
     been registered as a gywindow.

     The 64 possible window IDs (0 to 63) are probed starting from
     START. If START<0, window IDs are probed in desceding order
     starting from 64-START. For instance, if START==42, the IDs will
     be probed in this order: 42, 43, ..., 62, 63, 0, 1, ..., 40, 41.

     If no free ID can be found, gy_gtk_ywindow_free_id() returns [].
          
   SEE ALSO: gywindow, window, window_geometry
*/
{
  if (is_void(start)) start=0;
  if (start >= 0) dir = 1;
  else { dir = -1; start = 64+start; }
  n=start;

  // window_geometry returns [] if the window is not realized, but the
  // documentation says it return all 0. Let's check both conditions.
  // A window ID may also have alreay been registered by gy_gtk but
  // not be realized yet.
  while ((!is_void((a=window_geometry(n))) &&
          !allof(a==0)) ||
         !is_void(__gywindow_find_by_yid(n))) {
    n += dir;
    if (n==64) n=0;
    else if (n==-1) n=63;
    if (n==start) return [];
  }
  
  return n;
}

func gywindow(&yid, freeid=, dpi=, width=, height=, style=,
              on_realize=, on_configure=, grab=)
/* DOCUMENT gywindow, yid

    When the Gtk main loop is running, the Yorick main loop is
    stuck. This prevents things like window zooming to
    happen. gywindow embeds Yorick window YID in a Gtk window and
    emulates zooming etc. If YID already exists, it is first killed
    and its content erased.

    If you are interested in embedding a Yorick window in your own
    application, have a look at gy_gtk_ywindow.

   KEYWORDS:
    freeid: if true, a new ID is found using gy_gtk_ywindow_free_id.
    dpi, width, height, style: see window
    on_realize, on_configure, grab: see gy_gtk_window_connect

   SEE ALSO: gyterm, gy_gtk_ywindow, gy_gtk_ywindow_free_id, window,
             gywinkill
*/
{
  local on_delete;
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
        dpi=dpi, width=width, height=height, style=style,
        on_realize=on_realize, on_configure=on_configure, grab=grab;
      on_delete=1;
    }
  gy_gtk_main, __gywindow_find_by_yid(yid).win, on_delete=on_delete;
}

//// error message

func gyerror(msg, buttons=)
/* DOCUMENT gyerror, msg
     Display error message in a Gtk window.
     
     gy_gtk.i sets after_error so that all error messages are
     redirected to gyerror: it is usually not necessary to call
     gyerror directly.

   SEE ALSO: gy_gtk_i
 */
{
  aspect = 1.61803398875; // nombre d'or
  if (is_void(buttons)) buttons = Gtk.ButtonsType.CLOSE;
  dmsg = Gtk.MessageDialog("message-type", Gtk.MessageType.error,
                           buttons=buttons,
                           text=msg);
  noop, dmsg.set_title("Yorick error");
  noop, dmsg.set_size_request(long(200*aspect), 200);
  ans = dmsg.run();
  noop, dmsg.destroy();
  gy_gtk_idler_flush;
  return ans;
}

//// mouse wrapper

if (is_void(__gy_gtk_builtin_mouse)) __gy_gtk_builtin_mouse=mouse;
func gy_gtk_mouse(system, style, prompt)
/* DOCUMENT result= gy_gtk_mouse(system, style, prompt)
   
     Suspends the gy_gtk_idler loop, calls mouse(), and resets the
     gy_gtk_idler loop.

     mouse() does not play well with after(), which gy_gtk.i uses
     extensively.  This function wraps around mouse() to alleviate
     this.
     
     By default, gy_gtk.i installs gy_gtk_mouse as
     mouse, replacing the built-in which remains available as
     __gy_gtk_builtin_mouse.

   SEE ALSO: gy_gtk_idler, __gy_gtk_builtin_mouse, after, gy_gtk_i
 */
{
  gy_gtk_idler, 0;
  res = __gy_gtk_builtin_mouse(system, style, prompt);
  if (gy_gtk_main_running) gy_gtk_idler, 1;
  return res;
}
mouse = gy_gtk_mouse;

//// utilities

func gy_gtk_xid(wdg)
/* DOCUMENT id=gy_gtk_xid(wdg)
   
     Get X11 window ID associated with widget WDG.
     This allows displaying a Yorick window inside a Gtk widget.

   EXAMPLE:
     builder=Gtk.Builder.new(glade_file);
     ywin = builder.get_object(yorick_widget_name);
     func on_ywin_event(void) {
       window, parent=gy_gtk_xid(ywin);
     }
     gy_signal_connect, ywin, "event", on_ywin_event;
     
   SEE ALSO: gy_i, gy_gdk_window
 */

{
  return gy.GdkX11.X11Window(Gtk.Widget(wdg).window).get_xid();
}


extern Y_GLADE, Y_DATA;
/* DOCUMENT Y_GLADE, Y_DATA
    Semi-colon separated list of directories where glade files may be
    located.

    By default, YGLADE is initialized to:
        Y_GLADE="./:"+Y_USER+":"+pathform(_(Y_SITES,Y_SITE)+"glade/");

    Likewise, Y_DATA directories contain additional
    platform-independent files, such as icons.
    
   SEE ALSO: gy_gtk_builder
 */
if (is_void(Y_GLADE))
  Y_GLADE="./:"+Y_USER+":"+pathform(_(Y_SITES,Y_SITE)+"glade/");

if (is_void(Y_DATA))
  Y_DATA="./:"+Y_USER+":"+pathform(_(Y_SITES,Y_SITE)+"data/");

func gy_gtk_builder(fname)
/* DOCUMENT builder = gy_gtk_builder(fname)
   
    Looks for a file named FNAME in the Y_GLADE path and returns a
    Gtk.Builder object with FNAME loaded.

   SEE ALSO: Y_PATH, gy
 */
{
  file = find_in_path(fname,takefirst=1,path=Y_GLADE);
  if (is_void(file)) gyerror("No such file in glade path: " + fname);
  streplace,file,strfind("~",file),get_env("HOME");
  builder = Gtk.Builder();
  noop, builder.add_from_file(file);
  return builder;
}
extern gy_gtk_before_init;
func gy_gtk_init(argv)
/* DOCUMENT gy_gtk_init, argv
   
     Initialize Gtk. Called automatically by #include "gy_gtk.i"
     
   EXTERNAL VARIABLES:
   
     If gy_gtk_before_init is a function, it is called before
     Gtk.init_check(). This allows for instance setting the
     application name:

     func gy_gtk_before_init
     {
      noop, gy.GLib.set_prgname("MyProg");
      noop, gy.GLib.set_application_name("MyProg");
     }
     #include "gy_gtk.i"
     
   SEE ALSO: gy_gtk_i
 */
{
  extern __gy_gtk_initialized;
  Gtk = gy.require("Gtk", "3.0");
  if (__gy_gtk_initialized) return Gtk;
  if (is_func(gy_gtk_before_init)) gy_gtk_before_init;
  ret = gy.Gtk.init_check(numberof(argv),argv);
  gy_setlocale;
  if (!ret) error, "Gtk initialization failed";
  __gy_gtk_initialized=1;
  return Gtk;
}
gy_gtk_init;

extern gy_gtk_idler_period;
/* DOCUMENT gy_gtk_idler_period
    gy_gtk_idler repeatedly reschedules using after for running every
    gy_gtk_idler_period seconds. Default: 0.05. You may set this
    variable at any time to change the frequency of the loop.
   SEE ALSO: gy_gtk_idler
 */
if (!is_numerical(gy_gtk_idler_period)) gy_gtk_idler_period = 0.05;

func gy_gtk_idler (start_stop)
/* DOCUMENT gy_gtk_idler, start_stop
   
    Start or stop the gy Gtk idler, which takes care of processing the
    Gtk events. While the idler is in place, it runs every
    gy_gtk_idler_period seconds using after().

    Errors may break the idling loop (although gy_gtk.i installs an
    error handler which re-enables the loop). If your Gtk interface
    appears to be frozen whereas Yorick itself is not, try
      gy_gtk_idler, 1

   PARAMETER
    start_stop: 1 to start the loop, 0 to stop it.

   EXTERNAL VARIABLES:
    gy_gtk_idler_period

   SEE ALSO: gy_gtk_i, gy_gtk_idler_period, after,
             gy_gtk_idler_flush, gy_gtk_idler_maybe_stop
 */
{
  extern gy_gtk_idler_period, gy_gtk_main_running;
  if (is_void(start_stop)) start_stop=gy_gtk_main_running;

  // always stop idler
  if (gy_gtk_main_running) {
    after, -, gy_gtk_idler;
  }

  // if start_stop==0, that's all
  if (!start_stop)  {
    gy_gtk_main_running=O;
    return;
  }
  
  gy_gtk_main_running=1;
  // process Gtk events
  gy_gtk_idler_flush;

  // read mouse to update non-grabing gywindows
  if (!is_void( (psn=current_mouse()) )  &&
      !is_void( (cur=__gywindow_find_by_yid(psn(0))) )) {
    noop, cur.slabel.set_text(swrite(long(psn(3))));
    noop, cur.xlabel.set_text(swrite(psn(1)));
    noop, cur.ylabel.set_text(swrite(psn(2)));
  }

  // start idler
  after, gy_gtk_idler_period, gy_gtk_idler;

  // let Yorick display its prompt
  maybe_prompt;
}


func __gy_gtk_on_error
{
  extern after_error;
  __gywindow_on_error;
  ans = gyerror(catch_message+"\n\nDo you want to continue?",
                buttons=Gtk.ButtonsType.YES_NO);
  if (ans==Gtk.ResponseType.YES) gy_gtk_idler;
  else{
    gy_gtk_idler, 0;
    after_error=[];
    write, "gy_gtk_main loop stopped on user request";
  }
}

func gy_gtk_idler_flush
/* DOCUMENT gy_gtk_idler_flush
     Process all pending events in the Gtk+ 3 event queue.
   SEE ALSO: gy_gtk_idler
 */
{
    while (Gtk.events_pending ())  noop, Gtk.main_iteration ();
}

func __gywindow_ungrab
{
  extern __gywindow_device;
  if (!is_void(__gywindow_device)) noop, __gywindow_device.ungrab(Gdk.CURRENT_TIME);
  __gywindow_device = [];
}
