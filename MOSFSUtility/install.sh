./mosfsutil 63 $UPANIX_HOME/USBImage/30MUSB.img << EOF
1
3
osin
3
bin
3
lib
1
6
EOF

./mosfsutil 63 $UPANIX_HOME/USBImage/30MUSB.img << EOF
1
2
osin
4
$UPANIX_HOME/osutils/.procinit
.procinit
4
$UPANIX_HOME/osutils/.dll
.dll
1
6
EOF

./mosfsutil 63 $UPANIX_HOME/USBImage/30MUSB.img << EOF
1
2
bin
4
$UPANAPPS_HOME/shell/msh
msh
4
$UPANAPPS_HOME/editor/edit
edit
1
6
EOF

./mosfsutil 63 $UPANIX_HOME/USBImage/30MUSB.img << EOF
1
2
lib
4
$UPANAPPS_HOME/libc/libc.so
libc.so
4
$UPANAPPS_HOME/libmterm/libmterm.so
libmterm.so
1
6
EOF
