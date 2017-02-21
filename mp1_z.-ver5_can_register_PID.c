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

 static struct proc_dir_entry* proc_dir;
 static struct proc_dir_entry* proc_entry;
 static char msg[10];

 size_t num_byte_from_user;


 static struct timer_list my_timer;
 //static char proc_buffer;
 struct file *proc_entry_fs;

struct process_list{
    struct list_head link;
    long cpu_time;
    int PID;
};

static LIST_HEAD(Head);
static struct work_struct update;
static DECLARE_WORK(update, mykmod_work_handler);

int len,temp,flag;


 ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
//	loff_t pos=0;
	
//   if(count>temp)//why do we need these conditions to make "cat" print only once
// {
// 	count=temp;
// }
// temp=temp-count;
// copy_to_user(buf,msg, count);
// if(count==0)
// temp=len;
	//count= vfs_read(file, msg, num_byte_from_user, &pos)
	
return count;

}


ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
long user_PID;
copy_from_user(msg,buf,count);

//printk(KERN_INFO "In write");
len=count;  // why use these variables?
temp=len;
kstrtol(msg,0,&user_PID);
printk(KERN_ALERT "user_PID=%ld \n", user_PID);
num_byte_from_user = (size_t)count;

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
    struct process_list *newNode;
  	newNode = kmalloc(sizeof(struct process_list), GFP_KERNEL);
    (*newNode).cpu_time=0;
    (*newNode).PID=PID;          //For testing purpose, set PID to 2. Make it the value received from user space.
    list_add(&((*newNode).link), &Head);
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
	// if(flag==1){
	// 	flag=0;
	// }else{
	// 	flag=1;
	// }
  schedule_work(&update);
  mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
} 

static void mykmod_work_handler(struct work_struct *pwork)
{
	// if(flag==1){
	// 	  proc_buffer='T';
	// 	}else{
	// 	  proc_buffer='K';
	// 	}
   proc_entry_fs = filp_open(ENTRY_NAME, O_WRONLY|O_CREAT, 0644);

  // write_proc(proc_entry_fs,&proc_buffer,1,NULL);
   filp_close(proc_entry_fs,THIS_MODULE);
}

// void cleanup_list(void) {
//   struct process_list *current, *next;
//   struct list_head* temp_listHead;
//   printk(KERN_ALERT "Cleaning up list safely");
//   temp_listHead = &Head;

//   // for(temp_listHead = &Head; temp_listHead!= &Head; temp_listHead= (*temp_listHead).next)// post increament could be problematic
//   // {

//   // }

//   // list_for_each_entry(current, &Head, link){
//   //   printk(KERN_INFO "freeing node %dn", current->PID);
//   //   list_del(&((*current).link));
//   //   kfree(current);
//   // }
// }

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{

   //declaration
   //struct list_head* second_list_head;
   struct process_list* second_struct;
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
  /* setup timer interval to 1000 msecs */
   mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
  // msg=kmalloc(GFP_KERNEL,10*sizeof(char));

   flag=0;//for tesing purpose

    add_node_to_list(2);
    
    //second_list_head=Head.next;
    second_struct=container_of( Head.next, struct process_list, link);
    //printk (KERN_ALERT "PID= %d\n", (*second_struct).PID);

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
   // Insert your code here ...
   remove_proc_entry(ENTRY_NAME, proc_dir);
   remove_proc_entry(DIR_NAME, NULL);
   // kfree(msg);
   del_timer(&my_timer);
   //cleanup_list();
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

 MODULE_LICENSE("GPL");
 module_init(mp1_init);
 module_exit(mp1_exit);


