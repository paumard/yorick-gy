GObject Introspection plug-in for yorick
----------------------------------------

Yorick is an interactive programming language for scientific computing
that includes scientific visualization functions, and text and binary
I/O functions geared to millions of numbers.

[yorick.github.com]:         http://yorick.github.com
[yorick.sourceforge.net]:    http://yorick.sourceforge.net
[github.com/dhmunro/yorick]: http://github.com/dhmunro/yorick

This plug-in allows using GObject-introspection.
[https://developer.gnome.org/gi/unstable/index.html]: https://developer.gnome.org/gi/unstable/index.html

In turn, this allows writting Gtk graphical user interfaces directly
in Yorick.


Be warned:
----------

As of now, this is in an early development phase. std::dislaimer
appplies.


Build instructions:
-------------------

For building, you need to the library libgirepository and its header
files. This library gives access to the so called GObject
introspection repository. The Makefile uses pkg-config to find the
headers etc., make sure pkg-config can find
gobject-introspection-1.0.pc or edit the Makefile
appropriateley. Then:

At the moment, it is also necessary to have the Gdk X11 library and
headers installed.

yorick -batch make.i
make
make install


Using instructions:
-------------------

You need the typelib files for the library you want to access. For
instance, under Debian GNU/Linux, to build Gtk 3.0 interfaces, you
need the package: gir-1.2-gtk-3.0.
