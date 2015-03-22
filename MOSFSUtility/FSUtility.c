# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

/***** Local Includes ****/

# include "Common.h"
# include "FileSystem.h"
# include "Directory.h"

/**** Function Declartion ******/
int Initialize() ;
void UnInitialize() ;
void StartUtility() ;
void ChangeDirectory() ;
void CreateDirectory() ;
void CopyFileToMOSFS() ;
void CopyFileFromMOSFS() ;
void DumpFSTable() ;

/****************************************************/

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "\n Usage: mosfsutil <LBSStartSector> <DiskImageFileName>\n") ;
		exit(1) ;
	}
	
	int iLBAStartSector = atoi(argv[1]) ;
	if(iLBAStartSector < 0)
	{
		fprintf(stderr, "\n Invalid LBAStartSector = %d", iLBAStartSector) ;
		exit(1) ;
	}

	uiLBAStartSector = iLBAStartSector ;
	DiskImageFileName = argv[2] ;

	if(Initialize() < 0)
		exit(2) ;

	StartUtility() ;

	UnInitialize() ;

	return 0 ;
}

int Initialize()
{
	if((fd_DiskImageFile = open(DiskImageFileName, O_RDWR)) < 0)
	{
		perror("OPEN") ;
		return -1 ;
	}
	
	FileSystem_Initialize() ;
	if(FileSystem_Mount(&FSMountInfo_FD1) != FileSystem_SUCCESS)
	{
		fprintf(stderr, "\nFileSystem Init Failed\n") ;
		return -1 ;
	}

	return 0 ;
}

void UnInitialize()
{
	if(FileSystem_UnMount(&FSMountInfo_FD1) != FileSystem_SUCCESS)
	{
		fprintf(stderr, "\n FileSystem UnMount Failed\n") ;
	}
	
	close(fd_DiskImageFile) ;
}

void StartUtility()
{
	int iChoice ;
	byte errCode ;

	while(1)
	{
		printf("\n\n ******************************************* \n") ;
		printf("\n 1) List Contents.\n 2) Change Directory.\n 3) Create Directory.\n 4) Copy File To MOS FS.\n 5) Copy File From MOS FS.\n 6) Exit.\n 7) Dump FS Table.\n") ;
		printf("\n\tChoice: ") ;
		scanf("%d", &iChoice) ;

		switch(iChoice)
		{
			case 1:
					if((errCode = Directory_GetContent_SPIKE(&FSMountInfo_FD1)) != Directory_SUCCESS)
						fprintf(stderr, "\n Failed To Read Dir Entry: %u\n", errCode) ;
					break ;

			case 2:
					ChangeDirectory() ;
					break ;

			case 3:
					CreateDirectory() ;
					break ;

			case 4:
					CopyFileToMOSFS() ;
					break ;

			case 5:
					CopyFileFromMOSFS() ;
					break ;

			case 6:
					return ;	

			case 7:
					DumpFSTable() ;
					break ;

			default:
					fprintf(stderr, "\n Invalid Choice...\n") ;
		}
	}
}

void ChangeDirectory()
{
	byte errCode ;
	char szDirName[100] ;

	printf("\n Enter Dir Name: ") ;
	scanf("%s", szDirName) ;

	if((errCode = Directory_Change(&FSMountInfo_FD1, szDirName)) != Directory_SUCCESS)
	{
		fprintf(stderr, "\n Directory Change Failed: %u", errCode) ;
		return ;
	}

	fprintf(stderr, "\n Directory Changed Successfully\n") ;
}

void CreateDirectory()
{
	byte errCode ;
	char szSrcFileName[100] ;

	printf("\n Enter Directoy Name: ") ;
	scanf("%s", szSrcFileName) ;

	if((errCode = Directory_Create(&FSMountInfo_FD1, szSrcFileName, ATTR_DIR_DEFAULT | ATTR_TYPE_DIRECTORY)) != Directory_SUCCESS)
	{
		fprintf(stderr, "\nFailed to Create Directory on MOS FS Image File: %u\n", errCode) ;
		return ;
	}
}

void CopyFileToMOSFS()
{
	byte errCode ;
	char szSrcFileName[100], szDestFileName[100] ;
	int fd_SrcCopyFile, n, uiDestFileOffset ;
	char buf[4096];

	printf("\n Enter Source File Name: ") ;
	scanf("%s", szSrcFileName) ;

	printf("\n Enter Destination File Name: ") ;
	scanf("%s", szDestFileName) ;

	if((errCode = Directory_Create(&FSMountInfo_FD1, szDestFileName, ATTR_FILE_DEFAULT | ATTR_TYPE_FILE)) != Directory_SUCCESS)
	{
		fprintf(stderr, "\nFailed to Create File on MOS FS Image File: %u\n", errCode) ;
		return ;
	}

	if((fd_SrcCopyFile = open(szSrcFileName, O_RDONLY)) < 0)
	{
		PUT_ERROR_MSG("OPEN") ;
		return ;
	}

	uiDestFileOffset = 0 ;
	for(;;)
	{
		if((n = read(fd_SrcCopyFile, &buf, 512)) < 0)
		{
			PUT_ERROR_MSG("READ") ;
			close(fd_SrcCopyFile) ;
			continue ;
		}

		if(n == 0)
			break ;

		if((errCode = Directory_FileWrite(&FSMountInfo_FD1, szDestFileName, (char*)&buf, uiDestFileOffset, n)) != Directory_SUCCESS)
		{
			fprintf(stderr, "\n Error While writing the Dest File on MOS FS Image File: %u\n", errCode) ;
			break ; 
		}

		uiDestFileOffset += n ;
	}
	
	close(fd_SrcCopyFile) ;

	if(n == 0)
		printf("\n File Successfully Copied\n") ;
						
}

void CopyFileFromMOSFS()
{
	byte errCode ;
	char szSrcFileName[100], szDestFileName[100] ;
	int fd_DestCopyFile, n, uiSrcFileOffset ;
	char buf[1024] ;

	printf("\n Enter Source File Name: ") ;
	scanf("%s", szSrcFileName) ;

	printf("\n Enter Destination File Name: ") ;
	scanf("%s", szDestFileName) ;

	if((fd_DestCopyFile = open(szDestFileName, O_WRONLY | O_CREAT, 0664)) < 0)
	{
		PUT_ERROR_MSG("OPEN") ;
		return ;
	}

	uiSrcFileOffset = 0 ;
	n = 0 ;
	for(;;)
	{
		if((errCode = Directory_FileRead(&FSMountInfo_FD1, szSrcFileName, (char*)&buf, uiSrcFileOffset, 512, &n)) != Directory_SUCCESS)
		{
			if(errCode != Directory_ERR_EOF)
			{
				fprintf(stderr, "\n Error While reading the File from MOS FS Image File: %u\n", errCode) ;
				break ;
			}
			else
			{
				if(write(fd_DestCopyFile, &buf, n) < 0)
				{
					PUT_ERROR_MSG("WRITE") ;
					close(fd_DestCopyFile) ;
					return ;	
				}
				n = 0 ;
				break ;
			}
		}

		if(write(fd_DestCopyFile, &buf, n) < 0)
		{
			PUT_ERROR_MSG("WRITE") ;
			close(fd_DestCopyFile) ;
			return ;	
		}

		uiSrcFileOffset += 512 ;
		if(n < 512)
		{
			n = 0 ;
			break ;
		}
	}
	
	close(fd_DestCopyFile) ;

	if(n == 0)
		printf("\n\n File Successfully Read\n") ;
}

void DumpFSTable()
{
	byte errCode ;
	char szFileName[100] ;
	int fd, n;

	printf("\n Enter DumpFile Name: ") ;
	scanf("%s", szFileName) ;

	if((fd = open(szFileName, O_WRONLY | O_CREAT, 0664)) < 0)
	{
		PUT_ERROR_MSG("OPEN") ;
		return ;
	}

	FileSystem_BootBlock* FSBootBlock = &(FSMountInfo_FD1.FSBootBlock) ;
	
	char buf[512] ;
	int iSec = 1 + FSBootBlock->BPB_RsvdSecCnt ;
	int end = iSec + FSBootBlock->BPB_FSTableSize ;

	printf("\n FS Table Size (in sectors / bytes) = %d / %d", FSBootBlock->BPB_FSTableSize, FSBootBlock->BPB_FSTableSize * 512) ;
	
	for(; iSec < end; iSec++)
	{
		if(LSEEK(fd_DiskImageFile, iSec * 512, SEEK_SET) < 0)
		{
			PUT_ERROR_MSG("LSEEK") ;
			return ;
		}

		if((n = read(fd_DiskImageFile, buf, 512)) < 0)
		{
			PUT_ERROR_MSG("READ") ;
			return ;
		}

		if(write(fd, buf, n) < n)
		{
			PUT_ERROR_MSG("WRITE") ;
			return ;
		}
	}

	close(fd) ;

	printf("\n DONE\n") ;
}

