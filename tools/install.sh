#!/bin/sh
# gcc
curl -OL http://repos.moxielogic.org:7007/MoxieLogic/x86_64/bootstrap-moxie-elf-gcc-8.0.0-0.151.el7.centos.x86_64.rpm
# as, objdump
curl -OL http://repos.moxielogic.org:7007/MoxieLogic/x86_64/moxielogic-moxie-elf-binutils-2.29.51-0.151.x86_64.rpm

rpm2cpio bootstrap-moxie-elf-gcc-8.0.0-0.151.el7.centos.x86_64.rpm | xz -d | cpio -idmv
rpm2cpio moxielogic-moxie-elf-binutils-2.29.51-0.151.x86_64.rpm | xz -d | cpio -idmv
