plug_in, "gy";
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

extern gy_init;
/* DOCUMENT gy=gy_init()
    Initialize gy. Normally called only in gy.i.
 */

extern gy_list;
/* DOCUMENT gy_list, OBJECT
   
    List symbols in gy stuff OBJECT.
    
   EXAMPLE:
    gy_list, gy.Gtk
    gy_list, "Gtk"
    gy_list, gy.Gtk.Window
    gy_list, gy.Gtk.Window()
   SEE ALSO: gy
 */

extern gy_signal_connect;
/* DOCUMENT gy_connect_signal, object, signal, handler
         or gy_connect_signal, builder
   
    Connect signal to signal handler.
    
    The handler must accept all the parameters described in the C
    documentation for the signal.

   ARGUMENTS:
    builder: if first argument is a Gtk Builder object, the signals
             information it contains will be used to automatically
             connect GObject signals to Yorick interpreted functions.
             Whether or not the Yorick functions actually exists is
             not checked.
    object:  a gy object supporting signals, e.g. an instance of
             gy.Gtk.Entry.
    signal:  the signal name, e.g. "activated".
    handler: the Yorick command to be executed when the object
             receives the signal. HANDLER will be called like:
             include, ["noop, " + handler + "(par1, ..., parn)"], 1
             where par1 to parn are the parameters described in the C
             documentation for SIGNAL. HANDLER may be a string or a
             Yorick function.
             
   EXAMPLE:
    See gy.

   SEE ALSO: gy
*/

extern gy_debug;
/* DOCUMENT mode = gy_debug();
         or gy_debug, mode;
    Get or set gy debug mode.
   SEE ALSO: gy
*/

extern gy_setlocale;
/* DOCUMENT gy_setlocale, [category,] locale
         or locale=gy_setlocale()

     Get or set locale used in the Yorick process.

     Allways resets LC_NUMERIC to "C" as other values may break
     Yorick.

   SEE ALSO: gy
 */

extern gy_id;
/* DOCUMENT id = gy_id(object)
    
      Get unique id of gy object. Two variables may hold the same
      underlying object: the id is unique.

   SEE ALSO: gy
*/

extern __gy_gtk_builder_connector;
/* DOCUMENT __gy_gtk_builder_connector()
    Return Pointer to C function used by gy_signal_connect when passed
    a Gtk.Buildervobject. Internal use only.
 */

extern gy_object_free;
/* DOCUMENT gy_object_free, obj
    Free object pointed to by gy object OBJ. May be necessary to
    prevent some memory leaks. Experimental: Use with caution.
*/
