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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
  .iterate = upanfs_iterate,
#else
  .readdir = simplefs_readdir,
#endif
};

static void upanfs_destory_inode(struct inode *inode)
{
  upanfs_dir_entry *direntry = (upanfs_dir_entry*)inode->i_private;
  printk(KERN_INFO "deleting direntry %s (%lu)\n", direntry->Name, inode->i_ino);
  //delete directory here ??
}

static void upanfs_put_super(struct super_block *sb)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  printk(KERN_INFO "Calling upanfs_put_super");
}

static struct dentry *upanfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
  printk(KERN_INFO "dir lookup");
  return 0;
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
  printk(KERN_INFO "iterating directory");
  return 0;
}

upanfs_dir_entry* upanfs_get_direnty(struct super_block *sb, uint64_t inode_no)
{
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  upanfs_dir_entry* direntry = NULL;
  upanfs_dir_entry* inode_buffer = NULL;

  int i;
  struct buffer_head *bh;

  /* The inode store can be read once and kept in memory permanently while mounting.
   * But such a model will not be scalable in a filesystem with
   * millions or billions of files (inodes) */
/* TODO:
  bh = sb_bread(sb, SIMPLEFS_INODESTORE_BLOCK_NUMBER);

  sfs_inode = (struct simplefs_inode *)bh->b_data;

  for (i = 0; i < sfs_sb->inodes_count; i++) {
    if (sfs_inode->inode_no == inode_no) {
      inode_buffer = kmem_cache_alloc(sfs_inode_cachep, GFP_KERNEL);
      memcpy(inode_buffer, sfs_inode, sizeof(*inode_buffer));

      break;
    }
    sfs_inode++;
  }
  brelse(bh);
*/
  return inode_buffer;
}

static bool upanfs_get_fs_bootblock(struct buffer_head *bh, upanfs_mount_block* mblock)
{
  if(((byte*)(bh->b_data))[510] != (byte)0x55 || ((byte*)(bh->b_data))[511] != (byte)0xAA)
  {
     printk(KERN_ERR "Invalid BPB sector end");
     return false;
  }

  memcpy(&mblock->bpb, bh->b_data, sizeof(upanfs_boot_block));

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
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  mblock->fstable = (byte*)kmalloc(512 * mblock->bpb.BPB_FSTableSize, GFP_KERNEL);
  for(i = 0; i < mblock->bpb.BPB_FSTableSize; i++)
  {
    if(!(bh = sb_bread(sb, i + mblock->bpb.BPB_RsvdSecCnt + 1)))
      return false;
     memcpy(mblock->fstable + 512 * i, bh->b_data, 512);
  }
  return true;
}

int upanfs_get_real_sec_num(int iSectorID, upanfs_mount_block* mblock)
{
	return iSectorID + 1 + mblock->bpb.BPB_RsvdSecCnt + mblock->bpb.BPB_FSTableSize ;
}

bool upanfs_read_root_directory(struct super_block* sb)
{
  struct buffer_head* bh;
  upanfs_mount_block* mblock = (upanfs_mount_block*)sb->s_fs_info;
  upanfs_cwd* fs_cwd = &mblock->fs_cwd ;
  if(!(bh = sb_bread(sb, upanfs_get_real_sec_num(0, mblock))))
    return false;
  memcpy((byte*)&fs_cwd->dir_entry, bh->b_data, sizeof(upanfs_dir_entry));
  fs_cwd->uiSectorNo = 0 ;
  fs_cwd->uiSectorEntryPosition = 0 ;
  return true;
}


static bool upanfs_load(struct buffer_head *bh, struct super_block *sb, void *data)
{
  struct inode *root_inode;
  upanfs_mount_block* mblock = (upanfs_mount_block*)kmalloc(sizeof(upanfs_mount_block), GFP_KERNEL);
  /* A magic number that uniquely identifies upanfs filesystem type */
  sb->s_magic = 0x93;
  sb->s_fs_info = mblock;

  if(!upanfs_get_fs_bootblock(bh, mblock))
    return false;

  if(!upanfs_load_fs_table(sb))
    return false;

  if(!upanfs_read_root_directory(sb))
    return false;

  //memcpy_1((char*)current_fs_cwd, (char*)&mblock->fs_cwd, sizeof(upanfs_cwd)) ;

  sb->s_maxbytes = mblock->bpb.BPB_BytesPerSec;
  sb->s_op = &upanfs_sops;

  root_inode = new_inode(sb);
  root_inode->i_ino = 1;
  inode_init_owner(root_inode, NULL, S_IFDIR);
  root_inode->i_sb = sb;
  root_inode->i_op = &upanfs_inode_ops;
  root_inode->i_fop = &upanfs_dir_operations;
  root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;

  root_inode->i_private = upanfs_get_direnty(sb, 1);

  /* TODO: move such stuff into separate header. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
  sb->s_root = d_make_root(root_inode);
#else
  sb->s_root = d_alloc_root(root_inode);
  if (!sb->s_root)
    iput(root_inode);
#endif

  if (!sb->s_root)
    return -ENOMEM;

  return 0;
}

static int upanfs_mount_init(struct super_block *sb, void *data, int silent)
{
  struct buffer_head *bh;
  bool ret = false;
  sb->s_fs_info = 0;

  if(!(bh = sb_bread(sb, 1)))
  {
    printk(KERN_ERR "Failed to read first BPB block");
    return -ENOMEM;
  }

  ret = upanfs_load(bh, sb, data);
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
  if(mblock != 0)
  {
    printk(KERN_INFO "destroying mount block\n");
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
