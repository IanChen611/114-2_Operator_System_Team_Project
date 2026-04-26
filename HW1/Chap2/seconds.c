#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>

#define PROC_NAME "seconds"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team B");
MODULE_DESCRIPTION("Report elapsed seconds since module load");

static unsigned long start_jiffies;
static struct proc_dir_entry *proc_entry;

static int seconds_show(struct seq_file *m, void *v)
{
    unsigned long elapsed_jiffies;
    unsigned long elapsed_seconds;

    elapsed_jiffies = jiffies - start_jiffies;
    elapsed_seconds = elapsed_jiffies / HZ;

    seq_printf(m, "%lu\n", elapsed_seconds);
    return 0;
}

static int seconds_open(struct inode *inode, struct file *file)
{
    return single_open(file, seconds_show, NULL);
}

static const struct proc_ops seconds_proc_ops = {
    .proc_open    = seconds_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init seconds_init(void)
{
    start_jiffies = jiffies;

    proc_entry = proc_create(PROC_NAME, 0, NULL, &seconds_proc_ops);
    if (!proc_entry) {
        pr_err("failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created, start_jiffies=%lu, HZ=%d\n",
            PROC_NAME, start_jiffies, HZ);
    return 0;
}

static void __exit seconds_exit(void)
{
    if (proc_entry) {
        proc_remove(proc_entry);
    }

    pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(seconds_init);
module_exit(seconds_exit);