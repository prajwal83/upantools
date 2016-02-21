mkdir -p Mnt/bin
mkdir -p Mnt/lib
mkdir -p Mnt/osin

cp ../../upanapps/shell/msh Mnt/bin/
cp ../../upanapps/editor/edit Mnt/bin/

cp ../../upanapps/libc/libc.so Mnt/lib/
cp ../../upanapps/libmterm/libmterm.so Mnt/lib/

cp ../../upanix/osutils/.dll Mnt/osin/
cp ../../upanix/osutils/.procinit Mnt/osin/
