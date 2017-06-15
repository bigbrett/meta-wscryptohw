[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

# meta-wscryptohw
A Yocto meta-layer supporting the AES, SHA256, and RSA hardware crypto blocks for use in a hardware supported vpn running on the Zedboard (Xilinx Zynq-7020) 

Created by Brett Nicholas.

# Dependencies
This layer depends on:
```
  URI: git://git.openembedded.org/bitbake
  branch: master
  
  URI: git://git.openembedded.org/openembedded-core
  layers: meta
  branch: master
  
  URI: git://git.yoctoproject.org/meta-xilinx
  branch: morty
  
  URI: https://github.com/Xilinx/meta-xilinx-tools.git
  branch: rel-v2016.3     
```

# Table of Contents
  1. Adding the wscryptohw layer to your build
  2. Testing the kernel drivers
  3. Misc
  4. TODO
  5. Notes

# 1. Adding the wscryptohw layer to your build

In order to use this layer, you need to make the Yocto build system aware of
it.

Assuming the wscryptohw layer exists at the top-level of your yocto build tree, you can add it to the build system by adding the location of the wscryptohw layer to bblayers.conf, along with any other layers needed. e.g.:

```
  BBLAYERS ?= " \
    /path/to/yocto/meta \
    /path/to/yocto/meta-poky \
    /path/to/yocto/meta-yocto-bsp \
    /path/to/yocto/meta-xilinx
    /path/to/yocto/meta-xilinx-tools
    /path/to/yocto/meta-wscryptohw \
    "
```
The output kernel driver binaries can be found at `${WORKDIR}/ws<METHOD>.ko`, where \<METHOD\> is one of the following crypto methods: aes, sha256, or rsa. The module should be automatically installed in your image's /lib/modules folder. If you can't find it, try the following: 
```
bitbake -e ws<METHOD>-mod | grep ^WORKDIR //first find the value of ${WORKDIR}
find ${WORKDIR} -name "ws<METHOD>kern.ko" 
```
# 2. Testing the Kernel Driver
Each kernel driver (ws\<METHOD\>.ko) contains a self-checking test program that can be run in userspace to test the correct operation of the driver. The test program is included in the recipe ws\<METHOD\>test The test recipe should be automatically built after the layer is added to your build. The output files are located at `${WORKDIR}/ws<METHOD>test`, or at `${WORKDIR}/image/usr/bin/ws<METHOD>test`. See the `ws<METHOD>test_${PV}.bb` file if you are unsure.

If the test recipe is not added to your build for some reason, you can manually build it using the command `bitbake ws<METHOD>test`

# 3. Misc

Currently, the driver has the base address of the peripheral hard-coded, and does not use the built in device tree. It works, however could use much improvement. I'm sure there are many a lurking oops. There is also the possibility of using a linux device driver framework. 

# 4. TODO 
1. Integrate linux device tree support and structures (linux/of.h I think..)
2. Create helper functions to abstract away the different ways we might want to use the hardware blocks

# 5. Notes
Below are a series of notes to myself. They probably will mean nothing to you.

## YOCTO BUILD ERRORS AND THEIR RESOLUTIONS
### Recipe: wssha256engine
#### Initial QA errors in do_compile 
when I first tried building the recipe, I got the following errors:
```
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: wssha256engine: The compile log indicates that host include and/or library paths were used.
         Please check the log '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/temp/log.do_compile' for more information. [compile-host-path]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: No GNU_HASH in the elf binary: '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [ldflags]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: -dev package contains non-symlink .so: wssha256engine-dev path '/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [dev-elf]
ERROR: wssha256engine-0.1-r0 do_package_qa: QA run found fatal errors. Please consider fixing them.
ERROR: wssha256engine-0.1-r0 do_package_qa: Function failed: do_package_qa
ERROR: Logfile of failure stored in: /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/temp/log.do_package_qa.19467
ERROR: Task (/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/meta-wssha256/recipes-kernel/wssha256engine/wssha256engine_0.1.bb:do_package_qa) failed with exit code '1'
```
Errors were resolved as follows:

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: wssha256engine: The compile log indicates that host include and/or library paths were used.`
* Changed the -L${libdir} to -L=${libdir} In EXTRA_OECONF_append to make it sysroot relative (https://lists.yoctoproject.org/pipermail/yocto/2012-August/008906.html)

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: No GNU_HASH in the elf binary: '/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [ldflags]`
* Added ${LDFLAGS} to the override of makefile LIB variable in EXTRA_OECONF_append

`ERROR: wssha256engine-0.1-r0 do_package_qa: QA Issue: -dev package contains non-symlink .so: wssha256engine-dev path '/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/packages-split/wssha256engine-dev/usr/lib/libwssha256engine.so' [dev-elf]`
* Added `FILES_${PN} += "${libdir}/*.so"` and `FILES_SOLIBSDEV = ""` to recipe to ensure that the .so links weren't only included in ${PN}-dev

#### Error linking test script against libcrypto
When I tried to build the test program using the Yocto flow, I got the following error 
```
| Building Tests...
| arm-poky-linux-gnueabi-gcc  -march=armv7-a -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a9 --sysroot=/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/sysroots/zedboard-zynq7 -g -Wall test/wssha256engine_test.c -I include  -Llib -L=/usr/lib -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -o bin/test
| /tmp/ccYXjZxs.o: In function `main':
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:18: undefined reference to `OPENSSL_add_all_algorithms_noconf'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:19: undefined reference to `ERR_load_crypto_strings'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:22: undefined reference to `ENGINE_load_dynamic'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:26: undefined reference to `ENGINE_by_id'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:55: undefined reference to `ENGINE_ctrl_cmd_string'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:56: undefined reference to `ENGINE_ctrl_cmd_string'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:57: undefined reference to `ENGINE_ctrl_cmd_string'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:66: undefined reference to `ENGINE_init'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:72: undefined reference to `ENGINE_get_name'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:83: undefined reference to `EVP_MD_CTX_create'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:86: undefined reference to `EVP_sha256'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:86: undefined reference to `EVP_DigestInit_ex'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:88: undefined reference to `EVP_DigestUpdate'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:90: undefined reference to `EVP_DigestFinal'
| /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/git/test/wssha256engine_test.c:96: undefined reference to `EVP_MD_CTX_destroy'
```
This was resolved by properly linking against libcrypto. I originally added the systroot library directory to the library search paths, but forgot to actually link against libcrypto (which has the definitions for the EVP API. The solution is to to add `-lcrypto` to the makefile variable overrides:
```
EXTRA_OEMAKE = "'CC=${CC}' \
                'LIB=-Llib -L=${libdir} ${LDFLAGS} -lcrypto'"
```
#### QA issue when installing test binary in image
```
ERROR: wssha256engine-0.1-r0 do_package: QA Issue: wssha256engine: Files/directories were installed but not shipped in any package:
  /usr/bin
Please set FILES such that these items are packaged. Alternatively if they are unneeded, avoid installing them or delete them within do_install.
wssha256engine: 1 installed and not shipped files. [installed-vs-shipped]
ERROR: wssha256engine-0.1-r0 do_package: Fatal QA errors found, failing task.
ERROR: wssha256engine-0.1-r0 do_package: Function failed: do_package
ERROR: Logfile of failure stored in: /home/brett/Thesis/Zynq_Linux/Yocto/wslinux/build/tmp/work/cortexa9hf-neon-poky-linux-gnueabi/wssha256engine/0.1-r0/temp/log.do_package.29073
ERROR: Task (/home/brett/Thesis/Zynq_Linux/Yocto/wslinux/meta-wssha256/recipes-kernel/wssha256engine/wssha256engine_0.1.bb:do_package) failed with exit code '1'
```
This was resolved by adding the appropriate output directories to the package. Do this by changing the FILES\_${PN} variable in the recipe from `FILES_${PN} += " ${libdir}/*.so` to the following: 
```
FILES_${PN} += " ${libdir} \
                 ${bindir} \ 
                 ${libdir}/*.so \
                 ${bindir}/test "
```
