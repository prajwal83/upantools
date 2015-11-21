#ifndef _UPAN_FS_H_
#define _UPAN_FS_H_

#define ATTR_DELETED_DIR 0x1000
#define ATTR_TYPE_DIRECTORY 0x2000

#define EOC 0x0FFFFFFF
#define DIR_ENTRIES_PER_SECTOR 7

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

struct timeval_mos
{
	unsigned tSec ;
} PACKED ;

typedef struct
{
  byte      Name[33];
  struct timeval_mos  CreatedTime;
  struct timeval_mos  AccessedTime;
  struct timeval_mos  ModifiedTime;
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
  upanfs_cwd fs_cwd;
} PACKED upanfs_mount_block;

#endif
