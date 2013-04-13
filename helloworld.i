#include "gy.i"

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

/*
// From a file: grab some glade file and fix the three variables below!
glade_file_name="../Gyoto/yorick/gyotoy.xml";
main_window_name="window1";
yorick_window_name="yorick_window";
Gtk=gy.Gtk;
Gtk.disable_setlocale();
Gtk.init_check(0,);
builder = Gtk.Builder.new();
builder.add_from_file(glade_file_name);
win = builder.get_object(main_window_name);
ywin = builder.get_object(yorick_window_name);
// connect some signals
func on_ywin_event(void) {
   noop, Gtk.main_quit();
   set_idler, __gyterm_idler;
   window, parent=gy_xid(ywin);
}
gy_signal_connect, ywin, "event", "on_ywin_event";
win.show_all()
Gtk.main()
*/
