#define LINUX
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include "mp1_given.h"
#include <linux/init.h>
#include <linux/fs.h>   // for basic filesystem
#include <linux/proc_fs.h>  // for the proc filesystem
#include <linux/jiffies.h>  // for jiffies
#include <asm/uaccess.h>

#define DEBUG 1

#define ENTRY_NAME "status"
#define DIR_NAME "mp1_z"
//prototypes
static void mykmod_work_handler(struct work_struct *pwork);
void add_node_to_list(long PID);
//Declarations for Workqueue
static DECLARE_WORK(update, mykmod_work_handler);
static struct work_struct update;

//Declarations fot proc files
 static struct proc_dir_entry* proc_dir;
 static struct proc_dir_entry* proc_entry;
 static char msg[1024];
 size_t num_byte_from_user;

//Declaration for timer
 static struct timer_list my_timer;

//Declaration for link list
struct process_list{
    struct list_head link;
    long cpu_time;
    long PID;
};
static struct process_list Head;
static int list_mutex=0;

//Returns process information when user reads the proc entry
 ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
  int pos=0;
  int len;
  char *pid= (char*)kmalloc(count,GFP_KERNEL);
  struct process_list *process_entry;

  if((int)*offp >0){
    kfree((void*)pid);
    return 0;
  }

  while(list_mutex);//wait if mutex=1
  list_mutex=1;//lock
  list_for_each_entry(process_entry, &Head.link, link) {

      len=sprintf(pid+pos,"PID= %lu, CPU_time=%lu \n", process_entry->PID, process_entry->cpu_time);
      pos+= len;
  }
  list_mutex=0;
  copy_to_user(buf,pid,pos);
  kfree((void*)pid);

  *offp +=pos;
  
return pos;
}

//The user program registers itself by writting to the proc entry
ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
  long user_PID;
  copy_from_user(msg,buf,count); 
  kstrtol(msg,0,&user_PID);
  num_byte_from_user = (size_t)count;
  add_node_to_list(user_PID);
  return count;
}

 static const struct file_operations entry_fops = {
    .owner  = THIS_MODULE,
    .open = NULL,
    .write = write_proc,
    .read = read_proc,
    .release = NULL,
 };

//Add node to link list and initialize its values
void add_node_to_list(long PID) 
{
    struct process_list *newNode;
    newNode = kmalloc(sizeof(struct process_list), GFP_KERNEL);
    (*newNode).cpu_time=0;
    (*newNode).PID=PID;          
    while(list_mutex);//wait if mutex=1
    list_mutex=1;//lock
    list_add(&((*newNode).link), &(Head.link));
    list_mutex=0;//unlock
}

//Updates process cpu time in link list. This function is only called by a workqueue task
void update_cpu_time(void)
{   //long cpuTime=0;
    struct process_list* temp_Node=NULL;
    while(list_mutex);//wait if mutex=1
    list_mutex=1;//lock
    list_for_each_entry(temp_Node, &Head.link, link)
    {   
       if (get_cpu_use((int)(temp_Node->PID), &(temp_Node->cpu_time)) == 0){
        printk(KERN_INFO "Successfully updated cpu times");
       } else {
        printk(KERN_INFO "Failed to update new times");
      }
    }
    list_mutex=0;//lock
}

//Timer event handler. First half
void my_timer_callback( unsigned long data )
{
  mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
  schedule_work(&update);
} 

//Timer event handler. Second half
static void mykmod_work_handler(struct work_struct *pwork)
{
  update_cpu_time();
}

//Deallocate memory for the link list upon unloading module
void cleanup_list(void) {
    struct process_list *temp_node, *backup_node;
    printk(KERN_ALERT "Cleaning up list safely");

    while(list_mutex);//lock
    list_mutex=1;
    list_for_each_entry_safe(temp_node, backup_node, &Head.link, link){
    printk(KERN_INFO "freeing node %ldn", (*temp_node).PID);
    list_del(&((*temp_node).link));
    kfree(temp_node);
  }
    list_mutex=0;
}

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{

   //declaration
// Insert your code here ...
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif

   proc_dir = proc_mkdir(DIR_NAME, NULL);
   proc_entry= proc_create(ENTRY_NAME,0666, proc_dir, &entry_fops);
   if (!proc_entry) {
         return -ENOMEM;
    }
   setup_timer(&my_timer, my_timer_callback, 0);
  /* setup timer interval to 5000 msecs */
   INIT_LIST_HEAD( &(Head.link) );
   mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));

   list_mutex=0;
   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}


void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif
   // Insert your code here ...
   remove_proc_entry(ENTRY_NAME, proc_dir);
   remove_proc_entry(DIR_NAME, NULL);
   del_timer(&my_timer);
   cleanup_list();
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

 MODULE_LICENSE("GPL");
 module_init(mp1_init);
 module_exit(mp1_exit);


