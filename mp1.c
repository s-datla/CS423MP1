#define LINUX
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/fs.h>   // for basic filesystem
#include <linux/proc_fs.h>  // for the proc filesystem
#include <linux/jiffies.h>  // for jiffies
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>

// Prototypes
void cleanup_list(void);
void update_cpu_time(void);

#include "mp1_given.h"

// Definitions
#define DEBUG 1
#define ENTRY_NAME "status"
#define DIR_NAME "mp1"

// Proc Filesystem variables
static struct proc_dir_entry* proc_dir;
static struct proc_dir_entry* proc_entry;

// WorkQueue definitions
static void mykmod_work_handler(struct work_struct *pwork);
static LIST_HEAD(Head);
static struct work_struct update;
static DECLARE_WORK(update, mykmod_work_handler);

// Timer definition
static struct timer_list my_timer;
struct file *proc_entry_fs;

// Linked List definitions
struct process_list{
  struct list_head list;
  unsigned long cpu_time;
  int PID;
};
struct process_list p_list;

// Other variables
int len,temp,flag;
int emptyFlag = 0;


ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
  int len = 0, total = 0;
  char my_buffer[100];
  if (*offp > 0) {
      total = 0
  } else {
    struct process_list *process_entry;
    list_for_each_entry(process_entry, &p_list.list, list) {
      len = sprintf(my_buffer,"PID: %d , CPU: %lu \n", process_entry->PID, process_entry->cpu_time);
      copy_to_user(buf,my_buffer,len+1)
      total += len + 1;
    }
  }
  return total;
}


ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
  long user_PID;
  copy_from_user(msg,buf,count);
  kstrtoul(msg,0,&user_PID);
  add_node_to_list(user_PID);
  return count;
}

static const struct file_operations entry_fops = {
  .owner  = THIS_MODULE,
  .open = NULL,
  .write = write_proc,
  .read = read_proc,
  .release = NULL
};



void update_cpu_time(void)//this part goes to the work queue
{
  struct process_list *process_entry;
  printk(KERN_ALERT "Updating CPU times");
  list_for_each_entry(process_entry, &p_list.list, list) {
    if (get_cpu_use(process_entry->PID, &(process_entry->cpu_time)) == 0){
      printk(KERN_INFO "Successfully updated cpu times");
    } else {
      printk(KERN_INFO "Failed to update new times");
    }
    printk(KERN_INFO "PID: %d; CPU_TIME: %lu\n;", process_entry->PID, process_entry->cpu_time);
  }
}

int add_node_to_list(unsigned long PID){
  struct process_list *newNode;
  newNode = kmalloc(sizeof(*newNode), GFP_KERNEL);
  newNode->PID = PID;
  newNode->cpu_time = 0;
  if (get_cpu_use(PID, &(newNode->cpu_time)) == 0){
    printk(KERN_INFO "Successfully updated cpu times");
  } else {
    printk(KERN_INFO "Failed to add new node");
    return -1;
  }
  INIT_LIST_HEAD(&newNode->list);
  list_add_tail(&(newNode->list),&(p_list.list));
  if (emptyFlag == 0) emptyFlag = 1;
  return 0;
}

void my_timer_callback( unsigned long data )
{
  if(flag==1){
    flag=0;
  }else{
    flag=1;
  }
  schedule_work(&update);
  mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
}

static void mykmod_work_handler(struct work_struct *pwork)
{
  if(emptyFlag != 0){
    update_cpu_time();
  }else{
    printk(KERN_ALERT "Empty list");
  }
}


// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
  #ifdef DEBUG
  printk(KERN_ALERT "MP1 MODULE LOADING\n");
  #endif
  // Insert your code here ...
  proc_dir = proc_mkdir(DIR_NAME, NULL);
  proc_entry= proc_create(ENTRY_NAME,0666, proc_dir, &entry_fops);
  if (!proc_entry) {
    return -ENOMEM;
  }
  // Setting up timer functions
  setup_timer(&my_timer, my_timer_callback, 0);
  /* setup timer interval to 1000 msecs */
  mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
  flag=0;//for tesing purpose

  printk(KERN_ALERT "Initialising Process List");
  INIT_LIST_HEAD(&p_list.list);

  printk(KERN_ALERT "MP1 MODULE LOADED\n");
  return 0;
}

// mp1_exit - Called when module is unloading
void __exit mp1_exit(void)
{
  #ifdef DEBUG
  printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
  #endif
  remove_proc_entry(ENTRY_NAME, proc_dir);
  remove_proc_entry(DIR_NAME, NULL);
  del_timer(&my_timer);
  cleanup_list();
  printk(KERN_ALERT "MP1 MODULE UNLOADED\n");

}

// Function used to free up the memory allocated by the
void cleanup_list(void) {
  struct process_list *process_entry, *temp;
  printk(KERN_ALERT "Cleaning up list safely");
  list_for_each_entry_safe(process_entry, temp, &p_list.list, list){
    printk(KERN_INFO "Freeing node %d \n", process_entry->PID);
    list_del(&process_entry->list);
    kfree(process_entry);
  }
}

MODULE_LICENSE("GPL");
module_init(mp1_init);
module_exit(mp1_exit);
