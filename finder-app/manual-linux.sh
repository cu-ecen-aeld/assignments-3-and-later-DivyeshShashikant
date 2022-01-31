#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

if [ ! -d ${OUTDIR} ]
then
	mkdir -p ${OUTDIR}
	echo "DIrectory created"
fi

if [ ! -e ${OUTDIR} ]
then
	echo "Directory could not be created"
	exit 1

fi


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    
    #deep clean the kernel build tree - removing the .config file with any existing configurations
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper
    
    #configure our "virt" arm dev board we will simulate in QEMU
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
    
    #build a kernel image for booting with QEMU
    make -j16 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all
    
    #build any kernel modules
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules
    
    #build the device tree
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs
    echo "Device tree built"
  
    
    
    
fi 
#copy the generated image to OUTDIR
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
echo "Adding the Image in outdir"
echo "$OUTDIR"


echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
	cd "$OUTDIR"
	mkdir rootfs
	cd rootfs
	mkdir bin dev etc home lib lib64 proc sbin sys tmp usr var
	mkdir usr/bin usr/lib usr/sbin
	mkdir -p var/log	
	
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and insatll busybox
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
	
cd ${OUTDIR}/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
cp $SYSROOT/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
echo "copied ld-linux-aarch64.so.1"
cp $SYSROOT/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
echo "copied libm.so.6"
cp $SYSROOT/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
echo "copied libresolv.so.2"
cp $SYSROOT/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64
echo "copied libc.so.6"


# TODO: Make device nodes
#NULL node
sudo mknod -m 666 dev/null c 1 3
#CONSOLE node
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cp ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home
echo "copied finder.sh"
cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home
echo "copied finder-test.sh"
cp ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home
echo "copied writer"
cp ${FINDER_APP_DIR}/conf/ -r ${OUTDIR}/rootfs/home
echo "copied /conf"
cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home
echo "copied autorun-qemu.sh"


# TODO: Chown the root directory
  cd $OUTDIR/rootfs
  sudo chown -R root:root *
	
# TODO: Create initramfs.cpio.gz
cd $OUTDIR/rootfs
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
cd ..
gzip -f initramfs.cpio

