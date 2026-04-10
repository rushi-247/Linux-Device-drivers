savedcmd_pcd.ko := ld -r -m elf_x86_64 -z noexecstack --no-warn-rwx-segments --build-id=sha1  -T /usr/src/linux-headers-6.17.0-19-generic/scripts/module.lds -o pcd.ko pcd.o pcd.mod.o .module-common.o
