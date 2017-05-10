#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "SHA256 UIO driver"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://wssha256uio.c \
           file://wssha.h \
           file://wssha.c \
           file://xsha256.c \
           file://xsha256_hw.h \
           file://xsha256.h \
           file://xsha256_linux.c \
           "

S = "${WORKDIR}"

do_compile() {
	     ${CC} wssha256uio.c wssha.c xsha256.c xsha256_linux.c -o wssha256uio ${LDFLAGS}
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 wssha256uio ${D}${bindir}
}
