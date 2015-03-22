# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>
# include <string.h>

# include "Directory.h"

byte 
Directory_Create(FileSystem_MountInfo* FSMountInfo, char* szDirName, unsigned short usDirAttribute) 
{
	byte bStatus ;
	byte bIsPresent ;
	char bSectorBuffer[512] ;
	unsigned uiSectorNo ;
	unsigned uiSectorPos ;
	unsigned uiFreeSectorID ;

	if(FSMountInfo->FScwd.DirEntry.uiStartSectorID == EOC)
	{
		RETURN_IF_NOT(bStatus, FileSystem_AllocateSector(FSMountInfo, &uiFreeSectorID), FileSystem_SUCCESS) ;

		uiSectorNo = uiFreeSectorID ;
		uiSectorPos = 0 ;

		FSMountInfo->FScwd.DirEntry.uiStartSectorID = uiFreeSectorID ;
	}
	else
	{
		RETURN_IF_NOT(bStatus, 
				Directory_FindDirectory(FSMountInfo, szDirName, &uiSectorNo, &uiSectorPos, &bIsPresent, bSectorBuffer),
				Directory_SUCCESS) ;

		if(bIsPresent == TRUE)
			return Directory_ERR_EXISTS ;

		if(uiSectorPos == EOC)
		{
			RETURN_IF_NOT(bStatus, FileSystem_AllocateSector(FSMountInfo, &uiFreeSectorID), FileSystem_SUCCESS) ;

			RETURN_IF_NOT(bStatus, FileSystem_SetSectorEntryValue(FSMountInfo, uiSectorNo, uiFreeSectorID), 
						FileSystem_SUCCESS) ;

			uiSectorNo = uiFreeSectorID ;
			uiSectorPos = 0 ;
		}
	}

	FileSystem_CurrentWorkingDirectory* FScwd = &FSMountInfo->FScwd ;

	Directory_PopulateDirEntry(((FileSystem_DIR_Entry*)bSectorBuffer) + uiSectorPos, szDirName, usDirAttribute, FScwd->uiSectorNo, FScwd->uiSectorEntryPosition) ;

	RETURN_IF_NOT(bStatus, Directory_RawWrite(FSMountInfo, uiSectorNo, uiSectorNo + 1, bSectorBuffer), Directory_SUCCESS) ;
	
	FScwd->DirEntry.uiSize++ ;

	RETURN_IF_NOT(bStatus, 
					Directory_RawRead(FSMountInfo, FScwd->uiSectorNo, FScwd->uiSectorNo + 1, bSectorBuffer),
					Directory_SUCCESS) ;

	memcpy( (char*)(((FileSystem_DIR_Entry*)bSectorBuffer) + FScwd->uiSectorEntryPosition),(char*)&FScwd->DirEntry, sizeof(FileSystem_DIR_Entry)) ;

	RETURN_IF_NOT(bStatus,
					Directory_RawWrite(FSMountInfo, FScwd->uiSectorNo, FScwd->uiSectorNo + 1, bSectorBuffer),
					Directory_SUCCESS) ;

	return Directory_SUCCESS ;
}

void
Directory_PopulateDirEntry(FileSystem_DIR_Entry* dirEntry, char* szDirName, unsigned short usDirAttribute, unsigned uiParentSecNo, byte bParentSecPos)
{
	strcpy(dirEntry->Name, szDirName) ;
	
	dirEntry->usAttribute = usDirAttribute ;
	
	//TODO: To be Populated after implemeting Timer Interrupt and Clock [START]
	dirEntry->CreatedTime = 0 ;
	dirEntry->AccessedTime = 0 ;
	dirEntry->ModifiedTime = 0 ;
	//TODO: To be Populated after implemeting Timer Interrupt and Clock [END]

	dirEntry->uiStartSectorID = EOC ;
	dirEntry->uiSize = 0 ;

	dirEntry->uiParentSecID = uiParentSecNo ;
	dirEntry->bParentSectorPos = bParentSecPos ;
	dirEntry->iUserID = 0 ;
}

byte
Directory_GetContent_SPIKE(FileSystem_MountInfo* FSMountInfo)
{
	FileSystem_DIR_Entry* currentWorkingDirectory = &(FSMountInfo->FScwd.DirEntry) ;

	byte bStatus ;
	char bSectorBuffer[512] ;

	unsigned uiCurrentSectorID ;
	unsigned uiScanDirCount ;
	unsigned uiSectorPosIndex ;

	FileSystem_DIR_Entry* curDir ;

	uiScanDirCount = 0 ;
	uiCurrentSectorID = currentWorkingDirectory->uiStartSectorID ;

	while(uiCurrentSectorID != EOC)
	{
		RETURN_IF_NOT(bStatus, Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						Directory_SUCCESS) ;

		for(uiSectorPosIndex = 0; uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR; uiSectorPosIndex++)
		{
			curDir = ((FileSystem_DIR_Entry*)bSectorBuffer) + uiSectorPosIndex ;

			if(curDir->usAttribute & ATTR_DELETED_DIR)
			{
				continue ;
			}
			else
			{
				printf("\n %u: %s", uiScanDirCount, curDir->Name) ;

				uiScanDirCount++ ;
				if(uiScanDirCount >= currentWorkingDirectory->uiSize)
					return Directory_SUCCESS ;
			}
		}

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiCurrentSectorID),
						FileSystem_SUCCESS) ;
	}

	printf("\n") ;
	return Directory_SUCCESS ;
}

byte
Directory_FindDirectory(FileSystem_MountInfo* FSMountInfo, char* szDirName, 
			unsigned* uiSectorNo, unsigned* uiSectorPos, byte* bIsPresent, char* bDestSectorBuffer)
{
	FileSystem_DIR_Entry* currentWorkingDirectory = &(FSMountInfo->FScwd.DirEntry) ;

	byte bStatus ;
	char bSectorBuffer[512] ;
	byte bDeletedEntryFound ;
	unsigned uiCurrentSectorID ;
	unsigned uiNextSectorID ;
	unsigned uiScanDirCount ;
	unsigned uiSectorPosIndex ;

	FileSystem_DIR_Entry* curDir ;

	*uiSectorNo = EOC ;
	*uiSectorPos = EOC ;
	*bIsPresent = FALSE ;
	bDeletedEntryFound = FALSE ;
	uiScanDirCount = 0 ;

	uiCurrentSectorID = currentWorkingDirectory->uiStartSectorID ;

	while(uiCurrentSectorID != EOC)
	{
		RETURN_IF_NOT(bStatus, Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						Directory_SUCCESS) ;

		for(uiSectorPosIndex = 0; uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR; uiSectorPosIndex++)
		{
			curDir = ((FileSystem_DIR_Entry*)bSectorBuffer) + uiSectorPosIndex ;
			
			if(strcmp(szDirName, curDir->Name) == 0 && !(curDir->usAttribute & ATTR_DELETED_DIR))
			{
				memcpy( (char*)bDestSectorBuffer,(char*)&bSectorBuffer, 512) ;
				*uiSectorNo = uiCurrentSectorID ;
				*uiSectorPos = uiSectorPosIndex ;
				*bIsPresent = TRUE ;
				
				return Directory_SUCCESS ;
			}

			if((curDir->usAttribute & ATTR_DELETED_DIR) && bDeletedEntryFound == FALSE)
			{
				memcpy( (char*)bDestSectorBuffer,(char*)&bSectorBuffer, 512) ;
				*uiSectorNo = uiCurrentSectorID ;
				*uiSectorPos = uiSectorPosIndex ;

				bDeletedEntryFound = TRUE ;
			}
			else
			{
				uiScanDirCount++ ;
				if(uiScanDirCount >= currentWorkingDirectory->uiSize)
					break ;
			}
		}

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
						FileSystem_SUCCESS) ;

		if(uiScanDirCount >= currentWorkingDirectory->uiSize)
		{
			if(bDeletedEntryFound == TRUE)
				return Directory_SUCCESS ;

			if(uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR - 1)
			{
				memcpy( (char*)bDestSectorBuffer,(char*)&bSectorBuffer, 512) ;
				*uiSectorNo = uiCurrentSectorID ;
				*uiSectorPos = uiSectorPosIndex + 1 ;

				return Directory_SUCCESS ;
			}

			if(uiSectorPosIndex == DIR_ENTRIES_PER_SECTOR - 1)
			{
				if(uiNextSectorID != EOC)
				{
					*uiSectorNo = uiNextSectorID ;
					*uiSectorPos = 0 ;
	
					return Directory_SUCCESS ;
				}
				
				*uiSectorNo = uiCurrentSectorID ;
				*uiSectorPos = EOC ;

				return Directory_SUCCESS ;
			}
		}
		uiCurrentSectorID = uiNextSectorID ;
	}

	return Directory_ERR_FS_TABLE_CORRUPTED ;
}

byte
Directory_RawRead(FileSystem_MountInfo* FSMountInfo, unsigned uiStartSectorID, unsigned uiEndSectorID, char* bSectorBuffer)
{
	uiStartSectorID = FileSystem_GetRealSectorNumber(uiStartSectorID, &(FSMountInfo->FSBootBlock)) ;
	uiEndSectorID = FileSystem_GetRealSectorNumber(uiEndSectorID, &(FSMountInfo->FSBootBlock)) ;

	if(LSEEK(fd_DiskImageFile, uiStartSectorID * 512, SEEK_SET) < 0)
	{
		PUT_ERROR_MSG("LSEEK") ;
		return EXTERNAL_ERROR ;
	}
	
	if(read(fd_DiskImageFile, bSectorBuffer, 512) < 0)
	{
		PUT_ERROR_MSG("READ") ;
		return EXTERNAL_ERROR ;
	}
	
	return Directory_SUCCESS ;
}

byte
Directory_RawWrite(FileSystem_MountInfo* FSMountInfo, unsigned uiStartSectorID, unsigned uiEndSectorID, char* bSectorBuffer)
{
	uiStartSectorID = FileSystem_GetRealSectorNumber(uiStartSectorID, &(FSMountInfo->FSBootBlock)) ;
	uiEndSectorID = FileSystem_GetRealSectorNumber(uiEndSectorID, &(FSMountInfo->FSBootBlock)) ;

	if(LSEEK(fd_DiskImageFile, uiStartSectorID * 512, SEEK_SET) < 0)
	{
		PUT_ERROR_MSG("LSEEK") ;
		return EXTERNAL_ERROR ;
	}

	if(write(fd_DiskImageFile, bSectorBuffer, 512) < 0)
	{
		PUT_ERROR_MSG("WRITE") ;
		return EXTERNAL_ERROR ;
	}

	return Directory_SUCCESS ;
}

byte
Directory_FileWrite(FileSystem_MountInfo* FSMountInfo, char* szFileName, char* bDataBuffer, 
							unsigned uiOffset, unsigned uiDataSize)
{
	if(uiDataSize == 0)
		return Directory_ERR_ZERO_WRITE_SIZE ;

	byte bStatus ;
	unsigned uiSectorNo ;
	unsigned uiSectorPos ;
	byte bIsPresent ;
	char bDirectoryBuffer[512] ;

	RETURN_IF_NOT(bStatus, 
			Directory_FindDirectory(FSMountInfo, szFileName, &uiSectorNo, &uiSectorPos, &bIsPresent, bDirectoryBuffer),
			Directory_SUCCESS) ;

	if(bIsPresent == FALSE)
		return Directory_ERR_NOT_EXISTS ;

	FileSystem_DIR_Entry* dirFile = ((FileSystem_DIR_Entry*)bDirectoryBuffer) + uiSectorPos ;

	if((dirFile->usAttribute & ATTR_TYPE_DIRECTORY) != 0)
		return Directory_ERR_IS_DIRECTORY ;

	RETURN_IF_NOT(bStatus, Directory_ActualFileWrite(FSMountInfo, bDataBuffer, uiOffset, uiDataSize, dirFile), Directory_SUCCESS) ;

	if(dirFile->uiSize < (uiOffset + uiDataSize))
	{
		dirFile->uiSize = uiOffset + uiDataSize ;

		RETURN_IF_NOT(bStatus, Directory_RawWrite(FSMountInfo, uiSectorNo, uiSectorNo + 1, bDirectoryBuffer),
						Directory_SUCCESS) ;
	}

	return Directory_SUCCESS ;
}

byte
Directory_ActualFileWrite(FileSystem_MountInfo* FSMountInfo, char* bDataBuffer, unsigned uiOffset,
					unsigned uiDataSize, FileSystem_DIR_Entry* dirFile)
{
	if(uiOffset > dirFile->uiSize)
		return Directory_ERR_INVALID_OFFSET ;
		
	unsigned uiStartWriteSectorNo, uiStartWriteSectorPos ;
	unsigned uiCurrentSectorID, uiNextSectorID, uiPrevSectorID ;
	unsigned uiSectorIndex ;
	unsigned uiWriteRemainingCount, uiWrittenCount ;
	unsigned uiCurrentFileSize ;

	byte bStatus ; 
	byte bStartAllocation ;
	char bSectorBuffer[512] ;

	uiStartWriteSectorNo = uiOffset / 512 ;
	uiStartWriteSectorPos = uiOffset % 512 ;

	uiPrevSectorID = uiCurrentSectorID = dirFile->uiStartSectorID ;
	uiCurrentFileSize = dirFile->uiSize ;
	uiSectorIndex = 0 ;
	
	while(uiSectorIndex != uiStartWriteSectorNo)
	{
		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
						FileSystem_SUCCESS) ;

		uiSectorIndex++ ;
		uiPrevSectorID = uiCurrentSectorID ;
		uiCurrentSectorID = uiNextSectorID ;
	}

	if(uiCurrentSectorID == EOC)
	{
		RETURN_IF_NOT(bStatus, FileSystem_AllocateSector(FSMountInfo, &uiCurrentSectorID), FileSystem_SUCCESS) ;
		
		if(uiCurrentSectorID == 1)
		{
			printf("\n Alloc Sector = 1!!");
			while(1);
		}

		if(dirFile->uiStartSectorID == EOC)
		{
			dirFile->uiStartSectorID = uiCurrentSectorID ;
		}
		else
		{
			RETURN_IF_NOT(bStatus, FileSystem_SetSectorEntryValue(FSMountInfo, uiPrevSectorID, uiCurrentSectorID), FileSystem_SUCCESS) ;
		}
	}
	
	uiWrittenCount = 0 ;
	uiWriteRemainingCount = uiDataSize ;

	if(uiStartWriteSectorPos != 0)
	{
		RETURN_IF_NOT(bStatus, Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						Directory_SUCCESS) ;

		uiWrittenCount = 512 - uiStartWriteSectorPos ;
		if(uiDataSize <= uiWrittenCount)
			uiWrittenCount = uiDataSize ;

		memcpy((char*)(bSectorBuffer + uiStartWriteSectorPos), (char*)bDataBuffer, uiWrittenCount) ;

		RETURN_IF_NOT(bStatus,
					Directory_RawWrite(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
					Directory_SUCCESS) ;
			
		if(uiWrittenCount == uiDataSize)
			return Directory_SUCCESS ;

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
					FileSystem_SUCCESS) ;

		uiPrevSectorID = uiCurrentSectorID ;
		uiCurrentSectorID = uiNextSectorID ;

		uiWriteRemainingCount -= uiWrittenCount ;
	}

	bStartAllocation = FALSE ;
	
	while(TRUE)
	{
		if(uiCurrentSectorID == EOC || bStartAllocation == TRUE)
		{
			bStartAllocation = TRUE ;
			RETURN_IF_NOT(bStatus, FileSystem_AllocateSector(FSMountInfo, &uiCurrentSectorID), FileSystem_SUCCESS) ;
			RETURN_IF_NOT(bStatus, FileSystem_SetSectorEntryValue(FSMountInfo, uiPrevSectorID, uiCurrentSectorID), FileSystem_SUCCESS) ;
		}
		
		if(uiWriteRemainingCount < 512)
		{
			if(bStartAllocation == FALSE && (uiOffset + uiDataSize) < uiCurrentFileSize)
			{
				RETURN_IF_NOT(bStatus, 
							Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
							FileSystem_SUCCESS) ;
			}

			memcpy((char*)&bSectorBuffer, (char*)(bDataBuffer + uiWrittenCount), uiWriteRemainingCount) ;
							
			RETURN_IF_NOT(bStatus, 
						Directory_RawWrite(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						Directory_SUCCESS) ;
		
			return Directory_SUCCESS ;
		}

		RETURN_IF_NOT(bStatus,
				Directory_RawWrite(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bDataBuffer + uiWrittenCount),
				Directory_SUCCESS) ;
		
		uiWrittenCount += 512 ;
		uiWriteRemainingCount -= 512 ;

		if(uiWriteRemainingCount == 0)
			return Directory_SUCCESS ;

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID), FileSystem_SUCCESS) ;

		uiPrevSectorID = uiCurrentSectorID ;
		uiCurrentSectorID = uiNextSectorID ;
	}

	return Directory_ERR_FS_TABLE_CORRUPTED ;
}

byte
Directory_FileRead(FileSystem_MountInfo* FSMountInfo, char* szFileName, char* bDataBuffer, 
						unsigned uiOffset, unsigned uiDataSize, unsigned* uiReadFileSize)
{
	*uiReadFileSize = 0 ;
	if(uiDataSize == 0)
		return Directory_ERR_ZERO_WRITE_SIZE ;

	byte bStatus ;
	unsigned uiSectorNo ;
	unsigned uiSectorPos ;
	byte bIsPresent ;
	char bDirectoryBuffer[512] ;

	RETURN_IF_NOT(bStatus, 
			Directory_FindDirectory(FSMountInfo, szFileName, &uiSectorNo, &uiSectorPos, &bIsPresent, bDirectoryBuffer),
			Directory_SUCCESS) ;

	if(bIsPresent == FALSE)
		return Directory_ERR_NOT_EXISTS ;

	FileSystem_DIR_Entry* dirFile = ((FileSystem_DIR_Entry*)bDirectoryBuffer) + uiSectorPos ;

	if((dirFile->usAttribute & ATTR_TYPE_DIRECTORY) != 0)
		return Directory_ERR_IS_DIRECTORY ;

//**************************************************************

	if(uiOffset >= dirFile->uiSize)
		return Directory_ERR_EOF ;
		
	unsigned uiStartReadSectorNo, uiStartReadSectorPos ;
	unsigned uiCurrentSectorID, uiNextSectorID ;
	unsigned uiSectorIndex ;
	unsigned uiReadRemainingCount, uiReadCount ;
	unsigned uiCurrentFileSize ;

	char bSectorBuffer[512] ;

	uiStartReadSectorNo = uiOffset / 512 ;
	uiStartReadSectorPos = uiOffset % 512 ;

	uiCurrentSectorID = dirFile->uiStartSectorID ;
	uiCurrentFileSize = dirFile->uiSize ;
	uiSectorIndex = 0 ;
	
	while(uiSectorIndex != uiStartReadSectorNo)
	{
		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
						FileSystem_SUCCESS) ;

		uiSectorIndex++ ;
		uiCurrentSectorID = uiNextSectorID ;
	}

	uiReadCount = 0 ;
	uiReadRemainingCount = (uiDataSize < (uiCurrentFileSize - uiOffset) && uiDataSize > 0) ? uiDataSize : (uiCurrentFileSize  - uiOffset) ;

	if(uiStartReadSectorPos != 0)
	{
		RETURN_IF_NOT(bStatus, Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						Directory_SUCCESS) ;

		uiReadCount = 512 - uiStartReadSectorPos ;
		if(uiDataSize <= uiReadCount)
			uiReadCount = uiDataSize ;

		memcpy((char*)bDataBuffer, (char*)(bSectorBuffer + uiStartReadSectorPos), uiReadCount) ;

		if(uiReadCount == uiDataSize)
		{
			*uiReadFileSize = uiReadCount ;
			return Directory_SUCCESS ;
		}
		
		uiReadRemainingCount -= uiReadCount ;

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
						FileSystem_SUCCESS) ;

		uiCurrentSectorID = uiNextSectorID ;
	}

	while(TRUE)
	{
		if(uiCurrentSectorID == EOC)
		{
			*uiReadFileSize = uiReadCount ;
			return Directory_SUCCESS ;
		}
		
		if(uiReadRemainingCount < 512)
		{
			RETURN_IF_NOT(bStatus, 
						Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bSectorBuffer),
						FileSystem_SUCCESS) ;

			memcpy((char*)(bDataBuffer + uiReadCount), (char*)&bSectorBuffer, uiReadRemainingCount) ;

			*uiReadFileSize = uiReadCount + uiReadRemainingCount ;
			return Directory_SUCCESS ;
		}

		RETURN_IF_NOT(bStatus,
				Directory_RawRead(FSMountInfo, uiCurrentSectorID, uiCurrentSectorID + 1, bDataBuffer + uiReadCount),
				Directory_SUCCESS) ;
		
		uiReadCount += 512 ;
		uiReadRemainingCount -= 512 ;
		*uiReadFileSize = uiReadCount ;

		if(uiReadRemainingCount == 0)
			return Directory_SUCCESS ;

		RETURN_IF_NOT(bStatus, FileSystem_GetSectorEntryValue(FSMountInfo, uiCurrentSectorID, &uiNextSectorID),
					FileSystem_SUCCESS) ;

		uiCurrentSectorID = uiNextSectorID ;
	}

	return Directory_ERR_FS_TABLE_CORRUPTED ;
}

byte Directory_Change(FileSystem_MountInfo* FSMountInfo, char* szDirName)
{
	byte bStatus ;
	unsigned uiSectorNo, uiSectorPos ;
	byte bIsPresent ;
	byte bDirectoryBuffer[512] ;

	RETURN_IF_NOT(bStatus, 
			Directory_FindDirectory(FSMountInfo, szDirName, &uiSectorNo, &uiSectorPos, &bIsPresent,
			 bDirectoryBuffer), Directory_SUCCESS) ;

	if(bIsPresent == FALSE)
		return Directory_ERR_NOT_EXISTS ;

	FileSystem_DIR_Entry* dirFile = ((FileSystem_DIR_Entry*)bDirectoryBuffer) + uiSectorPos ;

	if((dirFile->usAttribute & ATTR_TYPE_DIRECTORY) == 0)
		return Directory_ERR_IS_NOT_DIRECTORY ;

	memcpy((char*)&(FSMountInfo->FScwd.DirEntry), (char*)(dirFile), sizeof(FileSystem_DIR_Entry)) ;
	FSMountInfo->FScwd.uiSectorNo = uiSectorNo ;
	FSMountInfo->FScwd.uiSectorEntryPosition = uiSectorPos ;

	return Directory_SUCCESS ;
}

