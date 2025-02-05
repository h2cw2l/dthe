#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/cred.h>


#define MYFS_MAGIC 0x64668735

static struct vfsmount *myfs_mount;
static int myfs_mount_count;

/*
static struct inode *myfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);

    if (inode) {
        inode->i_mode = mode;
        inode->i_uid = current_fsuid();
        inode->i_gid = current_fsgid();
        inode->i_size = VMACACHE_SIZE;
        // inode->i_blksize = VMACACHE_SIZE;
        inode->i_blocks = 0;
        inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
        switch (mode & S_IFMT) {
            default:
                init_special_inode(inode, mode, dev);
                break;
            case S_IFREG:
                printk("create a file\n");
                break;
            case S_IFDIR:
                inode->i_op = &simple_dir_inode_operations;
                inode->i_fop = &simple_dir_operations;
                printk("create a dir file\n");
                inc_nlink(inode);
                break;
        }
    }

    return inode;
}

static int myfs_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
    struct inode *inode;
    int error = -EPERM;

    if (dentry->d_inode) {
        return -EEXIST;
    }

    inode = myfs_get_inode(dir->i_sb, mode, dev);
    if (inode) {
        d_instantiate(dentry, inode);
        dget(dentry);
        error = 0;
    }

    return error;
}

static int myfs_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
    int res;

    res = myfs_mknod(dir, dentry, mode | S_IFDIR, 0);
    if (!res) {
        inc_nlink(dir);
    }

    return res;
}

static int myfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
    return myfs_mknod(dir, dentry, mode | S_IFREG, 0);
}
*/

static struct inode * myfs_get_inode(struct super_block * sb, int mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if(inode)
	{
		inode -> i_mode = mode;
		//@i_uid：user id
		inode->i_uid  = current_fsuid();
		//@i_gid：group id组标识符
		inode->i_gid  = current_fsgid();
		//@i_size：文件长度
		inode -> i_size = VMACACHE_SIZE;
		//@i_blocks：指定文件按块计算的长度
		inode -> i_blocks = 0;
		//@i_atime：最后访问时间
		//@i_mtime：最后修改时间
		//@i_ctime：最后修改inode时间
		inode -> i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

		switch(mode & S_IFMT)
		{
			default:
				init_special_inode(inode,mode,dev);
				break;
			case S_IFREG:
				printk("creat a file\n");
				break;
			case S_IFDIR:
				printk("creat a content\n");
				//inode_operations
				inode -> i_op = &simple_dir_inode_operations;
				//file_operation	
				inode -> i_fop = &simple_dir_operations;
				//@：文件的链接计数，使用stat命令可以看到Links的值，硬链接数目
				//inode -> i_nlink++;
				inc_nlink(inode);
				break;			
		}
	}
	return inode;
}



//把创建的inode和dentry连接起来
static int myfs_mknod(struct inode * dir, struct dentry * dentry, int mode, dev_t dev)
{
	struct inode * inode;
	int error = -EPERM;

	if(dentry -> d_inode)
		return -EPERM;

	inode = myfs_get_inode(dir->i_sb, mode, dev);
	if(inode)
	{
		d_instantiate(dentry,inode);
		dget(dentry);
		error = 0;

	}
	return error;
}

//************************************************************************
//							创建目录，文件
//************************************************************************

static int myfs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
{
	int res;

	res = myfs_mknod(dir, dentry, mode|S_IFDIR, 0);
	if(!res)
	{
		inc_nlink(dir);

	}
	return res;
}

static int myfs_create(struct inode * dir, struct dentry * dentry, int mode)
{
	return myfs_mknod(dir, dentry, mode|S_IFREG, 0);
}

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	//这个结构体如下：
	//struct tree_descr { const char *name; const struct file_operations *ops; int mode; };
	static struct tree_descr debug_files[] = {{""}};

	return simple_fill_super(sb,MYFS_MAGIC,debug_files);
}

/*
这个函数是按照内核代码中的样子改的，是struct dentry *类型，这里是一个封装，这里可以返回好几种函数：
　－　mount_single
　－　mount_bdev　
　－　mount_nodev
*/

static struct dentry *myfs_get_sb(struct file_system_type *fs_type, int flags,
		       const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, myfs_fill_super);
}


/*
static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
    static struct tree_descr debug_files[] = {{""}};
    return simple_fill_super(sb, MYFS_MAGIC, debug_files);
}

static struct dentry *myfs_get_sb(struct file_system_type *fs_type,
                                       int flags, const char *dev_name, void *data)
{
    return mount_single(fs_type, flags, data, myfs_fill_super);
}
*/

static struct file_system_type myfs_type = {
    .owner = THIS_MODULE,
    .name  = "myfs",
    .mount = myfs_get_sb,
    .kill_sb = kill_litter_super,
};

/*
static int myfs_create_by_name(const char *name, mode_t mode,
                               struct dentry *parent, struct dentry **dentry)
{
    int error = 0;

    dump_stack();
    if (!parent) {
        printk("myfs_mount %p\n", myfs_mount);
        if (myfs_mount && myfs_mount->mnt_sb) {
            parent = myfs_mount->mnt_sb->s_root;
        }
    }
    if (!parent) {
        printk("Ah! can not find a parent!\n");
        return -EFAULT;
    }
    printk("name %s, parent %s\n", name, parent);
    *dentry = NULL;
    inode_lock(d_inode(parent));
    *dentry = lookup_one_len(name, parent, strlen(name));
    if (!IS_ERR(*dentry)) {
        if ((mode & S_IFMT) == S_IFDIR) {
            error = myfs_mkdir(parent->d_inode, *dentry, mode);
        } else {
            error = myfs_create(parent->d_inode, *dentry, mode);
        }
    } else {
        error = PTR_ERR(*dentry);
    }
    inode_unlock(d_inode(parent));

    return error;
}
*/

static int myfs_create_by_name(const char * name, mode_t mode,
				struct dentry * parent, struct dentry ** dentry)
{
	int error = 0;
        printk("myfs_create_by_name, entry, parent %p, name %s\n", parent, name);
	if(!parent)
	{
		if(myfs_mount && myfs_mount -> mnt_sb)
		{
			parent = myfs_mount->mnt_sb->s_root;
		}
	}

	if(!parent)
	{
		printk("can't find a parent");
		return -EFAULT;
	}
        printk("myfs_create_by_name, parent not NULL, name %s\n", name);
	*dentry = NULL;

	// inode_lock(d_inode(parent));
        printk("myfs_create_by_name, before lookup\n");
	*dentry = lookup_one_len(name,parent,strlen(name));
        printk("myfs_create_by_name, after lookup, *dentry %p\n", *dentry);
	if(!IS_ERR(*dentry))
	{
		if((mode & S_IFMT) == S_IFDIR)
		{
		    printk("create dir, name %s\n");
			error = myfs_mkdir(parent->d_inode, *dentry, mode);
		}
		else
		{
		    printk("create file, name %s\n");
			error = myfs_create(parent->d_inode, *dentry, mode);
		}
	}
	//error是０才对
	if (IS_ERR(*dentry)) {
		error = PTR_ERR(*dentry);
	}
	// inode_unlock(d_inode(parent));

	return error;
}

/*
struct dentry *myfs_create_file(const char *name, mode_t mode,
                                struct dentry *parent, void *data,
                                struct file_operations *fops)
{
    struct dentry *dentry = NULL;
    int error;
 
    printk("myfs: creating file %s, parent %s\n", name, parent);
    error = myfs_create_by_name(name, mode, parent, &dentry);
    if (error) {
        dentry = NULL;
        goto exit;
    }
    printk("myfs_create_file, dentry %p\n", dentry);
    if (dentry->d_inode) {
        if (data) {
            dentry->d_inode->i_private = data;
        } 
        if (fops) {
            dentry->d_inode->i_fop = fops;
        }
    }

exit:
    return dentry;
}
*/

struct dentry * myfs_create_file(const char * name, mode_t mode,
				struct dentry * parent, void * data,
				struct file_operations * fops)
{
	struct dentry * dentry = NULL;
	int error;

	printk("myfs:creating file '%s', parent %p, &dentry %p\n", name, parent, &dentry);
	error = myfs_create_by_name(name, mode, parent, &dentry);
        printk("after create file %s\n", name);
	if(error)
	{
		dentry = NULL;
		goto exit;
	}
        /*
	if(dentry->d_inode)
	{
		if(data)
			dentry->d_inode->i_private = data;
		if(fops)
			dentry->d_inode->i_fop = fops;
	}
        */

exit:
	return dentry;
}

static int myfs_create_dir(const char *name, struct dentry *parent)
{
    return myfs_create_file(name, S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
                            parent, NULL, NULL);
}

static int __init myfs_init(void)
{
    int retval;
    struct dentry *pslot;

    retval = register_filesystem(&myfs_type);
    printk("----------------------retval %d -----------------------\n", retval);
    if (!retval) {
        myfs_mount = kern_mount(&myfs_type);
        if (IS_ERR(myfs_mount)) {
            printk(KERN_ERR, "myfs: could not mount!\n");
            unregister_filesystem(&myfs_type);
            return retval;
        }
    }

    pslot = myfs_create_dir("woman", NULL);
    myfs_create_file("lbb", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    myfs_create_file("fbb", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    myfs_create_file("ljl", S_IFREG | S_IRUGO, pslot, NULL, NULL);

    pslot = myfs_create_dir("man", NULL);
    myfs_create_file("ldh", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    myfs_create_file("lcw", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    myfs_create_file("jw",  S_IFREG | S_IRUGO, pslot, NULL, NULL);
 
    return retval;    
}

static void __exit myfs_exit(void)
{
    simple_release_fs(&myfs_mount, &myfs_mount_count);
    unregister_filesystem(&myfs_type);

    return;
}

module_init(myfs_init);
module_exit(myfs_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a simple fs");
MODULE_VERSION("Ver 0.1");
