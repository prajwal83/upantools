> depend.d
for i in `ls *.c 2> /dev/null`
do
	obj=${i%.*}.o
	gcc -I./ -m32 -M $i >> depend.d
	echo "\t@echo \"compiling $i...\"" >> depend.d
	echo "\t@gcc -g -c -m32 -I./ $i -o ${obj}" >> depend.d
done
