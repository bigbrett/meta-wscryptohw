SUMMARY = "Linux driver for a SHA256 encryption block in ZYNQ 7000 FPGA logic"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module cmake

SRC_URI = "git://github.com/bigbrett/wssha256-kmod.git;branch=kmodtest"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

# Add the .so to the main package’s files list
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/*.so \
                 ${libdir}/*.a \
                 ${bindir}/wssha256-kmodtest"

# Ensure that the DEV package doesn't grab them first
# commenting this out did not change anything
FILES_SOLIBSDEV = ""

EXTRA_OECMAKE = "-DKERNEL_SRC=${STAGING_KERNEL_DIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
do_install() {
# install uapi libraries
      install -d ${D}${libdir}
      install -m 0755 ${B}/lib/*.so ${D}${libdir}
      install -m 0755 ${B}/lib/*.a ${D}${libdir}
# install test binary
      install -d ${D}${bindir}
      install -m 0755 ${B}/test/wssha256-kmodtest ${D}{bindir}
}
