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

func gyhelloworld_print(wdg, event, userdata) {
  "Hello World!";
  return 1;
}

func gyhelloworld(glade) {
  Gtk=gy_gtk_init();
  if (glade) {
    main_window_name="window1";
    builder = gy_gtk_builder("gyhelloworld.xml");
    win = builder.get_object(main_window_name);
    gy_signal_connect, builder;
  } else {
    win = Gtk.Window();
    button = Gtk.Button(label="Hello World!");
    noop, win.add(button);
    gy_signal_connect, button, "clicked", gyhelloworld_print;
  }
  gy_gtk_main, win;
}
