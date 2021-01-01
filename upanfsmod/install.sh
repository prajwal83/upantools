mkdir -p Mnt/bin
mkdir -p Mnt/lib
mkdir -p Mnt/osin

cp ../../upanapps/out/msh Mnt/bin/
cp ../../upanapps/out/edit Mnt/bin/

cp ../../upanlibs/out/libc.so Mnt/lib/
cp ../../upanapps/out/libmterm.so Mnt/lib/

cp ../../upanix/osutils/.dll Mnt/osin/
