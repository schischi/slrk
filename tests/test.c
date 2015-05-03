#include "slrk.h"
#include "tests.h"
#include "test.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kallsyms.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

extern struct unit_test *__tests[];
extern struct unit_test *__etests[];

static int my_close(struct inode *inode, struct file *filp);
static int my_open(struct inode *inode, struct file *filp);
static ssize_t my_read(struct file *filp, char __user *buf, size_t size,
        loff_t *off);
static int my_mmap(struct file *filp, struct vm_area_struct *vma);

static int tests_cnt;
static struct dentry *dir;
static struct list_head tests = LIST_HEAD_INIT(tests);
static const struct file_operations my_fops = {
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .mmap = my_mmap,
};

static void export_user_elf(struct user_elf *elf)
{
    elf->file = debugfs_create_file(elf->name, 0555, dir, elf,
            &my_fops);
    //elf->file = debugfs_create_file_size(elf->name, 0555, dir, elf,
    //        &my_fops, (size_t)*elf->size);
}

static void add_unit_test(struct unit_test *test)
{
    list_add(&test->list, &tests);
    if (test->elf.name)
        export_user_elf(&test->elf);
    tests_cnt += test->n;
}

void init_unit_tests(void)
{
    int mod_cnt = 0;
    struct unit_test **ptr;

    dir = debugfs_create_dir("slrk", 0);
    if (!dir)
        return;

    /* Initialize all the tests registered in the .tests section */
    for (ptr = __tests; ptr < __etests; ++ptr) {
        add_unit_test(*ptr);
        ++mod_cnt;
    }
    pr_log("Loaded %d tests in %d modules\n", tests_cnt, mod_cnt);
}

void run_unit_tests(void)
{
    struct unit_test *it;

    list_for_each_entry(it, &tests, list) {
        pr_log("==> Testing "COLOR_BLUE"%s"COLOR_RESET"\n", it->name);
        if (it->run)
            it->run();
    }
}

void cleanup_unit_tests(void)
{
    debugfs_remove_recursive(dir);
}

static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    struct page *pageptr;

    pageptr = virt_to_page(vma->vm_private_data);
    get_page(pageptr);
    vmf->page = pageptr;
    return 0;
}

static struct vm_operations_struct mmap_vm_ops = {
    .fault =   mmap_fault,
};

static int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct user_elf *elf = filp->private_data;

    vma->vm_ops = &mmap_vm_ops;
    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
    /* assign the file private data to the vm private data */
    vma->vm_private_data = elf->ptr;
    return 0;
}


static int my_close(struct inode *inode, struct file *filp)
{
    struct user_elf *elf = filp->private_data;
    free_page((unsigned long)elf->ptr);
    kfree(elf);
    filp->private_data = NULL;
    return 0;
}

static int my_open(struct inode *inode, struct file *filp)
{
    struct user_elf *orig_elf = inode->i_private;
    struct user_elf *elf = kmalloc(sizeof (struct user_elf), GFP_KERNEL);
    if (!elf) {
        pr_log("kmalloc failed\n");
        return 0;
    }
    elf->size = orig_elf->size;
    elf->name = orig_elf->name;
    elf->ptr = (void *)get_zeroed_page(GFP_KERNEL);
    if (!elf->ptr) {
        pr_log("kmalloc failed\n");
        return 0;
    }
    memcpy(elf->ptr, orig_elf->ptr, *orig_elf->size);
    filp->private_data = elf;
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t size,
        loff_t *off)
{
    unsigned long ret;
    struct user_elf *elf = filp->private_data;
    loff_t sz = (loff_t)*elf->size;

    if (*off >= sz)
        return 0;
    if (size > sz - *off)
        size = (size_t)sz - (size_t)*off;
    ret = copy_to_user(buf, &(((char *)elf->ptr)[*off]), (unsigned long)size);
    //TODO
    //if (ret != size)
    //    pr_log("copy ret failed %lu - %zu\n", ret, size);
    *off += size;

    return (ssize_t)size;
}
