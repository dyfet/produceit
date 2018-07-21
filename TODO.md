shellit: optional overwrite passwd, maybe locally parse chroot passwd for user uid/gid
shellit: --root option, same as using sudo shellit but with no overwrites
shellit: --path to specify a directory path to start in chroot
shellit: --script to specify a script to run instead of login shell or cli options
deb-release: add options to run deb-scan... and to sign

add:
bootstrapit to create a shellit chroot using lxc templates as a base, maybe auto-add
	existing user to environment, too.
