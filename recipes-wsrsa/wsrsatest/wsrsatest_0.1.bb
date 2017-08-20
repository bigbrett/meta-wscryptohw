#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "RSA test application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://wsrsatest.c \
           file://wsrsakern.h"

S = "${WORKDIR}"

do_compile() {
	     ${CC} wsrsatest.c -o wsrsatest ${LDFLAGS}
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 wsrsatest ${D}${bindir}
}
