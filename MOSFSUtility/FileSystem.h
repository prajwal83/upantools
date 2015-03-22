#ifndef _FileSystem_H_
#define _FileSystem_H_

# include "Common.h"

#define FileSystem_SUCCESS								0
#define FileSystem_ERR_INVALID_BPB_SIGNATURE			1
#define FileSystem_ERR_INVALID_BOOT_SIGNATURE			2
#define FileSystem_ERR_INVALID_SECTORS_PER_TRACK		3
#define FileSystem_ERR_INVALID_NO_OF_HEADS				4
#define FileSystem_ERR_INVALID_DRIVE_SIZE_IN_SECTORS	5
#define FileSystem_ERR_INVALID_CLUSTER_ID				6
#define FileSystem_ERR_ZERO_FATSZ32						7
#define FileSystem_ERR_UNKNOWN_DEVICE_TYPE				8
#define FileSystem_ERR_UNSUPPORTED_SECTOR_SIZE			9
#define FileSystem_ERR_NO_FREE_CLUSTER					10
#define FileSystem_ERR_BPB_JMP							11
#define FileSystem_ERR_UNSUPPORTED_MEDIA				12
#define FileSystem_ERR_INVALID_EXTFLAG					13
#define FileSystem_ERR_FS_VERSION						14
#define FileSystem_ERR_FSINFO_SECTOR					15
#define FileSystem_ERR_INVALID_VOL_ID					16
#define FileSystem_ERR_ALREADY_MOUNTED					17
#define FileSystem_ERR_NOT_MOUNTED						18

#define MEDIA_REMOVABLE	0xF0
#define MEDIA_FIXED		0xF8

#define ATTR_DIR_DEFAULT 0x01ED  //0000 0001 1110 1101 => 0000(Rsv) 000(Dir) 111(u:rwx) 101(g:r-x) 101(o:r-x)
#define ATTR_FILE_DEFAULT 0x03A4  //0000(Rsv) 001(File) 110(u:rw-) 100(g:r--) 100(o:r--)
#define ATTR_DELETED_DIR 0x1000
#define ATTR_TYPE_DIRECTORY 0x2000
#define ATTR_TYPE_FILE		0x4000

#define EOC	0x0FFFFFFF
#define DIR_ENTRIES_PER_SECTOR 7

typedef enum
{
	FS_MOUNT,
	FS_UNMOUNT
} MOUNT_TYPE ;

typedef struct
{
	byte			BPB_jmpBoot[3] ;
	
	byte			BPB_Media ;
	unsigned short	BPB_SecPerTrk ;
	unsigned short	BPB_NumHeads ;

	unsigned short	BPB_BytesPerSec ;
	unsigned		BPB_TotSec32 ;
	unsigned		BPB_HiddSec ;
	
	unsigned short	BPB_RsvdSecCnt ;
	unsigned		BPB_FSTableSize ;
	
	unsigned short	BPB_ExtFlags ;
	unsigned short	BPB_FSVer ;
	unsigned short	BPB_FSInfo ;
	
	byte			BPB_BootSig ;
	unsigned		BPB_VolID ;
	byte			BPB_VolLab[11 + 1] ;

	unsigned		uiUsedSectors ;
} PACKED FileSystem_BootBlock ;

typedef struct
{
	unsigned	FSI_LeadSig ;  //0x41615252
	byte		FSI_Reserved1[480] ;
	unsigned	FSI_StrucSig ;  //0x61417272
	unsigned	FSI_FreeCount ;
	unsigned	FSI_NxtFree ;
	byte		FSI_Reserved2[12] ;
	unsigned	FSI_TrailSig ;  //0xAA550000
} PACKED FileSystem_Info ;

typedef struct
{
	byte			Name[33] ;
	unsigned		CreatedTime ;
	unsigned		AccessedTime ;
	unsigned		ModifiedTime ;
	byte			bParentSectorPos ;
	unsigned short	usAttribute ;
	unsigned		uiSize ;
	unsigned		uiStartSectorID ;
	unsigned		uiParentSecID ;
    int             iUserID ;
} PACKED FileSystem_DIR_Entry ;

typedef struct
{
	FileSystem_DIR_Entry DirEntry ;
	unsigned uiSectorNo ;
	unsigned uiSectorEntryPosition ;
} PACKED FileSystem_CurrentWorkingDirectory ;

typedef struct
{
	FileSystem_BootBlock FSBootBlock ; 
	FileSystem_Info FSInfo ;
	FileSystem_CurrentWorkingDirectory FScwd ;
	byte bMounted ;
} PACKED FileSystem_MountInfo ;

unsigned FileSystem_GetRealSectorNumber(const unsigned uiSectorID, const FileSystem_BootBlock* FSBootBlock) ;

byte FileSystem_GetSectorEntryValue(const FileSystem_MountInfo* FSMountInfo, const unsigned uiSectorID,
		unsigned* uiSectorEntryValue) ;
		
byte FileSystem_SetSectorEntryValue(const FileSystem_MountInfo* FSMountInfo, const unsigned uiSectorID,
		unsigned uiSectorEntryValue) ;

byte FileSystem_GetFSBootBlock(FileSystem_MountInfo* FSMountInfo) ;
byte FileSystem_FSTableMounter(FileSystem_MountInfo* FSMountInfo, MOUNT_TYPE mountType) ;
byte FileSystem_ReadRootDirectory(FileSystem_MountInfo* FSMountInfo) ;
byte FileSystem_Mount(FileSystem_MountInfo* FSMountInfo) ;
byte FileSystem_UnMount(FileSystem_MountInfo* FSMountInfo) ;
byte FileSystem_AllocateSector(FileSystem_MountInfo* FSMountInfo, unsigned* uiFreeSectorID) ;

void FileSystem_Initialize() ;

FileSystem_MountInfo FSMountInfo_FD1 ;
char* FSMountBuffer ;

#endif

