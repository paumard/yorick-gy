/*
  This file is the standard "Hello World!" example written using gy.

  Below is also an example using a glade file.
 */

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

Gtk=gy.Gtk;
Gtk.init_check(0,);
gy_setlocale;
win = Gtk.Window.new(Gtk.WindowType.toplevel);

button = Gtk.Button.new_with_label("Hello World!");
win.add(button);

func hello(wdg) {
  "Hello World!";
}

gy_signal_connect, button, "clicked", hello;

func winhide(wdg, evt) {
  noop, wdg.hide();
  noop, gy.Gtk.main_quit();
}

gy_signal_connect(win, "delete-event", winhide);

noop, win.show_all();
noop, Gtk.main();

/*
// From a file: grab some glade file and fix the three variables below!
glade_file_name="../Gyoto/yorick/gyotoy.xml";
main_window_name="window1";
yorick_window_name="yorick_window";
Gtk=gy.Gtk;
Gtk.init_check(0,);
gy_setlocale;
builder = Gtk.Builder.new();
builder.add_from_file(glade_file_name);
win = builder.get_object(main_window_name);
ywin = builder.get_object(yorick_window_name);
// connect some signals
func on_ywin_event(widget, event) {
   noop, Gtk.main_quit();
   set_idler, __gyterm_idler;
   widget;
   gy_xid(widget);
   window, parent=gy_xid(widget);
}
gy_signal_connect, ywin, "event", "on_ywin_event";
gy_signal_connect, win, "delete-event", "__gyterm_suspend";
win.show_all()
Gtk.main()
*/
