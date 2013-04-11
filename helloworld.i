#include "gy.i"
Gtk=gy_require("Gtk");
Gtk.init(0,);
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

/*
// From a file:
Gtk=gy_require("Gtk");
Gtk.init(0,);
builder = Gtk.Builder.new()
builder.add_from_file(glade_file_name)
builder.get_object("window1")
win = builder.get_object(main_window_name)
// connect some signals
win.show_all()
Gtk.main()
*/
