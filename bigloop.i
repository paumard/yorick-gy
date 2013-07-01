/*
   This file examplifies how to give oneself the ability to stop a
   long computation loop.
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


#include "gy_gtk.i"

// do stuff should perhaps not in itself last too long (at most a few seconds)
func do_stuff(void) {"do stuff";}

// run do_stuff from a loop and chck for Gtk event regularly
func bigloop_start(widget, event) {
  extern _stop, scene;
  _stop=0;
  while (!_stop) {
    do_stuff;
    while (gy.Gtk.events_pending ())
      noop, gy.Gtk.main_iteration ();
  }
}

// stop loop
func bigloop_stop(widget, event) {
  extern _stop;
  _stop=1;
  "stop";
}

// build window with "start" and "stop" buttons.
Gtk=gy.Gtk;
Gtk.init(0,);
gy_setlocale;
win=Gtk.Window.new(Gtk.WindowType.toplevel);
box=Gtk.Box.new(Gtk.Orientation.horizontal, 0);
noop, win.add(box);
but=Gtk.Button.new_with_label("start");
noop, box.pack_start(but, 0, 0, 0);
gy_signal_connect, but, "clicked", bigloop_start;
but=Gtk.Button.new_with_label("stop");
noop, box.pack_start(but, 0, 0, 0);
gy_signal_connect, but, "clicked", bigloop_stop;

// realize window and gyterm at once.
gy_gtk_main, win, on_delete=1;
gyterm;
