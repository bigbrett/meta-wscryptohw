#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#
SUMMARY = "wsaescbc engine to integrate the aes-cbc block into openSSL"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "openssl"

SRC_URI = "git://github.com/bigbrett/wsaescbcengine.git;branch=master"
SRCREV = "${AUTOREV}"


# Add the .so to the main package’s files list
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/*.so \
                 ${bindir}/wsaescbcenginetest "

# Ensure that the DEV package doesn't grab them first
# commenting this out did not change anything
FILES_SOLIBSDEV = ""

S = "${WORKDIR}/git"

# Override all the makefile variables for cross compilation
PARALLEL_MAKE = ""
EXTRA_OEMAKE = "'CC=${CC}' \
                'LIB=-Llib -L=${libdir} ${LDFLAGS} -lcrypto -lwssha'"
#                'LIB=-Llib -L=${libdir} ${LDFLAGS} -lcrypto -lwssha'"

#LIB = "-Llib -L=${libdir} ${LDFLAGS}"
#INC = "-I include -I =/usr/include/openssl"
#
#do_compile() {
## compile source code
#      mkdir ${S}/build ${S}/bin
#      ${CC} ${CFLAGS} -fPIC ${S}/src/wsaescbcengine.c -c -o ${S}/build/wsaescbcengine.o 
#      ${CC} -shared -o ${S}/bin/libwsaescbcengine.so ${LIB} ${S}/build/wsaescbcengine.o
## compile test TODO doesn't work
#      ${CC} ${CFLAGS} -o ${S}/bin/test ${S}/test/wsaescbcengine_test.c ${INC} ${LIB} 
#}

do_install() {
# install engine shared library
      install -d ${D}${libdir}
      install -m 0755 ${S}/bin/libwsaescbcengine.so ${D}${libdir}
# install test binary
      install -d ${D}${bindir}
      install -m 0755 ${S}/bin/wsaescbcenginetest ${D}{bindir}
}
