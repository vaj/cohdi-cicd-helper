# CoHDI CI/CD Helper for Worker Nodes

## `restrict_kmod.ko`

This kernel module blocks kmod (modprobe, insmod etc) from
inside containers.  To disable this effect set 0 to sysctl
variable `restrict_kmod`.  To remove this module you must
write 0 to `/sys/kernel/livepatch/restrict_kmode/enable`
file.

This module prevents nvidia-modprobe from inside the GPU
operator (and other) container.

In addition, it fakes the vendor ID (sysfs) when it is
accessed from inside containers.

Both behaviours are controlled by the sysctl knob
`kernel.restrict_kmode` (0 or 1).

## hidegpu

This is a shellscript to hide/unhide a GPU.  It is invoked
at boot time (from systemd hidegpu.service) to reset the
state (GPU is hidden).  To emulate hot-plugging the GPU,
invoke `hidegpu unhide` as root.  To emulate hot-removing
the GPU, invoke `hidegpu hide`.

When something bad happens (eg the sysctl variable is
changed outside the script) it returns exit code 1 with
the error message.

* `hidegpu: Module not loaded`  
    `restrict_kmod.ko` is not loaded.
* `hidegpu: Run as root.`  
    The script is invoked without the root privilege.
* `hidegpu: unhidden but something is wrong; exiting`  
    The GPU seems to be hidden but some other state is
unexpected.
* `hidegpu: hidden but something is wrong; exiting`  
    The GPU seems to be unhidden but some other state is
unexpected.
* `hidegpu: GPU is already hidden; exiting...`  
    `hidegpu hide` invoked while the GPU is hidden.
* `hidegpu: GPU is already unhidden; exiting...`  
    `hidegpu unhide` invoked while the GPU is unhidden.
* `hidegpu: GPU is used; exting...`  
    `hidegpu hide` invoked but the GPU is in use (possiblly
by `nvidia-persistenced`).

## rk.conf

Should be installed into `/etc/modules-load.d`, to load the
module at boot.

## blacklist-nouveau.conf

Should be installed into `/etc/modprobe.d`, to prevent
autoloading of the opensource drivers.

## debian/*

deb package can be made by `debuild -us -uc -b`.  After 
installing the package named `cohdi-cicd-helper_VER_amd64.deb`
the system must be rebooted.

## cohdi-cicd-helper.spec

This file is used to build the RPM package.

## How it works

The NVIDIA GPU Operator downloads, builds and loads
(modprobe) the NVIDIA GPU device driver in the container
(nvidia-driver-daemonset).  The `restrict_kmod.ko` kernel
module prevents loading of the device driver.

The device driver might be loaded automatically when
`/dev/nvida*` device node is accessed; module autoloading
can be blocked by the standard blacklist feature of
`modprobe` program.

In addition the Node Feature Discovery (NFD) container
watches the device hot-plug/remove via sysfs.  Faking
NVIDIA vendor ID prevents that the pod starts working.

COHDI removes the GPU device when the GPU is released by
writing 1 to sysfs `remove` file.  We need to re-add the
device by writing 1 to the parent PCI bridge (PCIe port).

`hidegpu` script controls all of the module loading, ID
faking and rescan.
