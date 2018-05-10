# xv6-wm

A stacking window manager and API for the xv6 operating system.

## Requirements:
* QEMU
* genisoimage
* GNU gcc and make

## Building:
```
$ make
```

## Running:
Without KVM enabled:
```
$ make qemu
```

With KVM enabled:
```
$ make qemu-kvm
```
