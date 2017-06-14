#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "AES test application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://wsaescbctest.c \
           file://wsaes.h \
           file://wsaeskern.h"

S = "${WORKDIR}"

do_compile() {
	     ${CC} wsaescbctest.c -o wsaescbctest ${LDFLAGS}
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 wsaescbctest ${D}${bindir}
}
