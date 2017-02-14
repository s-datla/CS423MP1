#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "mp1_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_ID");
MODULE_DESCRIPTION("CS-423 MP1");

#define DEBUG 1
#define PROC_DIR_NAME "mp1"
#define PROC_FILE_NAME "status"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;

char *msg;
int len, temp;
static ssize_t proc_read(struct file *filp,char *buf, size_t count, loff_t *offp){
	if (count > temp){
		count = temp;
	}
	temp = temp-count;
	copy_to_user(buf,msg,count);
	if (count == 0){
		temp = len;
	}
	return count;
}

struct file_operations proc_fops = {
	read: proc_read
};

void proc_init(void){
	proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
	if(!proc_dir){
		printk(KERN_ALERT "Failed to create MP1 directory!");
	}
	proc_entry = proc_create(PROC_FILE_NAME, 0666, proc_dir, &proc_fops);
	if(!proc_entry){
		printk(KERN_ALERT "Failed to create Status File!");
	}
	msg = "Created Proc MP1 Test\n";
	len = strlen(msg);
	temp = len;	
	printk(KERN_ALERT "Created proc entry");
	
}

// defined proc entry and exit functions
void proc_cleanup(void){	
	remove_proc_entry(PROC_FILE_NAME, proc_dir);
	remove_proc_entry(PROC_DIR_NAME, NULL);
}

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif
   // Insert your code here ...
   proc_init();
   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif
   // Insert your code here ...
   
   proc_cleanup();
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);
MODULE_LICENSE("GPL");
