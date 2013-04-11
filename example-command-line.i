#include "gy.i"
#include "string.i"

Gtk=gy_require("Gtk");
Gtk.disable_setlocale();
Gtk.init_check(0,);
win = Gtk.Window.new(Gtk.WindowType.toplevel);

//button = Gtk.Button.new_with_label("Hello World!");
//win.add(button);
//gy_signal_connect, button, "clicked", "\"Hello World!\"";

func idler(void) {
  Gtk.main();
}

func entry_activated(void) {
  noop, Gtk.main_quit();
  set_idler, idler;
  cmd=entry.get_text();
  include, strchar("if (catch(-1)) {return;} "+cmd), 1;
}
entry = Gtk.Entry.new();
gy_signal_connect, entry, "activate", "entry_activated";
noop, win.add(entry);

func winhide(void) {
  noop, win.hide();
  noop, Gtk.main_quit();
}

gy_signal_connect(win, "delete_event", "noop, winhide()");

noop, win.show_all();
noop, Gtk.main();

/*
// From a file: grab some glade file and fix the two variables below!
glade_file_name="../Gyoto/yorick/gyotoy.xml";
main_window_name="window1";
Gtk=gy_require("Gtk");
Gtk.init(0,);
builder = Gtk.Builder.new()
builder.add_from_file(glade_file_name)
win = builder.get_object(main_window_name)
// connect some signals
win.show_all()
Gtk.main()
*/
