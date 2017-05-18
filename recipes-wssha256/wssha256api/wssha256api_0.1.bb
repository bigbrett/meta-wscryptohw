#
#
#
#
#

SUMMARY = "Userspace API for wssha256 kernel module"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://wssha.c \
					 file://wssha.h \
					 file://wsshatest.c"

# Add the .so to the main package’s files list
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/*.so \
                 ${bindir}/wsshatest"
# Ensure that the DEV package doesn't grab them first
# # commenting this out did not change anything
FILES_SOLIBSDEV = ""


S = "${WORKDIR}"

do_compile() {
# Make shared library
			${CC} ${CFLAGS} -fPIC -c -o ${S}/wssha.o ${S}/wssha.c 
			${CC} ${CFLAGS} -shared -o ${S}/libwssha.so ${S}/wssha.o ${LDFLAGS}
# Compile test program linked against shared library
			${CC} ${CFLAGS} -L${S} ${S}/wsshatest.c -o ${S}/wsshatest -lwssha ${LDFLAGS}
}

do_install() {
	     install -d ${D}${libdir}
	     install -d ${D}${bindir}
	     install -m 0755 ${S}/libwssha.so ${D}${libdir}
	     install -m 0755 ${S}/wsshatest ${D}${bindir}
       rm -f ${S}/libwssha.so
}
