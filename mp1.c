#define LINUX
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/workqueue.h>

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
#define DIR_NAME "mp1"

//prototypes
static void mykmod_work_handler(struct work_struct *pwork);

struct process_list new_list;

 static struct proc_dir_entry* proc_dir;
 static struct proc_dir_entry* proc_entry;
 static char msg[1024];

 static struct timer_list my_timer;
 static char proc_buffer;
 struct file *proc_entry_fs;

struct process_list{
    struct list_head link;
    long cpu_time;
    unsigned int PID;
};

static LIST_HEAD(Head);
static struct work_struct update;
static DECLARE_WORK(update, mykmod_work_handler);

int len,temp,flag;


 ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{

  if(count>temp)//why do we need these conditions to make "cat" print only once
{
count=temp;
}
temp=temp-count;
copy_to_user(buf,msg, count);
if(count==0)
temp=len;
   
return count;

}


ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
copy_from_user(msg,buf,count);
//printk(KERN_INFO "In write");
len=count;  // why use these variables?
temp=len;
return count;
}

 static const struct file_operations entry_fops = {
    .owner  = THIS_MODULE,
    .open = NULL,
    .write = write_proc,
    .read = read_proc,
    //.llseek = seq_lseek,
    .release = NULL,
 };

void add_node_to_list(int PID) //change PID to proper datatype and edit struct process_list
{
    struct process_list new_node;
    new_node.cpu_time=0;
    new_node.PID=2;          //For testing purpose, set PID to 2. Make it the value received from user space.
    list_add(&new_node.link, &Head);
}

void update_cpu_time(long cpu_time)//this part goes to the work queue
{
    struct process_list* temp_Node=NULL;
    list_for_each_entry(temp_Node, &Head, link)
    {
      (*temp_Node).cpu_time=5;
    }
}

void my_timer_callback( unsigned long data )
{
     /* do your timer stuff here */
  // printk(KERN_ALERT "TICK\n");
  // proc_entry_fs = filp_open(ENTRY_NAME, O_WRONLY|O_CREAT, 0644);
  // proc_buffer[0]='T';
  // proc_buffer[1]='I';
  // proc_buffer[2]='C';
  // proc_buffer[3]='K';
  // proc_buffer[4]='\n';
  // proc_buffer[5]='\0';
    //struct process_list temp_Node;
    // struct process_list *pos;
    // pos=&temp_Node;
    // list_for_each_entry( pos, &Head, link)
    // {
    //   printk (KERN_ALERT "PID =%d CPU_CLOCK=%ld " , temp_Node.PID, temp_Node.cpu_time );
    // }
    // printk (KERN_ALERT "\n");
  //write_proc(proc_entry_fs,&proc_buffer,1,NULL);
  //filp_close(proc_entry_fs,THIS_MODULE);
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
	if(flag==1){
		  proc_buffer='T';
		}else{
		  proc_buffer='K';
		}
  proc_entry_fs = filp_open(ENTRY_NAME, O_WRONLY|O_CREAT, 0644);

  write_proc(proc_entry_fs,&proc_buffer,1,NULL);
  filp_close(proc_entry_fs,THIS_MODULE);
}


// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif
   proc_dir = proc_mkdir(DIR_NAME, NULL);
   proc_entry= proc_create(ENTRY_NAME,0666, proc_dir, &entry_fops);
   if (!proc_entry) {
         return -ENOMEM;
    }
   struct process_list *current_list, *a_list;

   setup_timer(&my_timer, my_timer_callback, 0);
  /* setup timer interval to 1000 msecs */
   mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));

   flag=0;//for tesing purpose

    add_node_to_list(2);
    
    // struct process_list ss;
    // struct process_list* sss;
    // sss=&ss;
    // list_for_each_entry(sss, &Head, link)
    // {
    //   printk (KERN_ALERT "PID \n");
    // }
   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}


void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif

   remove_proc_entry(ENTRY_NAME, proc_dir);
   remove_proc_entry(DIR_NAME, NULL);
   // kfree(msg);
   del_timer(&my_timer);
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

 MODULE_LICENSE("GPL");
 module_init(mp1_init);
 module_exit(mp1_exit);


