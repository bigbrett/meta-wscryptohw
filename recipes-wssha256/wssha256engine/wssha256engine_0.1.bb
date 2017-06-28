#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#
SUMMARY = "wssha256 engine to integrate the sha256 block into openSSL"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "openssl"

SRC_URI = "git://github.com/bigbrett/wssha256engine.git;branch=msg-len-fix"
SRCREV = "${AUTOREV}"


# Add the .so to the main package’s files list
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/*.so \
                 ${bindir}/wssha256enginetest "

# Ensure that the DEV package doesn't grab them first
# commenting this out did not change anything
FILES_SOLIBSDEV = ""

S = "${WORKDIR}/git"

# Override all the makefile variables for cross compilation
PARALLEL_MAKE = ""
EXTRA_OEMAKE = "'CC=${CC}' \
                'LIB=-Llib -L=${libdir} ${LDFLAGS} -lcrypto -lwssha'"

do_install() {
# install engine shared library
      install -d ${D}${libdir}
      install -m 0755 ${S}/bin/libwssha256engine.so ${D}${libdir}
# install test binary
      install -d ${D}${bindir}
      install -m 0755 ${S}/bin/wssha256enginetest ${D}{bindir}
}
