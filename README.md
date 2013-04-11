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

Build instructions:
-------------------

yorick -batch make.i
make
make install
