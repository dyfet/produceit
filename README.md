What is ProduceIt
=================

This is still in early development and will offer a cross-distro chroot based Linux package build and application testing environment derived somewhat from cape-devtools.  It is also somewhat like what mock offers for RedHat, and pbuilder for debian, though it uses the idea of pre-set dirty chroots containers for efficiently repeating builds with updates, and sets up build instances that can consume their own build dependencies to stage multi-package builds.  The chroots can also be entered as "user" environments for testing software.  ProduceIt also takes advantage of qemu-user-mode emulation to support hosting build and test environments in alternate cpu architectures.

Unlike cape-devtools, this implementation is built purely on c++11, and uses ruby for supporting non-setuid utilities.  This implementation will also offer better chroot container isolation than cape-devtools did, and may in the future also offer a new and efficient container deployment model based on overlayfs and runit for fast container service startup.

Support
=======
I maintain an email address for public contact for Tycho Softworks projects as [tychosoft@gmail.com](mailto://tychosoft@gmail.com).  Merge requests may be accepted when I have time to do so.  I also will be using the gitlab [produceit](https://gitlab.com/tychosoft/produceit) issue tracker for bug reporting and project management.
