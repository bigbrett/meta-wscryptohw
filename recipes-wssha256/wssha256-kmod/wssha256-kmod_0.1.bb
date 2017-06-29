SUMMARY = "Linux driver for a SHA256 encryption block in ZYNQ 7000 FPGA logic"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

SRC_URI = "git://github.com/bigbrett/wssha256-kmod.git;branch=master"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"


# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
