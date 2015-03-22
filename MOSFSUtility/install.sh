
./mosfsutil 0 ../../MOS/floppy/DRV1.FDD.img << EOF
1
2
osin
3
../../MOS/osutils/.procinit
.procinit
3
../../MOS/osutils/.dll
.dll
1
5
EOF

./mosfsutil 0 ../../MOS/floppy/DRV1.FDD.img << EOF
1
2
bin
3
../yard/msh
msh
3
../yard/edit
edit
1
5
EOF

./mosfsutil 0 ../../MOS/floppy/DRV1.FDD.img << EOF
1
2
lib
3
../yard/libc.so
libc.so
3
../yard/libmterm.so
libmterm.so
1
5
EOF

./mosfsutil 0 ../../MOS/floppy/DRV1.FDD.img << EOF
1
2
ldscripts
3
../yard/elf_i386.xc
elf_i386.xc
1
5
EOF
