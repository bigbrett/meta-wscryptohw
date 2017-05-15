#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#
SUMMARY = "wssha256 engine to integrate the sha256 block into openSSL"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "openssl"

SRC_URI = "git://github.com/bigbrett/wssha256engine.git"
SRCREV = "${AUTOREV}"


# Add the .so to the main package’s files list
FILES_${PN} += "${libdir}/*.so"
# Ensure that the DEV package doesn't grab them first
FILES_SOLIBSDEV = ""

S = "${WORKDIR}/git"

# Override all the makefile variables for cross compilation
EXTRA_OEMAKE = "'CC=${CC}' \
                'LIB=-Llib -L=${libdir} ${LDFLAGS}'"

#do_compile() {
#	     ${CC} wssha256engine.c -o wssha256engine.so ${LDFLAGS}
#}

do_install() {
	     install -d ${D}${libdir}
	     install -m 0755 ${S}/bin/libwssha256engine.so ${D}${libdir}
}
