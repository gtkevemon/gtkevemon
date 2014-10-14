This is the README file for GtkEveMon. It covers:

* Building GtkEveMon
* Running GtkEveMon
* Installing GtkEveMon
* Uninstalling GtkEveMon


BUILDING
========

GtkEveMon comes in form of source code. You can take a look at the
code, it's all located in the "src/" directory. In order to execute
it, it needs to be build. It does not come with a configure script.
So just execute the following command:

    $ make

To build a debug version simple run

    $ make debug

RUNNING
=======

Note that you DO NOT NEED to install GtkEveMon in order to use it.
If you don't want to install GtkEveMon, issue the following commands,
relative to this README file:

    $ cd src
    $ ./gtkevemon

If you're going to install GtkEveMon (read below), just type
"gtkevemon" anywhere in your system or make a nice shortcut in
your window manager yourself.


INSTALLING
==========

Make sure your current working directory is relative to this file.
To install GtkEveMon execute the following command (as super user):

    # make install

The files will be installed to "/usr/local/bin". If you want to change
that, edit the Makefile.

Configuration files as well as data files (SkillTree.xml and
CertificateTree.xml) are placed in "~/.gtkevemon" whether you
install GtkEveMon or not.


UNINSTALLING
============

If you want to uninstall GtkEveMon, just type
following command (as super user):

    # make uninstall

This will not remove your user data in your home directory.


RESSOURCES
==========

Homepage: https://github.com/gtkevemon/gtkevemon  
E-Mail: Simon Fuhrmann <SimonFuhrmann@gmx.de>
