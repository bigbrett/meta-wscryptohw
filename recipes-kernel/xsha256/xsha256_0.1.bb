#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "SHA256 Xilinx middleware driver"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://xsha256.c \
           file://xsha256_hw.h \
           file://xsha256.h \
           file://xsha256_linux.c \
           "

S = "${WORKDIR}"

do_compile() {
	     ${CC} xsha256.c xsha256_linux.c -o xsha256 ${LDFLAGS}
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 xsha256 ${D}${bindir}
}
