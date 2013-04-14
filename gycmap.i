#include "gy.i"

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
 
  
  noop, gy.Gtk.init(0,);
  gy_setlocale;
  __gycmap_gist_img = gy.Gtk.Image.new();
  __gycmap_gist_img.set_from_file(gist_png);
  __gycmap_gist_names=
    ["gray", "yarg", "heat", "earth", "stern", "rainbow", "ncar"];

  __gycmap_msh_img = gy.Gtk.Image.new();
  __gycmap_msh_img.set_from_file(png_dir+"/msh-cmap.png");

  __gycmap_mpl_img = gy.Gtk.Image.new();
  __gycmap_mpl_img.set_from_file(png_dir+"/mpl-cmap.png");
   
  __gycmap_gpl_img = gy.Gtk.Image.new();
  __gycmap_gpl_img.set_from_file(png_dir+"/gpl-cmap.png");

  __gycmap_gmt_img = gy.Gtk.Image.new();
  __gycmap_gmt_img.set_from_file(png_dir+"/gmt-cmap.png");

  __gycmap_div_img = gy.Gtk.Image.new();
  __gycmap_div_img.set_from_file(png_dir+"/cbc-div-cmap.png");

  __gycmap_seq_img = gy.Gtk.Image.new();
  __gycmap_seq_img.set_from_file(png_dir+"/cbc-seq-cmap.png");

  __gycmap_qual_img = gy.Gtk.Image.new();
  __gycmap_qual_img.set_from_file(png_dir+"/cb-qual-cmap.png");
  
  __gycmap_builder = gy.Gtk.Builder.new();
  noop, __gycmap_builder.add_from_file(glade);
  __gycmap_win = __gycmap_builder.get_object("window1");
  __gycmap_ebox = __gycmap_builder.get_object("eventbox");

  __gycmap_ebox.add(__gycmap_gist_img);
  __gycmap_cur_img = __gycmap_gist_img;
  __gycmap_cur_names = __gycmap_gist_names;
  __gycmap_cmd=gistct;
  
  combo=__gycmap_builder.get_object("combobox");
  combo.set_active_id("gist");
  gy_signal_connect, combo, "changed", "__gycmap_combo_changed";
  
  gy_signal_connect, __gycmap_ebox, "button-press-event", "__gycmap_callback";
  gy_signal_connect, __gycmap_win, "delete_event", "__gyterm_suspend";
  __gycmap_initialized=1;
}

func __gycmap_callback(widget, event) {
  extern __gycmap_cur_names;
  ev = gy.Gdk.EventButton(event);
  ev, x, x, y, y;
  __gycmap_cur_names(long(y/19)+1);
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

func gycmap(void) {
  extern __gycmap_initialized, __gycmap_builder, __gycmap_win, __gycmap_ebox;
  if (!__gycmap_initialized) __gycmap_init;
  noop, __gycmap_win.show_all();
  noop, gy.Gtk.main();
}
