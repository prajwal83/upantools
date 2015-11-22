#ifndef _UPAN_FS_H_
#define _UPAN_FS_H_

#define ATTR_DIR_DEFAULT	0x01ED  //0000 0001 1110 1101 => 0000(Rsv) 000(Dir) 111(u:rwx) 101(g:r-x) 101(o:r-x)
#define ATTR_FILE_DEFAULT	0x03A4  //0000(Rsv) 001(File) 110(u:rw-) 100(g:r--) 100(o:r--)
#define ATTR_DELETED_DIR 0x1000
#define ATTR_TYPE_DIRECTORY 0x2000

#define EOC 0x0FFFFFFF
#define DIR_ENTRIES_PER_SECTOR 7

#define ENTRIES_PER_TABLE_SECTOR	(128)
#define FSTABLE_BLOCK_ID(SectorID) (SectorID / ENTRIES_PER_TABLE_SECTOR)
#define FSTABLE_BLOCK_OFFSET(SectorID) (SectorID % ENTRIES_PER_TABLE_SECTOR) 

#define PACKED __attribute__((packed))

typedef unsigned char byte;

//typedef enum bool { false, true };

typedef struct
{ 
  byte           BPB_jmpBoot[3];
  byte           BPB_Media;
  unsigned short BPB_SecPerTrk;
  unsigned short BPB_NumHeads;
  unsigned short BPB_BytesPerSec;
  unsigned       BPB_TotSec32;
  unsigned       BPB_HiddSec;
  unsigned short BPB_RsvdSecCnt;
  unsigned       BPB_FSTableSize;
  unsigned short BPB_ExtFlags;
  unsigned short BPB_FSVer;
  unsigned short BPB_FSInfo;
  byte           BPB_BootSig;
  unsigned       BPB_VolID;
  byte           BPB_VolLab[11 + 1];
  unsigned       uiUsedSectors;
} PACKED upanfs_boot_block;

typedef struct
{
	unsigned tSec;
} PACKED timeval_upanfs;

typedef struct
{
  byte      Name[33];
  timeval_upanfs  CreatedTime;
  timeval_upanfs  AccessedTime;
  timeval_upanfs  ModifiedTime;
  byte      bParentSectorPos; 
  unsigned short  usAttribute;
  unsigned    uiSize;
  unsigned    uiStartSectorID;
  unsigned    uiParentSecID;
  int       iUserID;
} PACKED upanfs_dir_entry;

typedef struct
{
  upanfs_dir_entry dir_entry;
  unsigned uiSectorNo;
  unsigned uiSectorEntryPosition;
} PACKED upanfs_cwd;

typedef struct
{
  byte* fstable;
  upanfs_boot_block bpb;
} PACKED upanfs_mount_block;

#endif
