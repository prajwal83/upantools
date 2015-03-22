#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdio.h>

#define SECTOR_SIZE 512

int main(int argc, char *argv[])
{
	int bfd, ffd, n, iw, startSec;
	char secBuf[SECTOR_SIZE];

	if(argc != 4)
	{
		printf("\n Invalid Argument list\n");
		printf("\n Usage: writesec <.bin filename> <floppy img/dev file> <SecStart>\n");
		return -1;
	}

	startSec = atoi(argv[3]);
	
	if(startSec < 1)
	{
		printf("\n Invalid Sector number\n");
		return -1;
	}
	
	if(startSec == 1)
	{
		printf("\n Will not write to bootsector\n");
		return -1;
	}

	if((bfd = open(argv[1], O_RDONLY)) < 0)
	{
		perror("READING .BIN FILE");
		return -1;
	}
	
	if((ffd = open(argv[2], O_RDWR)) < 0)
	{
		perror("OPENING FLOPPY DRIVE");
		return -1;
	}
	
	if(lseek(ffd, (off_t)(startSec-1)*SECTOR_SIZE, SEEK_SET) < 0)
	{
		perror("SEEEKING FIRST SECTOR OF FLOPPY DISK");
		return -1;
	}

	for(iw = 0;; iw++)
	{
		/*if(iw == 5)
				break;*/

		if((n = read(bfd, &secBuf, SECTOR_SIZE)) < 0)
		{
			perror(" ");
			return -1;
		}

		if(n == 0)
			break;
	
		if(write(ffd, &secBuf, n) < 0)
		{
			perror("WRITE SECTOR TO FLOPPY");
			return -1;
		}
	}
	
	//fwrite(&bootSecBuf, 1, SECTOR_SIZE, stdout);
	printf("\n Successfully Written to floppy starting at Sector %d\n", startSec);
	printf("\n %d Sectors Written\n", iw);
	close(bfd);
	close(ffd);
	
	return 0;
}
			
