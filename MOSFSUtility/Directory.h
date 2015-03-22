#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

# include "Common.h"
# include "FileSystem.h"

#define Directory_SUCCESS					0
#define Directory_ERR_UNKNOWN_DEVICE		1
#define Directory_ERR_FS_TABLE_CORRUPTED	2
#define Directory_ERR_EXISTS				3
#define Directory_ERR_NOT_EXISTS			4
#define Directory_ERR_ZERO_WRITE_SIZE		5
#define Directory_ERR_NOT_EMPTY				6
#define Directory_ERR_INVALID_OFFSET		7
#define Directory_ERR_IS_DIRECTORY			8
#define Directory_ERR_EOF					9
#define Directory_ERR_IS_NOT_DIRECTORY		10

byte Directory_GetContent_SPIKE(FileSystem_MountInfo* FSMountInfo) ;
void Directory_PopulateDirEntry(FileSystem_DIR_Entry* dirEntry, char* szDirName, unsigned short usDirAttribute, unsigned uiParentSecNo, byte bParentSectorPos) ;
byte Directory_Create(FileSystem_MountInfo* FSMountInfo, char* szDirName, unsigned short usDirAttribute) ;

byte Directory_FindDirectory(FileSystem_MountInfo* FSMountInfo, char* szDirName, unsigned* uiSectorNo, 
							unsigned* uiSectorPos, byte* bIsPresent, char* bDestSectorBuffer) ;

byte Directory_FileWrite(FileSystem_MountInfo* FSMountInfo, char* szFileName, char* bDataBuffer, 
							unsigned uiOffset, unsigned uiDataSize) ;
byte Directory_ActualFileWrite(FileSystem_MountInfo* FSMountInfo, char* bDataBuffer, unsigned uiOffset,
							unsigned uiDataSize, FileSystem_DIR_Entry* dirFile) ;

byte
Directory_FileRead(FileSystem_MountInfo* FSMountInfo, char* szFileName, char* bDataBuffer, 
						unsigned uiOffset, unsigned uiDataSize, unsigned* uiReadFileSize) ;
			
byte Directory_RawRead(FileSystem_MountInfo* FSMountInfo, unsigned uiStartSectorID, unsigned uiEndSectorID, 
						char* bSectorBuffer) ;
byte Directory_RawWrite(FileSystem_MountInfo* FSMountInfo, unsigned uiStartSectorID, unsigned uiEndSectorID, 
						char* bSectorBuffer) ;
byte Directory_Change(FileSystem_MountInfo* FSMountInfo, char* szDirName) ;

#endif
