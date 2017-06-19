#
#
#
#
#

SUMMARY = "Userspace API for wsaescbc256 kernel module"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://Makefile \
           file://wsaescbc.c \
		   file://wsaescbc.h \
		   file://wsaeskern.h \
		   file://wsaescbc_api_test.c "

# Add the .so to the main package’s files list
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/libwsaescbc.a \
                 ${bindir}/wsaescbc_api_test "

# Ensure that the DEV package doesn't grab them first
# # commenting this out did not change anything
#FILES_SOLIBSDEV = ""

S = "${WORKDIR}"

do_compile() {
# Make shared library
			${CC} ${CFLAGS} -g -c -o ${S}/wsaescbc.o ${S}/wsaescbc.c 
			${AR} -c -v -q ${S}/libwsaescbc.a ${S}/wsaescbc.o #${LDFLAGS}
# Compile test program linked against shared library
			${CC} ${CFLAGS} ${S}/wsaescbc_api_test.c ${S}/libwsaescbc.a -o ${S}/wsaescbc_api_test ${LDFLAGS} 
}

do_install() {
	     install -d ${D}${libdir}
	     install -d ${D}${bindir}
	     install -m 0755 ${S}/libwsaescbc.a ${D}${libdir}
	     install -m 0755 ${S}/wsaescbc_api_test ${D}${bindir}
}
