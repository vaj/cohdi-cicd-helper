# CoHDI CI/CD Helper for Worker Nodes

## `restrict_kmod.ko`

This kernel module blocks kmod (modprobe, insmod etc) from
inside containers.  To disable this effect set 0 to sysctl
variable `restrict_kmod`.  To remove this module you must
write 0 to `/sys/kernel/livepatch/restrict_kmode/enable`
file.

This module prevents nvidia-modprobe from inside the GPU
operator (and other) container.

## hidegpu

This is a shellscript to hide/unhide a GPU.  It is invoked
at boot time (from systemd hidegpu.service) to hide the
GPU (the initial state).  To emulate hot-plugging the GPU,
invoke `hidegpu hide` as root.  To emulate hot-removing
the GPU, invoke `hidegpu unhide`.

## rk.conf

Should be installed into `/etc/modules-load.d`, to load the
module at boot.

## blacklist-nouveau.conf

Should be installed into `/etc/modprobe.d`, to prevent
autoloading of the opensource driver.

## debian/*

deb package can be made by `debuild -us -uc -b`.  After 
installing the package named `cohdi-cicd-helper_VER_amd64.deb`
the system must be rebooted.
