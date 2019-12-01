/*
 * 沐阿里 3160300326
 *
 * ZJU OS 2019-2020 : Project 1 Part 2 Template
 *
 */
 
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/list.h>
 
/*
* Execute this function when the kernel module is loaded 
*/ 
static __init int init(void)
{
    struct task_struct *task,*p;  //read process information pointer
    struct list_head *pos;      //list all processes
    int running=0, interruptible=0, uninterruptible=0, zombie=0, stopped=0, dead=0, traced=0;  
    
	//Used to count various types of processes
    printk("Process Kernel module begins\n"); //start kernel module process
    task=&init_task;  //Initialization
    list_for_each(pos,&task->tasks) //loop
    {
        //The @POS pointer points to a list of all processes of type @struct task struct
        p=list_entry(pos,struct task_struct,tasks); 
        //Print current process information to buffer
        printk("name: %s\nprocess id: %d\nprocess state: %ld\nfather name: %s\n",p->comm,p->pid,p->state,p->parent->comm);
        switch(p->state){ //Count each type of process according to process status
                   case EXIT_ZOMBIE:zombie++;break; //Zombie processes
                   case EXIT_DEAD:dead++;break;
                   case TASK_RUNNING:running++;break; //Running processes
                   case TASK_INTERRUPTIBLE:interruptible++;break; //Interruptible processes
                   case TASK_UNINTERRUPTIBLE:uninterruptible++;break; //Uninterruptible processes
                   case TASK_STOPPED:stopped++;break; //Stopped processes
                   case TASK_TRACED:traced++;break; //Traced processes
                   default:break;
         }
    }
    printk("Total tasks: %d\n",running+interruptible+uninterruptible+stopped+traced+zombie+dead); //Number of all processes
    printk("TASK_RUNNING: %d\n",running); //Number of running processes
    printk("TASK_INTERRUPTIBLE: %d\n",interruptible); //Number of interruptible processes
    printk("TASK_UNINTERRUPTIBLE: %d\n",uninterruptible); //Number of uninterruptible processes
    printk("TASK_STOPPED: %d\n",stopped); //Number of stopped processes
    printk("TASK_TRACED: %d\n",traced); //Number of traced processes 
    printk("EXIT_ZOMBIE: %d\n",zombie); //Number of zombie processes
    printk("EXIT_DEAD: %d\n",dead);  //Number of dead processes
    printk("Process Kernel Module Ends\n");
    //Return value
    return 0;
}

//Function to unload the kernel module
static __exit void cleanup(void)
{
    printk("End!\n");
}

module_init(init);  //Execute init function when initializing and loading the kernel module
module_exit(cleanup);  //Execute the cleanup function when uninstalling the kernel module
