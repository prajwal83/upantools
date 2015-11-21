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

static int upanfs_get_real_sec_num(int iSectorID, upanfs_mount_block* mblock)
{
	return iSectorID + 1 + mblock->bpb.BPB_RsvdSecCnt + mblock->bpb.BPB_FSTableSize ;
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

static ssize_t upanfs_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos)
{
  return 0;
}

static ssize_t upanfs_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos)
{
  return 0;
}

static bool upanfs_read_dir_entry(struct super_block* sb, upanfs_dir_entry* dir_entry, unsigned sector, unsigned sector_pos)
{
  struct buffer_head* bh;
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  int real_sector = upanfs_get_real_sec_num(sector, mblock);
  int block_no = sector_to_block(sb, real_sector);
  int offset;
  if(!(bh = sb_bread(sb, block_no)))
    return false;
  offset = block_offset(sb, real_sector);
  memcpy((byte*)dir_entry, bh->b_data + offset + sector_pos *  sizeof(upanfs_dir_entry), sizeof(upanfs_dir_entry));
  brelse(bh);
  return true;
}

static void upanfs_destory_inode(struct inode *inode)
{
  //upanfs_dir_entry *direntry = (upanfs_dir_entry*)inode->i_private;
//  printk(KERN_INFO "deleting direntry %s (%lu)\n", direntry->Name, inode->i_ino);
  //delete directory here ??
}

static void upanfs_put_super(struct super_block *sb)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  printk(KERN_INFO "Calling upanfs_put_super");
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
  if(!upanfs_read_dir_entry(sb, &parent, parent_inode->i_ino, (unsigned)parent_inode->i_private))
    return NULL;
  uiCurrentSectorID = parent.uiStartSectorID;
  while(uiCurrentSectorID != EOC)
  {
    byte* dir_buffer;
    int sector = upanfs_get_real_sec_num(uiCurrentSectorID, mblock);
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
          inode->i_fop = &upanfs_dir_operations;
        else
          inode->i_fop = &upanfs_file_operations;

        inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
        inode->i_private = uiSectorPosIndex;

        inode_init_owner(inode, parent_inode, isDir ? S_IFDIR | 0755 : S_IFREG | 0644);
        d_add(child_dentry, inode);
        status = 2;
        break;
      }

      if(!(cur_dir->usAttribute & ATTR_DELETED_DIR))
      {
        uiScanDirCount++ ;
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
    printk(KERN_ERR "file/directory [%s] not found\n", child_dentry->d_name.name);
  return NULL;
}

static int upanfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
  printk(KERN_INFO "create new file");
  return 0;
}

static int upanfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
  printk(KERN_INFO "create directory");
  return 0;
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

  inode = filp->f_dentry->d_inode;
  sb = inode->i_sb;
  mblock = (upanfs_mount_block*)sb->s_fs_info;
  if(!upanfs_read_dir_entry(sb, &dir_entry, inode->i_ino, (unsigned)inode->i_private))
  {
    printk(KERN_ERR "failed to read dir_entry while iterating: %s\n", filp->f_dentry->d_name.name);
    return -EPERM;
  }

  if(!(dir_entry.usAttribute & ATTR_TYPE_DIRECTORY))
  {
    printk(KERN_ERR "%s is not a directory to iterate\n", filp->f_dentry->d_name.name);
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
      return -ENOSPC;
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

static bool upanfs_get_fs_bootblock(byte* bpb_data, upanfs_mount_block* mblock)
{
  if(bpb_data[510] != (byte)0x55 || bpb_data[511] != (byte)0xAA)
  {
     printk(KERN_ERR "Invalid BPB sector end");
     return false;
  }

  memcpy(&mblock->bpb, bpb_data, sizeof(upanfs_boot_block));

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
  struct buffer_head *bh;
  int i;
  int sectorsPerBlock = sectors_per_block(sb);
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;

  mblock->fstable = (byte*)kmalloc(512 * mblock->bpb.BPB_FSTableSize, GFP_KERNEL);

  for(i = 0; i < mblock->bpb.BPB_FSTableSize; i += sectorsPerBlock)
  {
    int sector = i + mblock->bpb.BPB_RsvdSecCnt + 1;
    int n = mblock->bpb.BPB_FSTableSize - i;
    if(!(bh = sb_bread(sb, sector_to_block(sb, sector))))
      return false;
    if(n > sectorsPerBlock)
      n = sectorsPerBlock;
    memcpy(mblock->fstable + 512 * i, bh->b_data, 512 * n);
    brelse(bh);
  }

  return true;
}

static bool upanfs_load(byte* bpb_data, struct super_block *sb, void *data)
{
  struct inode *root_inode;
  upanfs_mount_block* mblock = (upanfs_mount_block*)kmalloc(sizeof(upanfs_mount_block), GFP_KERNEL);
  mblock->fstable = 0;
  /* A magic number that uniquely identifies upanfs filesystem type */
  sb->s_magic = 0x93;
  sb->s_fs_info = mblock;

  if(!upanfs_get_fs_bootblock(bpb_data, mblock))
    return false;

  if(!upanfs_load_fs_table(sb))
    return false;

  sb->s_maxbytes = mblock->bpb.BPB_BytesPerSec;
  sb->s_op = &upanfs_sops;

  root_inode = new_inode(sb);
  root_inode->i_ino = 0;
  inode_init_owner(root_inode, NULL, S_IFDIR | 0755);
  root_inode->i_sb = sb;
  root_inode->i_op = &upanfs_inode_ops;
  root_inode->i_fop = &upanfs_dir_operations;
  root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;

  root_inode->i_private = 0;
  sb->s_root = d_make_root(root_inode);
  if (!sb->s_root)
  {
    printk(KERN_ERR "failed to make_root");
    return false;
  }

  return true;
}

static int upanfs_mount_init(struct super_block *sb, void *data, int silent)
{
  struct buffer_head *bh;
  bool ret = false;
  byte* bpb_data;
  sb->s_fs_info = 0;

  if(sb->s_blocksize % 512 != 0)
  {
    printk(KERN_ERR "device block size is not a multiple of 512!");
    return -ENOSPC;
  }

  if(!(bh = sb_bread(sb, sector_to_block(sb, 1))))
  {
    printk(KERN_ERR "Failed to read first BPB block");
    return -ENOMEM;
  }

  bpb_data = bh->b_data + block_offset(sb, 1);
  ret = upanfs_load(bpb_data, sb, data);
  brelse(bh);
  return ret ? 0 : -ENOSPC;
}

static struct dentry* upanfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
  struct dentry *ret;
  ret = mount_bdev(fs_type, flags, dev_name, data, upanfs_mount_init);
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
    printk(KERN_INFO "destroying mount block\n");
    if(mblock->fstable)
      kfree(mblock->fstable);
    kfree(mblock);
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
