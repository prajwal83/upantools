

/* FAT 32 File System Support */

# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>
# include <string.h>

# include "FileSystem.h"

void
FileSystem_Initialize()
{
	FSMountInfo_FD1.bMounted = FALSE ;
	FSMountBuffer = NULL ;
}

byte
FileSystem_Mount(FileSystem_MountInfo* FSMountInfo)
{
	byte bStatus ;
	
	if(FSMountInfo->bMounted == TRUE)
		return FileSystem_ERR_ALREADY_MOUNTED ;

	RETURN_IF_NOT(bStatus, FileSystem_GetFSBootBlock(FSMountInfo), FileSystem_SUCCESS) ;

	printf("\n Sector Per Track @ Mount = %d", FSMountInfo->FSBootBlock.BPB_SecPerTrk);

	RETURN_IF_NOT(bStatus, FileSystem_FSTableMounter(FSMountInfo, FS_MOUNT), FileSystem_SUCCESS) ;

	RETURN_IF_NOT(bStatus, FileSystem_ReadRootDirectory(FSMountInfo), FileSystem_SUCCESS) ;

	FSMountInfo->bMounted = TRUE ;

	return FileSystem_SUCCESS ;
}

byte
FileSystem_GetFSBootBlock(FileSystem_MountInfo* FSMountInfo)
{
	char bArrFSBootBlock[512] ;
	byte bStatus ;
	
	if(LSEEK(fd_DiskImageFile, 512, SEEK_SET) < 0)
	{
		PUT_ERROR_MSG("LSEEK") ;
		return EXTERNAL_ERROR ;
	}

	if(read(fd_DiskImageFile, &bArrFSBootBlock, 512) < 0)
	{
		PUT_ERROR_MSG("READ") ;
		return EXTERNAL_ERROR ;
	}

	if((byte)bArrFSBootBlock[510] != (byte)0x55 || (byte)bArrFSBootBlock[511] != (byte)0xAA)
		return FileSystem_ERR_INVALID_BPB_SIGNATURE ;

	FileSystem_BootBlock* FSBootBlock = &FSMountInfo->FSBootBlock ;

	memcpy((char*)FSBootBlock, &bArrFSBootBlock, sizeof(FileSystem_BootBlock)) ;

	if(FSBootBlock->BPB_BootSig != 0x29)
		return FileSystem_ERR_INVALID_BOOT_SIGNATURE ;

/*
	if(FSBootBlock->BPB_SecPerTrk != 18)
		return FileSystem_ERR_INVALID_SECTORS_PER_TRACK ;

	if(FSBootBlock->BPB_NumHeads != 2)
		return FileSystem_ERR_INVALID_NO_OF_HEADS ;

	if(FSBootBlock->BPB_TotSec32 != 2880)
		return FileSystem_ERR_INVALID_DRIVE_SIZE_IN_SECTORS ;
*/	
	if(FSBootBlock->BPB_FSTableSize == 0)
		return FileSystem_ERR_ZERO_FATSZ32 ;

	if(FSBootBlock->BPB_BytesPerSec != 0x200)
		return FileSystem_ERR_UNSUPPORTED_SECTOR_SIZE ;
		
	return FileSystem_SUCCESS ;
}

byte
FileSystem_FSTableMounter(FileSystem_MountInfo* FSMountInfo, MOUNT_TYPE mountType)
{
	byte bStatus = FileSystem_SUCCESS ;
	unsigned i, uiCopyPosition ;
	
	FileSystem_BootBlock* FSBootBlock = &FSMountInfo->FSBootBlock ;

	if(mountType == FS_MOUNT)
		FSMountBuffer = (char*)malloc(sizeof(char) * FSBootBlock->BPB_FSTableSize * 512) ;

	if(mountType == FS_UNMOUNT)
	{
		char bSectorBuffer[512] ;

		bSectorBuffer[510] = 0x55 ; /* BootSector Signature */
		bSectorBuffer[511] = 0xAA ;

		printf("\n Sector Per Track @ UnMount = %d", FSMountInfo->FSBootBlock.BPB_SecPerTrk);

		memcpy(&bSectorBuffer, (char*)&FSMountInfo->FSBootBlock, sizeof(FileSystem_BootBlock)) ;

		if(LSEEK(fd_DiskImageFile, 512, SEEK_SET) < 0)
		{
			PUT_ERROR_MSG("LSEEK") ;
			return EXTERNAL_ERROR ;
		}

		if(write(fd_DiskImageFile, (char*)bSectorBuffer, 512) < 0)
		{
			PUT_ERROR_MSG("WRITE") ;
			return EXTERNAL_ERROR ;
		}
	}

	if(LSEEK(fd_DiskImageFile, (FSBootBlock->BPB_RsvdSecCnt + 1) * 512, SEEK_SET) < 0)
	{
		PUT_ERROR_MSG("LSEEK") ;
		return EXTERNAL_ERROR ;
	}

	for(i = 0, uiCopyPosition = 0; i < FSBootBlock->BPB_FSTableSize; i++, uiCopyPosition += 512)
	{
		if(mountType == FS_MOUNT)
		{
			if(read(fd_DiskImageFile, (char*)FSMountBuffer + uiCopyPosition, 512) < 0)
			{
				PUT_ERROR_MSG("READ") ;
				return EXTERNAL_ERROR ;
			}
		}
		else
		{
			if(write(fd_DiskImageFile, (char*)FSMountBuffer + uiCopyPosition, 512) < 0)
			{
				PUT_ERROR_MSG("WRITE") ;
				return EXTERNAL_ERROR ;
			}
		}
	}

	return bStatus ;
}

unsigned 
FileSystem_GetRealSectorNumber(const unsigned uiSectorID, const FileSystem_BootBlock* FSBootBlock) 
{
	return uiSectorID + 1/*BPB*/ + FSBootBlock->BPB_RsvdSecCnt + FSBootBlock->BPB_FSTableSize ;
}

byte
FileSystem_GetSectorEntryValue(const FileSystem_MountInfo* FSMountInfo, const unsigned uiSectorID,
		unsigned* uiSectorEntryValue)
{
	FileSystem_BootBlock* FSBootBlock = (FileSystem_BootBlock*)&FSMountInfo->FSBootBlock ;
	
	if(uiSectorID > (FSBootBlock->BPB_FSTableSize * FSBootBlock->BPB_BytesPerSec / 4))
		return FileSystem_ERR_INVALID_CLUSTER_ID ;

	*uiSectorEntryValue = ((unsigned*)FSMountBuffer)[uiSectorID] & 0x0FFFFFFF ;

	return FileSystem_SUCCESS ;
}

byte
FileSystem_SetSectorEntryValue(const FileSystem_MountInfo* FSMountInfo, const unsigned uiSectorID, unsigned uiSectorEntryValue)
{
	FileSystem_BootBlock* FSBootBlock = (FileSystem_BootBlock*)&FSMountInfo->FSBootBlock ;
	
	if(uiSectorID > (FSBootBlock->BPB_FSTableSize * FSBootBlock->BPB_BytesPerSec / 4))
		return FileSystem_ERR_INVALID_CLUSTER_ID ;
	
	uiSectorEntryValue &= 0x0FFFFFFF ;

	((unsigned*)FSMountBuffer)[uiSectorID] = ((unsigned*)FSMountBuffer)[uiSectorID] & 0xF0000000 ;

	((unsigned*)FSMountBuffer)[uiSectorID] = ((unsigned*)FSMountBuffer)[uiSectorID] | uiSectorEntryValue ;

	if(uiSectorEntryValue == EOC)
		FSBootBlock->uiUsedSectors++ ;
	else if(uiSectorEntryValue == 0)
		FSBootBlock->uiUsedSectors-- ;

	return FileSystem_SUCCESS ;
}

byte
FileSystem_ReadRootDirectory(FileSystem_MountInfo* FSMountInfo)
{
	byte bStatus = FileSystem_SUCCESS ;
	unsigned uiSec ;
	FileSystem_BootBlock* FSBootBlock = &FSMountInfo->FSBootBlock ;
	FileSystem_CurrentWorkingDirectory* FScwd = &FSMountInfo->FScwd ;
	
	char* bSectorBuffer[512] ;

	uiSec = FileSystem_GetRealSectorNumber(0, FSBootBlock) ;
	
	if(LSEEK(fd_DiskImageFile, uiSec * 512, SEEK_SET) < 0)
	{
		PUT_ERROR_MSG("LSEEK") ;
		return EXTERNAL_ERROR ;
	}

	if(read(fd_DiskImageFile, (char*)&FScwd->DirEntry, sizeof(FileSystem_DIR_Entry)) < 0)
	{
		PUT_ERROR_MSG("READ") ;
		return EXTERNAL_ERROR ;
	}

	FScwd->uiSectorNo = 0 ;
	FScwd->uiSectorEntryPosition = 0 ;
		
	return bStatus ;
}

byte
FileSystem_UnMount(FileSystem_MountInfo* FSMountInfo)
{
	byte bStatus ;

	if(FSMountInfo->bMounted == FALSE)
		return FileSystem_ERR_NOT_MOUNTED ;

	RETURN_IF_NOT(bStatus, FileSystem_FSTableMounter(FSMountInfo, FS_UNMOUNT), FileSystem_SUCCESS) ;

	FSMountInfo->bMounted =  FALSE ;

	if(FSMountBuffer)
	{
		free(FSMountBuffer) ;
		FSMountBuffer = NULL ;
	}
	
	return FileSystem_SUCCESS ;
}

byte
FileSystem_AllocateSector(FileSystem_MountInfo* FSMountInfo, unsigned* uiFreeSectorID)
{
	byte bStatus ;
	unsigned uiSectorID = 1 ;
	unsigned uiSectorEntryValue = EOC ;

	while(TRUE)
	{
		if(FileSystem_GetSectorEntryValue(FSMountInfo, uiSectorID, &uiSectorEntryValue) != FileSystem_SUCCESS)
			return FileSystem_ERR_NO_FREE_CLUSTER ;

		if(uiSectorEntryValue == 0)
			break ;
			
		uiSectorID++ ;
	}

	RETURN_IF_NOT(bStatus, FileSystem_SetSectorEntryValue(FSMountInfo, uiSectorID, EOC), FileSystem_SUCCESS) ;
		
	*uiFreeSectorID = uiSectorID ;	
	return FileSystem_SUCCESS ;
}

