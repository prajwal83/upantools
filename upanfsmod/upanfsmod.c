#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
//#include <linux/namei.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
//#include <linux/random.h>
#include <linux/version.h>
//#include <linux/jbd2.h>
//#include <linux/parser.h>
//#include <linux/blkdev.h>
#include "upanfsmod.h"

static struct dentry* upanfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);
static void upanfs_umount(struct super_block *sb);
static void upanfs_destory_inode(struct inode *inode);
static void upanfs_put_super(struct super_block *sb);
static struct dentry *upanfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static int upanfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static int upanfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
static int upanfs_rmdir(struct inode *dir, struct dentry *dentry);
static int upanfs_delete(struct inode *dir, struct dentry *dentry);
static int upanfs_iterate(struct file *filp, struct dir_context *ctx);
static ssize_t upanfs_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos);
static ssize_t upanfs_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos);

static struct file_system_type upanfs_type = {
  .owner = THIS_MODULE,
  .name = "upanfs",
  .mount = upanfs_mount,
  .kill_sb = upanfs_umount,
  .fs_flags = FS_REQUIRES_DEV, 
};

static const struct super_operations upanfs_sops = {
  .destroy_inode = upanfs_destory_inode,
  .put_super = upanfs_put_super,
};

static struct inode_operations upanfs_inode_ops = {
  .create = upanfs_create,
  .lookup = upanfs_lookup,
  .mkdir = upanfs_mkdir,
  .rmdir = upanfs_rmdir,
  .unlink = upanfs_delete,
};

const struct file_operations upanfs_dir_operations = {
  .owner = THIS_MODULE,
  .iterate = upanfs_iterate,
};

const struct file_operations upanfs_file_operations = {
  .read = upanfs_read,
  .write = upanfs_write,
};

static unsigned upanfs_get_sec_entry_val(struct super_block* sb, unsigned uiCurrentSectorID)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  return ((unsigned*)mblock->fstable)[uiCurrentSectorID] & EOC;
}

static unsigned upanfs_get_real_sec_num(unsigned uiSectorID, upanfs_mount_block* mblock)
{
	return uiSectorID + 1 + mblock->bpb.BPB_RsvdSecCnt + mblock->bpb.BPB_FSTableSize ;
}

static int sectors_per_block(struct super_block* sb)
{
  return sb->s_blocksize / 512;
}

static int sector_to_block(struct super_block* sb, int sector)
{
  return sector * 512 / sb->s_blocksize;
}

static int block_offset(struct super_block* sb, int sector)
{
  return (sector % sectors_per_block(sb)) * 512;
}

static struct timespec seconds_to_timespec(unsigned seconds)
{
  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = seconds * 1000;
  return ts;
}

static unsigned dir_size(unsigned fileCount)
{
  if(fileCount < DIR_ENTRIES_PER_SECTOR)
    return 512;
  return ((fileCount / DIR_ENTRIES_PER_SECTOR) + (fileCount % DIR_ENTRIES_PER_SECTOR ? 1 : 0)) * 512;
}

static unsigned sector_pos_cast(void* p)
{
  return (unsigned long)p;
}

static void* inode_private_cast(unsigned long p)
{
  return (void*)p;
}

static bool upanfs_read_dir_entry(struct super_block* sb, upanfs_dir_entry* dir_entry, unsigned sector, unsigned sector_pos)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  int offset;
  unsigned real_sector = upanfs_get_real_sec_num(sector, mblock);
  struct buffer_head* bh = sb_bread(sb, sector_to_block(sb, real_sector));
  if(!bh)
    return false;
  offset = block_offset(sb, real_sector);
  memcpy((byte*)dir_entry, bh->b_data + offset + sector_pos *  sizeof(upanfs_dir_entry), sizeof(upanfs_dir_entry));
  brelse(bh);
  return true;
}

static bool upanfs_read_sector(struct super_block* sb, unsigned sector, byte* buffer)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  int offset;
  unsigned real_sector = upanfs_get_real_sec_num(sector, mblock);
  struct buffer_head* bh = sb_bread(sb, sector_to_block(sb, real_sector));
  if(!bh)
    return false;
  offset = block_offset(sb, real_sector);
  memcpy(buffer, bh->b_data + offset, 512);
  brelse(bh);
  return true;
}

static bool upanfs_write_sector(struct super_block* sb, unsigned sector, byte* buffer)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  int offset;
  unsigned real_sector = upanfs_get_real_sec_num(sector, mblock);
  struct buffer_head* bh = sb_bread(sb, sector_to_block(sb, real_sector));
  if(!bh)
    return false;
  offset = block_offset(sb, real_sector);
  memcpy(bh->b_data + offset, buffer, 512);
  mark_buffer_dirty(bh);
  sync_dirty_buffer(bh);
  brelse(bh);
  return true;
}

static bool upanfs_write_dir_entry(struct super_block* sb, upanfs_dir_entry* dir_entry, unsigned sector, unsigned sector_pos)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  int offset;
  unsigned real_sector = upanfs_get_real_sec_num(sector, mblock);
  struct buffer_head* bh = sb_bread(sb, sector_to_block(sb, real_sector));
  if(!bh)
    return false;
  offset = block_offset(sb, real_sector);
  memcpy(bh->b_data + offset + sector_pos *  sizeof(upanfs_dir_entry), dir_entry, sizeof(upanfs_dir_entry));
  mark_buffer_dirty(bh);
  sync_dirty_buffer(bh);
  brelse(bh);
  return true;
}

static bool upanfs_set_sector_value(struct super_block* sb, unsigned uiSectorID, unsigned uiValue)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  struct buffer_head *bh;
  unsigned sector = mblock->bpb.BPB_RsvdSecCnt + 1 + FSTABLE_BLOCK_ID(uiSectorID);
  byte* fstable_block;

  ((unsigned*)mblock->fstable)[uiSectorID] = uiValue;

  if(!(bh = sb_bread(sb, sector_to_block(sb, sector))))
  {
    printk(KERN_ERR "failed to read fstable sector\n");
    return false;
  }

  fstable_block = bh->b_data + block_offset(sb, sector);
  ((unsigned*)fstable_block)[FSTABLE_BLOCK_OFFSET(uiSectorID)] = uiValue;
  
  mark_buffer_dirty(bh);
  sync_dirty_buffer(bh);
  brelse(bh);

  if(uiValue == EOC)
    mblock->bpb.uiUsedSectors++;
   else if(uiValue == 0)
    mblock->bpb.uiUsedSectors--;
  
  return true;
}

static bool upanfs_get_free_sector(struct super_block* sb, unsigned* uiSectorID)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  unsigned* fstable = (unsigned*)mblock->fstable;
  unsigned fstable_size = mblock->bpb.BPB_FSTableSize * ENTRIES_PER_TABLE_SECTOR;

  for(unsigned i = 0; i < fstable_size; ++i)
  {
    if(!(fstable[i] & EOC))
    {
      *uiSectorID = i;
      return true;
    }
  }
  return false;
}

static bool upanfs_allocate_sector(struct super_block* sb, unsigned* uiFreeSectorID)
{
  if(!upanfs_get_free_sector(sb, uiFreeSectorID))
    return false;

  if(!upanfs_set_sector_value(sb, *uiFreeSectorID, EOC))
    return false;

  return true;
}

static bool upanfs_deallocate_sector(struct super_block* sb, unsigned uiSectorID, unsigned* uiNextSectorID)
{
  *uiNextSectorID = upanfs_get_sec_entry_val(sb, uiSectorID);
  return upanfs_set_sector_value(sb, uiSectorID, 0);
}

static ssize_t upanfs_read(struct file * filp, char __user * buf, size_t len, loff_t* ppos)
{
  struct inode* finode = filp->f_path.dentry->d_inode;
  const char* fname = filp->f_path.dentry->d_name.name;
  struct super_block* sb = finode->i_sb;
  upanfs_dir_entry file_dir_entry;

	int iStartReadSectorNo, iStartReadSectorPos;
  size_t readCount, readRemainingCount;
  unsigned uiCurrentSectorID;

  byte sector_buffer[512];

  if(!upanfs_read_dir_entry(sb, &file_dir_entry, finode->i_ino, sector_pos_cast(finode->i_private)))
  {
    printk(KERN_ERR "failed to read dir entry for file: %s\n", fname);
    return -EFAULT;
  }
	
  if((file_dir_entry.usAttribute & ATTR_TYPE_DIRECTORY) != 0)
  {
    printk(KERN_ERR "%s is a directory - cannot read as a file\n", fname);
    return -EFAULT;
  }

  if(*ppos >= file_dir_entry.uiSize)
    return 0;

  iStartReadSectorNo = *ppos / 512;
  iStartReadSectorPos = *ppos % 512;
  uiCurrentSectorID = file_dir_entry.uiStartSectorID;
  for(int i = 0; i < iStartReadSectorNo; ++i)
    uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);

  readCount = 0;
	readRemainingCount = len < (file_dir_entry.uiSize - *ppos) ? len : (file_dir_entry.uiSize - *ppos);

	while(true)
	{
    size_t n;
		if(uiCurrentSectorID == EOC)
    {
      *ppos += readCount;
      return readCount;
    }
		
    if(!upanfs_read_sector(sb, uiCurrentSectorID, sector_buffer))
    {
      printk(KERN_ERR "failed to read sector: %u\n", uiCurrentSectorID);
      return -EFAULT;
    }

    n = 512 - iStartReadSectorPos;
    n = n < readRemainingCount ? n : readRemainingCount;

    if(copy_to_user(buf + readCount, sector_buffer + iStartReadSectorPos, n))
    {
      printk(KERN_ERR "error copying read buffer to user space for sector: %u, len: %zu", uiCurrentSectorID, n);
      return -EFAULT;
    }

    iStartReadSectorPos = 0;
    readCount += n;
    readRemainingCount -= n;

    if(readRemainingCount <= 0)
    {
      *ppos += readCount;
      return readCount;
    }

    uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
  }
}

static ssize_t upanfs_write(struct file * filp, const char __user * buf, size_t len, loff_t* ppos)
{
  struct inode* finode = filp->f_path.dentry->d_inode;
  const char* fname = filp->f_path.dentry->d_name.name;
  struct super_block* sb = finode->i_sb;
  upanfs_dir_entry file_dir_entry;

	unsigned uiCurrentSectorID, uiPrevSectorID = EOC;
	int iStartWriteSectorNo, iStartWriteSectorPos, iSectorIndex;
	size_t writeRemainingCount, writtenCount;
  byte sector_buffer[512];
  bool bStartAllocation;
  bool bDone = false;

  if(len == 0)
    return 0;

  if(!upanfs_read_dir_entry(sb, &file_dir_entry, finode->i_ino, sector_pos_cast(finode->i_private)))
  {
    printk(KERN_ERR "failed to read dir entry for file: %s\n", fname);
    return -EFAULT;
  }
	
  if((file_dir_entry.usAttribute & ATTR_TYPE_DIRECTORY) != 0)
  {
    printk(KERN_ERR "%s is a directory - cannot write as a file\n", fname);
    return -EFAULT;
  }

	iStartWriteSectorNo = *ppos / 512;
	iStartWriteSectorPos = *ppos % 512;

  uiCurrentSectorID = file_dir_entry.uiStartSectorID;
  iSectorIndex = 0;
  while(iSectorIndex < iStartWriteSectorNo && uiCurrentSectorID != EOC)
  {
    ++iSectorIndex;
    uiPrevSectorID = uiCurrentSectorID;
    uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
  }

	if(uiCurrentSectorID == EOC)
	{
		memset(sector_buffer, 0, 512);

		do
		{
      if(!upanfs_allocate_sector(sb, &uiCurrentSectorID))
      {
        printk(KERN_ERR "failed to allocate new sector while writing file: %s\n", fname);
        return -EFAULT;
      }

			if(file_dir_entry.uiStartSectorID == EOC)
			{
				file_dir_entry.uiStartSectorID = uiCurrentSectorID ;
			}
			else
			{
        if(!upanfs_set_sector_value(sb, uiPrevSectorID, uiCurrentSectorID))
        {
          printk(KERN_ERR "failed to set sector value for: %u\n", uiPrevSectorID);
          return -EFAULT;
        }
			}
			
			uiPrevSectorID = uiCurrentSectorID ;

      if(!upanfs_write_sector(sb, uiCurrentSectorID, sector_buffer))
      {
        printk(KERN_ERR "error writing to sector: %u\n", uiCurrentSectorID);
        return -EFAULT;
      }
			
      iSectorIndex++;

		} while(iSectorIndex <= iStartWriteSectorNo);
	}

	writtenCount = 0;
	writeRemainingCount = len;

	if(iStartWriteSectorPos != 0)
	{
    if(!upanfs_read_sector(sb, uiCurrentSectorID, sector_buffer))
    {
      printk(KERN_ERR "error reading sector: %u\n", uiCurrentSectorID);
      return -EFAULT;
    }

		writtenCount = 512 - iStartWriteSectorPos;
		if(len <= writtenCount)
			writtenCount = len;

    if(copy_from_user(sector_buffer + iStartWriteSectorPos, buf, writtenCount))
    {
      printk(KERN_ERR "error copying from user buffer for sector: %u", uiCurrentSectorID);
      return -EFAULT;
    }

    if(!upanfs_write_sector(sb, uiCurrentSectorID, sector_buffer))
    {
      printk(KERN_ERR "error writing sector: %u\n", uiCurrentSectorID);
      return -EFAULT;
    }
			
    if(writtenCount == len)
    {
      bDone = true;
    }
    else
    {
      uiPrevSectorID = uiCurrentSectorID;
      uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
      writeRemainingCount -= writtenCount;
    }
	}

	bStartAllocation = false ;
	
	while(!bDone)
	{
		if(uiCurrentSectorID == EOC || bStartAllocation == true)
		{
			bStartAllocation = true ;
      if(!upanfs_allocate_sector(sb, &uiCurrentSectorID))
      {
        printk(KERN_ERR "failed to allocate new sector\n");
        return -EFAULT;
      }

      if(!upanfs_set_sector_value(sb, uiPrevSectorID, uiCurrentSectorID))
      {
        printk(KERN_ERR "failed set sector value for: %u\n", uiPrevSectorID);
        return -EFAULT;
      }
		}
		
    if(writeRemainingCount < 512)
		{
			if(bStartAllocation == false && (*ppos + len) < file_dir_entry.uiSize)
			{
        if(!upanfs_read_sector(sb, uiCurrentSectorID, sector_buffer))
        {
          printk(KERN_ERR "failed to read sector: %u\n", uiCurrentSectorID);
          return -EFAULT;
        }
			}

      if(copy_from_user(sector_buffer, buf + writtenCount, writeRemainingCount))
      {
        printk(KERN_ERR "failed to copy from user buffer for sector: %u\n", uiCurrentSectorID);
        return -EFAULT;
      }

      if(!upanfs_write_sector(sb, uiCurrentSectorID, sector_buffer))
      {
        printk(KERN_ERR "failed to write sector: %u\n", uiCurrentSectorID);
        return -EFAULT;
      }

      writtenCount += writeRemainingCount;
      break;
		}

    if(copy_from_user(sector_buffer, buf + writtenCount, 512))
    {
      printk(KERN_ERR "failed to copy from user buffer for sector: %u\n", uiCurrentSectorID);
      return -EFAULT;
    }

    if(!upanfs_write_sector(sb, uiCurrentSectorID, sector_buffer))
    {
      printk(KERN_ERR "failed to write sector: %u\n", uiCurrentSectorID);
      return -EFAULT;
    }
		
		writtenCount += 512;
		writeRemainingCount -= 512;

		if(writeRemainingCount == 0)
      break;

		uiPrevSectorID = uiCurrentSectorID;
		uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
	}

  if(file_dir_entry.uiSize < (*ppos + len))
  {
    file_dir_entry.uiSize = *ppos + len;
    if(!upanfs_write_dir_entry(sb, &file_dir_entry, finode->i_ino, sector_pos_cast(finode->i_private)))
    {
      printk(KERN_ERR "failed to write dir-entry for file: %s\n", fname);
      return -EFAULT;
    }
  }
  finode->i_size = file_dir_entry.uiSize;
  *ppos += writtenCount;

  return writtenCount;
}

static void upanfs_destory_inode(struct inode *inode)
{
  // no need to implement this
}

static void upanfs_put_super(struct super_block *sb)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  if(mblock)
  {
    if(mblock->fstable)
      kfree(mblock->fstable);
    kfree(mblock);
  }
}

static struct dentry *upanfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
  struct super_block *sb = parent_inode->i_sb;
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  struct buffer_head *bh;
  unsigned uiScanDirCount = 0;
  unsigned uiCurrentSectorID;
  int status = 1; //1 = cont, 2 = done_success, 3 = done_failure
  upanfs_dir_entry parent;
  if(!upanfs_read_dir_entry(sb, &parent, parent_inode->i_ino, sector_pos_cast(parent_inode->i_private)))
    return NULL;
  uiCurrentSectorID = parent.uiStartSectorID;
  while(uiCurrentSectorID != EOC)
  {
    byte* dir_buffer;
    unsigned sector = upanfs_get_real_sec_num(uiCurrentSectorID, mblock);
    if(!(bh = sb_bread(sb, sector_to_block(sb, sector))))
    {
      printk(KERN_ERR "out of memory - failed to allocate bh");
      return NULL;
    }
    dir_buffer = bh->b_data + block_offset(sb, sector);
    for(unsigned uiSectorPosIndex = 0; uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR; uiSectorPosIndex++)
    {
      upanfs_dir_entry* cur_dir = ((upanfs_dir_entry*)dir_buffer) + uiSectorPosIndex ;
      if(strcmp(child_dentry->d_name.name, cur_dir->Name) == 0 && !(cur_dir->usAttribute & ATTR_DELETED_DIR))
      {
        bool isDir;
        struct inode *inode;
        inode = new_inode(sb);
        inode->i_ino = uiCurrentSectorID;
        inode->i_sb = sb;
        inode->i_op = &upanfs_inode_ops;

        isDir = cur_dir->usAttribute & ATTR_TYPE_DIRECTORY;
        if(isDir)
        {
          inode->i_fop = &upanfs_dir_operations;
          inode->i_size = dir_size(cur_dir->uiSize);
        }
        else
        {
          inode->i_fop = &upanfs_file_operations;
          inode->i_size = cur_dir->uiSize;
        }
        inode->i_atime = seconds_to_timespec(cur_dir->AccessedTime.tSec);
        inode->i_mtime = seconds_to_timespec(cur_dir->ModifiedTime.tSec);
        inode->i_ctime = seconds_to_timespec(cur_dir->CreatedTime.tSec);
        inode->i_private = inode_private_cast(uiSectorPosIndex);

        inode_init_owner(inode, parent_inode, isDir ? S_IFDIR | 0755 : S_IFREG | 0644);
        d_add(child_dentry, inode);
        status = 2;
        break;
      }

      if(!(cur_dir->usAttribute & ATTR_DELETED_DIR))
      {
        uiScanDirCount++;
        if(uiScanDirCount >= parent.uiSize)
        {
          status = 3;
          break;
        }
      }
    }
    brelse(bh);
    if(status == 1)
      uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
    else
      break;
  }
  if(status == 3)
    printk(KERN_INFO "file/directory [%s] not found\n", child_dentry->d_name.name);
  return NULL;
}

static int upanfs_create_dir_entry(struct inode* dir, struct dentry* dentry, umode_t mode)
{
  struct inode *newdir_inode;
	unsigned uiFreeSectorID;
  unsigned uiSectorNo;
  unsigned uiCurrentSectorID, uiPrevSectorID = EOC;
  unsigned uiSectorPos;
  unsigned uiScanDirCount = 0;
  bool found = false;
  struct timespec current_time = CURRENT_TIME;
  unsigned real_sector, offset;
  upanfs_dir_entry parent;
  upanfs_dir_entry* newdir;
  struct buffer_head* newdir_bh = NULL;
  struct super_block* sb = dir->i_sb;
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;

  if(!upanfs_read_dir_entry(sb, &parent, dir->i_ino, sector_pos_cast(dir->i_private)))
  {
    printk(KERN_ERR "failed to read parent dir at sector: %ld\n", dir->i_ino);
    return -EPERM;
  }

	if(parent.uiStartSectorID != EOC)
	{
    uiCurrentSectorID = parent.uiStartSectorID;
    while(uiCurrentSectorID != EOC)
    {
      byte* dir_buffer;
      unsigned sector = upanfs_get_real_sec_num(uiCurrentSectorID, mblock);
      if(!(newdir_bh = sb_bread(sb, sector_to_block(sb, sector))))
      {
        printk(KERN_ERR "out of memory - failed to allocate bh");
        return -ENOMEM;
      }
      dir_buffer = newdir_bh->b_data + block_offset(sb, sector);
      for(unsigned uiSectorPosIndex = 0; uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR; uiSectorPosIndex++)
      {
        upanfs_dir_entry* cur_dir = ((upanfs_dir_entry*)dir_buffer) + uiSectorPosIndex;
        if(cur_dir->usAttribute & ATTR_DELETED_DIR)
        {
          found = true;
          uiSectorNo = uiCurrentSectorID;
          uiSectorPos = uiSectorPosIndex;
          break;
        }
        uiScanDirCount++ ;
        if(uiScanDirCount >= parent.uiSize)
        {
          if(uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR - 1)
          {
            found = true;
            uiSectorNo = uiCurrentSectorID;
            uiSectorPos = uiSectorPosIndex + 1;
          }
          break;
        }
      }
      if(found)
        break;
      brelse(newdir_bh);
      newdir_bh = NULL;
      uiPrevSectorID = uiCurrentSectorID;
      uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
    }
	}
  if(!found)
  {
    unsigned sector;
    if(!upanfs_allocate_sector(sb, &uiFreeSectorID))
      return false;
    uiSectorNo = uiFreeSectorID;
    uiSectorPos = 0;
    sector = upanfs_get_real_sec_num(uiSectorNo, mblock);
    if(!(newdir_bh = sb_bread(sb, sector_to_block(sb, sector))))
    {
      printk(KERN_ERR "out of memory - failed to allocate bh");
      return -ENOMEM;
    }
    if(parent.uiStartSectorID == EOC)
      parent.uiStartSectorID = uiSectorNo;
    else if(uiPrevSectorID != EOC)
      upanfs_set_sector_value(sb, uiPrevSectorID, uiSectorNo);
  }
  if(newdir_bh == NULL)
  {
    printk(KERN_ERR "failed to create new dir: %s\n", dentry->d_name.name);
    return -EPERM;
  }
  real_sector = upanfs_get_real_sec_num(uiSectorNo, mblock);
  offset = block_offset(sb, real_sector);
  newdir = (upanfs_dir_entry*)(newdir_bh->b_data + offset + uiSectorPos * sizeof(upanfs_dir_entry));
  strcpy(newdir->Name, dentry->d_name.name);

  if(S_ISDIR(mode))
    newdir->usAttribute = ATTR_DIR_DEFAULT | ATTR_TYPE_DIRECTORY;
  else
    newdir->usAttribute = ATTR_FILE_DEFAULT | ATTR_TYPE_FILE;
  newdir->CreatedTime.tSec = newdir->AccessedTime.tSec = newdir->ModifiedTime.tSec = current_time.tv_sec;
	newdir->uiStartSectorID = EOC;
	newdir->uiSize = 0;
	newdir->uiParentSecID = dir->i_ino;
  newdir->bParentSectorPos = (byte)sector_pos_cast(dir->i_private);
	newdir->iUserID = 0;

  newdir_inode = new_inode(sb);
  if (!newdir_inode) 
  {
    printk(KERN_ERR "failed to create new inode for: %s\n", dentry->d_name.name);
    brelse(newdir_bh);
    return -ENOMEM;
  }

  newdir_inode->i_sb = sb;
  newdir_inode->i_op = &upanfs_inode_ops;
  if(S_ISDIR(mode))
    newdir_inode->i_fop = &upanfs_dir_operations;
  else
    newdir_inode->i_fop = &upanfs_file_operations;
  newdir_inode->i_atime = newdir_inode->i_mtime = newdir_inode->i_ctime = current_time;
  newdir_inode->i_ino = uiSectorNo;
  newdir_inode->i_private = inode_private_cast(uiSectorPos);
  newdir_inode->i_size = 512;

  mark_buffer_dirty(newdir_bh);
  sync_dirty_buffer(newdir_bh);
  brelse(newdir_bh);

  parent.uiSize++;
  if(!upanfs_write_dir_entry(sb, &parent, dir->i_ino, sector_pos_cast(dir->i_private)))
  {
    printk(KERN_ERR "failed to write parent directory: %s\n", parent.Name);
    return -EPERM;
  }
  
  dir->i_size = dir_size(parent.uiSize);
  inode_init_owner(newdir_inode, dir, mode);
  d_add(dentry, newdir_inode);

  return 0;
}

static int upanfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
  return upanfs_create_dir_entry(dir, dentry, mode);
}

static int upanfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
  return upanfs_create_dir_entry(dir, dentry, S_IFDIR | mode);
}

static int upanfs_delete_dir_entry(struct inode* dir, struct dentry* dentry)
{
  const char* fname = dentry->d_name.name;
  struct super_block* sb = dir->i_sb;
  struct inode* file_inode = dentry->d_inode;
  upanfs_dir_entry file_dir_entry;
  upanfs_dir_entry parent;
  unsigned uiCurrentSectorID, uiNextSectorID;

  if(!upanfs_read_dir_entry(sb, &file_dir_entry, file_inode->i_ino, sector_pos_cast(file_inode->i_private)))
  {
    printk(KERN_ERR "Error reading dir entry for: %s\n", fname);
    return -EFAULT;
  }

  if(!upanfs_read_dir_entry(sb, &parent, dir->i_ino, sector_pos_cast(dir->i_private)))
  {
    printk(KERN_ERR "Error reading parent dir entry for: %s\n", fname);
    return -EFAULT;
  }

	if((file_dir_entry.usAttribute & ATTR_TYPE_DIRECTORY) == ATTR_TYPE_DIRECTORY)
	{
		if(file_dir_entry.uiSize != 0)
    {
      printk(KERN_ERR "cannot delete directory %s - it's not empty\n", fname);
      return -ENOTEMPTY;
    }
	}

	uiCurrentSectorID = file_dir_entry.uiStartSectorID;
	while(uiCurrentSectorID != EOC)
	{
    if(!upanfs_deallocate_sector(sb, uiCurrentSectorID, &uiNextSectorID))
    {
      printk(KERN_ERR "error deallocate sector %u while deleting dir entry: %s\n", uiCurrentSectorID, fname);
      return -EFAULT;
    }
		uiCurrentSectorID = uiNextSectorID;
	}

  file_dir_entry.usAttribute |= ATTR_DELETED_DIR;

  if(!upanfs_write_dir_entry(sb, &file_dir_entry, file_inode->i_ino, sector_pos_cast(file_inode->i_private)))
  {
    printk(KERN_ERR "error writing deleted dir entry for: %s\n", fname);
    return -EFAULT;
  }

  parent.uiSize--;

  if(!upanfs_write_dir_entry(sb, &parent, dir->i_ino, sector_pos_cast(dir->i_private)))
  {
    printk(KERN_ERR "error writing parent of deleted dir entry for: %s\n", fname);
    return -EFAULT;
  }

  return 0;
}

static int upanfs_rmdir(struct inode* dir, struct dentry* dentry)
{
  return upanfs_delete_dir_entry(dir, dentry);
}

static int upanfs_delete(struct inode* dir, struct dentry* dentry)
{
  return upanfs_delete_dir_entry(dir, dentry);
}

static int upanfs_iterate(struct file *filp, struct dir_context *ctx)
{
  loff_t pos;
  struct inode *inode;
  struct super_block *sb;
  upanfs_mount_block* mblock;
  struct buffer_head *bh;
  upanfs_dir_entry dir_entry;
  unsigned uiCurrentSectorID;
  unsigned uiScanDirCount = 0;
  bool done = false;
  
  pos = ctx->pos;
  if (pos) {
    /* FIXME: We use a hack of reading pos to figure if we have filled in all data.
     * We should probably fix this to work in a cursor based model and
     * use the tokens correctly to not fill too many data in each cursor based call */
    return 0;
  }

  inode = filp->f_path.dentry->d_inode;
  sb = inode->i_sb;
  mblock = (upanfs_mount_block*)sb->s_fs_info;
  if(!upanfs_read_dir_entry(sb, &dir_entry, inode->i_ino, sector_pos_cast(inode->i_private)))
  {
    printk(KERN_ERR "failed to read dir_entry while iterating: %s\n", filp->f_path.dentry->d_name.name);
    return -EPERM;
  }

  if(!(dir_entry.usAttribute & ATTR_TYPE_DIRECTORY))
  {
    printk(KERN_ERR "%s is not a directory to iterate\n", filp->f_path.dentry->d_name.name);
    return -ENOTDIR;
  }

  uiCurrentSectorID = dir_entry.uiStartSectorID;
  while(uiCurrentSectorID != EOC)
  {
    byte* dir_buffer;
    int sector = upanfs_get_real_sec_num(uiCurrentSectorID, mblock);
    if(!(bh = sb_bread(sb, sector_to_block(sb, sector))))
    {
      printk(KERN_ERR "out of memory - failed to allocate bh");
      return -ENOMEM;
    }
    dir_buffer = bh->b_data + block_offset(sb, sector);
    for(unsigned uiSectorPosIndex = 0; uiSectorPosIndex < DIR_ENTRIES_PER_SECTOR; uiSectorPosIndex++)
    {
      upanfs_dir_entry* cur_dir = ((upanfs_dir_entry*)dir_buffer) + uiSectorPosIndex ;
      if(!(cur_dir->usAttribute & ATTR_DELETED_DIR))
      {
        dir_emit(ctx, cur_dir->Name, 33, uiCurrentSectorID, DT_UNKNOWN);
        ctx->pos += sizeof(upanfs_dir_entry);
        uiScanDirCount++ ;
        if(uiScanDirCount >= dir_entry.uiSize)
        {
          done = true;
          break;
        }
      }
    }
    brelse(bh);
    if(done)
      break;
    uiCurrentSectorID = upanfs_get_sec_entry_val(sb, uiCurrentSectorID);
  }
  return 0;
}

static bool upanfs_get_fs_bootblock(struct super_block* sb)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  struct buffer_head* bh;
  byte* bpb_data;
  if(!(bh = sb_bread(sb, sector_to_block(sb, 1))))
  {
    printk(KERN_ERR "Failed to read first BPB block");
    return false;
  }

  bpb_data = bh->b_data + block_offset(sb, 1);

  if(bpb_data[510] != (byte)0x55 || bpb_data[511] != (byte)0xAA)
  {
     printk(KERN_ERR "Invalid BPB sector end");
     return false;
  }

  memcpy(&mblock->bpb, bpb_data, sizeof(upanfs_boot_block));
  brelse(bh);

  if(mblock->bpb.BPB_BootSig != 0x29)
  {
    printk(KERN_ERR "Invalid BPB signature");
    return false;
  }

  if(mblock->bpb.BPB_FSTableSize == 0)
  {
    printk(KERN_ERR "upanfs with zero table size!!");
    return false;
  }

  if(mblock->bpb.BPB_BytesPerSec != 0x200)
  {
    printk(KERN_ERR "upanfs block size is incorrect");
    return false;
  }

  if(mblock->bpb.BPB_jmpBoot[0] != 0xEB 
    || mblock->bpb.BPB_jmpBoot[1] != 0xFE 
    || mblock->bpb.BPB_jmpBoot[2] != 0x90
    || mblock->bpb.BPB_ExtFlags  != 0x0080
    || mblock->bpb.BPB_FSVer != 0x0100
    || mblock->bpb.BPB_FSInfo != 1
    || mblock->bpb.BPB_VolID != 0x01)
  {
    printk(KERN_ERR "invalid upanfs BPB");
    return false;
  }

  return true;
}

static bool upanfs_load_fs_table(struct super_block* sb)
{
  int sectorsPerBlock = sectors_per_block(sb);
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  mblock->fstable = (byte*)kmalloc(512 * mblock->bpb.BPB_FSTableSize, GFP_KERNEL);

  for(int i = 0; i < mblock->bpb.BPB_FSTableSize;)
  {
    struct buffer_head *bh;
    int m, n;
    int sector = i + mblock->bpb.BPB_RsvdSecCnt + 1;
    if(!(bh = sb_bread(sb, sector_to_block(sb, sector))))
      return false;
    
    m = sector % sectorsPerBlock;
    n = sectorsPerBlock - m;
    if(i + n > mblock->bpb.BPB_FSTableSize)
      n = mblock->bpb.BPB_FSTableSize - i;

    memcpy(mblock->fstable + 512 * i, bh->b_data + block_offset(sb, sector), 512 * n);
    i += n;
    brelse(bh);
  }

  return true;
}

static int upanfs_load(struct super_block *sb, void *data, int silent)
{
  struct inode *root_inode;
  upanfs_mount_block* mblock;
  upanfs_dir_entry root_dir;
  sb->s_fs_info = 0;
  if(sb->s_blocksize % 512 != 0)
  {
    printk(KERN_ERR "device block size is not a multiple of 512!");
    return -EPERM;
  }
  mblock = (upanfs_mount_block*)kmalloc(sizeof(upanfs_mount_block), GFP_KERNEL);
  mblock->fstable = 0;
  /* A magic number that uniquely identifies upanfs filesystem type */
  sb->s_magic = 0x93;
  sb->s_fs_info = mblock;

  if(!upanfs_get_fs_bootblock(sb))
    return -EPERM;

  if(!upanfs_load_fs_table(sb))
    return -EPERM;

  if(!upanfs_read_dir_entry(sb, &root_dir, 0, 0))
  {
    printk(KERN_ERR "error reading root dir\n");
    return -EFAULT;
  }

  sb->s_maxbytes = mblock->bpb.BPB_BytesPerSec;
  sb->s_op = &upanfs_sops;

  root_inode = new_inode(sb);
  root_inode->i_ino = 0;
  inode_init_owner(root_inode, NULL, S_IFDIR | 0755);
  root_inode->i_sb = sb;
  root_inode->i_op = &upanfs_inode_ops;
  root_inode->i_fop = &upanfs_dir_operations;
  root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
  root_inode->i_private = inode_private_cast(0);
  root_inode->i_size = dir_size(root_dir.uiSize);
  sb->s_root = d_make_root(root_inode);
  if (!sb->s_root)
  {
    printk(KERN_ERR "failed to make_root");
    return -EPERM;
  }

  return 0;
}

static struct dentry* upanfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
  struct dentry *ret;
  ret = mount_bdev(fs_type, flags, dev_name, data, upanfs_load);
  if (unlikely(IS_ERR(ret)))
    printk(KERN_ERR "Error mounting upanfs");
  else
    printk(KERN_INFO "upanfs is succesfully mounted on [%s]\n", dev_name);
  return ret;
}

static void upanfs_umount(struct super_block *sb)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  if(mblock)
  {
    //sync bpb
    struct buffer_head* bpb_bh;
    byte* bpb_data;
    if(!(bpb_bh = sb_bread(sb, sector_to_block(sb, 1))))
    {
      printk(KERN_ERR "Failed to allocate for bpb_bh");
    }
    else
    {
      bpb_data = bpb_bh->b_data + block_offset(sb, 1);
      memcpy(bpb_data, &mblock->bpb, sizeof(upanfs_boot_block));
      mark_buffer_dirty(bpb_bh);
      sync_dirty_buffer(bpb_bh);
      brelse(bpb_bh);
    }
  }
  printk(KERN_INFO "Unmount succesfull.\n");
  kill_block_super(sb);
}

static int __init upanfs_init(void)
{
  int ret = register_filesystem(&upanfs_type);
  if (likely(ret == 0))
    printk(KERN_INFO "Sucessfully registered upanfs\n");
  else
    printk(KERN_ERR "Failed to register upanfs. Error:[%d]", ret);
  return ret;
}

static void __exit upanfs_fini(void)
{
  int ret = unregister_filesystem(&upanfs_type);
  if (likely(ret == 0))
    printk(KERN_INFO "Sucessfully unregistered upanfs\n");
  else
    printk(KERN_ERR "Failed to unregister upanfs. Error:[%d]", ret);
  return;
}

module_init(upanfs_init);
module_exit(upanfs_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("prajwala");
