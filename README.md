A Yocto meta-layer supporting the kernel driver for a memory-mapped sha256 hardware accelerator. 

Dependencies
============

This layer depends on:

  URI: git://git.openembedded.org/bitbake
  branch: master

  URI: git://git.openembedded.org/openembedded-core
  layers: meta
  branch: master

TODO: CUSTOM WS SOFTWARE LAYER
  URI: git://git.yoctoproject.org/xxxx
  layers: xxxx
  branch: master


Table of Contents
=================

  I. Adding the wssha256kern layer to your build
 II. Misc


I. Adding the wssha256kern layer to your build
=================================================

--- replace with specific instructions for the wssha256kern layer ---

In order to use this layer, you need to make the build system aware of
it.

Assuming the wssha256kern layer exists at the top-level of your
yocto build tree, you can add it to the build system by adding the
location of the wssha256kern layer to bblayers.conf, along with any
other layers needed. e.g.:

  BBLAYERS ?= " \
    /path/to/yocto/meta \
    /path/to/yocto/meta-poky \
    /path/to/yocto/meta-yocto-bsp \
    /path/to/yocto/meta-xilinx
    /path/to/yocto/meta-xilinx-tools
    /path/to/yocto/meta-wssha256kern \
    "


II. Misc
========

Currently, the driver has the base address of the peripheral hard-coded, and does not use the built in device tree. It works, however could use much improvement. 

#TODO: 
1. Integrate linux device tree support and structures (linux/of.h I think..)
2. Create helper functions to abstract away the different ways we might want to use the hardware blocks
3. Userspace API? 
