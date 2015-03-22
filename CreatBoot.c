#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdio.h>

#define SECTOR_SIZE 512

int main(int argc, char *argv[])
{
	int bfd, ffd;
	char bootSecBuf[SECTOR_SIZE];

	if(argc != 3)
	{
		printf("\n Invalid Argument list\n");
		printf("\n Usage: creatboot <.bin filename> <floppy img/dev file>\n");
		return -1;
	}

	if((bfd = open(argv[1], O_RDONLY)) < 0)
	{
		perror("READING .BIN FILE");
		return -1;
	}

	if(read(bfd, &bootSecBuf, SECTOR_SIZE) < 0)
	{
		perror(" ");
		return -1;
	}

	bootSecBuf[510] = 0x55;
	bootSecBuf[511] = 0xAA;

	close(bfd);

	if((ffd = open(argv[2], O_RDWR)) < 0)
	{
		perror("OPENING FLOPPY DRIVE");
		return -1;
	}

	if(lseek(ffd, (off_t)0, SEEK_SET) < 0)
	{
		perror("SEEEKING FIRST SECTOR OF FLOPPY DISK");
		return -1;
	}

	if(write(ffd, &bootSecBuf, SECTOR_SIZE) < 0)
	{
		perror("WRITE FIRST SECTOR TO FLOPPY");
		return -1;
	}

	//fwrite(&bootSecBuf, 1, SECTOR_SIZE, stdout);
	printf("\n BootSector Successfully loaded to Floppy\n");
	close(ffd);
	
	return 0;
}
			
