[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

# meta-wssha256kern
A Yocto meta-layer supporting the kernel driver for a memory-mapped sha256 hardware accelerator. 

Created by Brett Nicholas. 

# Dependencies
This layer depends on:
```
  URI: git://git.openembedded.org/bitbake
  branch: master
  
  URI: git://git.openembedded.org/openembedded-core
  layers: meta
  branch: master
```
# Table of Contents
  1. Adding the wssha256kern layer to your build
  2. Testing the kernel driver
  3. Misc
  4. TODO
  5. Notes

# 1. Adding the wssha256kern layer to your build

In order to use this layer, you need to make the Yocto build system aware of
it.

Assuming the wssha256kern layer exists at the top-level of your yocto build tree, you can add it to the build system by adding the location of the wssha256kern layer to bblayers.conf, along with any other layers needed. e.g.:

```
  BBLAYERS ?= " \
    /path/to/yocto/meta \
    /path/to/yocto/meta-poky \
    /path/to/yocto/meta-yocto-bsp \
    /path/to/yocto/meta-xilinx
    /path/to/yocto/meta-xilinx-tools
    /path/to/yocto/meta-wssha256kern \
    "
```
The output kernel driver binary can be found at `${WORKDIR}/wssha256kern.ko` and should be automatically installed in your image's /lib/modules folder. If you can't find it, try the following: 
```
bitbake -e wssha256kern | grep ^WORKDIR //first find the value of ${WORKDIR}
find ${WORKDIR} -name "wssha256kern.ko" 
```
# 2. Testing the Kernel Driver
The wssha256-mod kernel driver (wssha256kern.ko) contains a self-checking test program that can be run in userspace to test the correct operation of the driver. The test program is included in the recipe wssha256test. The test recipe should be automatically built after the wssha256kern layer is added to your build. The output files are located at `${WORKDIR}/wssha256test`, or at `${WORKDIR}/image/usr/bin/wssha256test`. See the `wssha256test_${PV}.bb` file if you are unsure.

If the wssha256test recipe is not added to your build for some reason, you can manually build it using the command `bitbake wssha256test`

# 3. Misc

Currently, the driver has the base address of the peripheral hard-coded, and does not use the built in device tree. It works, however could use much improvement. I'm sure there are many a lurking oops. There is also the possibility of using a linux device driver framework. 

# 4. TODO 
1. Userspace API recipe (for the openSSL engine and test program
2. IOCTL to check if the hardware device is ready (or interrupt support)
3. Integrate linux device tree support and structures (linux/of.h I think..)
4. Create helper functions to abstract away the different ways we might want to use the hardware blocks

# 5. Notes
Below are a series of notes to myself. They probably will mean nothing to you.

### YOCTO BUILD ERRORS AND THEIR RESOLUTIONS
when I first tried building the recipe, I got the following errors:
```
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: wssha256engine: The compile log indicates that host include and/or library paths were used.
         Please check the log '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/temp/log.do_compile' for more information. [compile-host-path]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: No GNU_HASH in the elf binary: '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [ldflags]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: -dev package contains non-symlink .so: wssha256engine-dev path '/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [dev-elf]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA run found fatal errors. Please consider fixing them.
ERROR: wssha256engine-0.1-r0 do_package_qa: Function failed: do_package_qa
ERROR: Logfile of failure stored in: /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/temp/log.do_package_qa.19467
ERROR: Task (/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/meta-wssha256kern/recipes-kernel/wssha256engine/wssha256engine_0.1.bb:do_package_qa) failed with exit code '1'
```
Errors were resolved as follows:

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: wssha256engine: The compile log indicates that host include and/or library paths were used.`
* Changed the -L${libdir} to -L=${libdir} In EXTRA_OECONF_append to make it sysroot relative (https://lists.yoctoproject.org/pipermail/yocto/2012-August/008906.html)

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: No GNU_HASH in the elf binary: '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [ldflags]`
* Added ${LDFLAGS} to the override of makefile LIB variable in EXTRA_OECONF_append

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: -dev package contains non-symlink .so: wssha256engine-dev path '/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [dev-elf]`
* Added `FILES_${PN} += "${libdir}/*.so"` and `FILES_SOLIBSDEV = ""` to recipe to ensure that the .so links weren't only included in ${PN}-dev
