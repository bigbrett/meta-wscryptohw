SUMMARY = "Linux driver for a SHA256 encryption block in ZYNQ 7000 FPGA logic"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://wssha256mod.c \
           file://xsha256.h \
           file://xsha256_hw.h \
           file://xsha256.c \
           file://libxil.a \
           file://COPYING \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
