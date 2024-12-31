#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


typedef uint32_t u32;
typedef int32_t i32;


struct process
{
 u32 pid;
 u32 arrival_time;
 u32 burst_time;
 TAILQ_ENTRY(process) pointers;


 /* Additional fields here */
  //TO DO
 u32 left_to_complete;
 u32 quantum_left;
 i32 first_time_scheduled;
 /* End of "Additional fields here" */
};


TAILQ_HEAD(process_list, process); //the Head is classed proces_list

void swap(struct process* data,u32 i, u32 j){ //swap the processes for sort
  struct process temp;
  temp.pid=data[i].pid;
  temp.arrival_time=data[i].arrival_time;
  temp.burst_time=data[i].burst_time;
  data[i]=data[j];
  data[j]=temp;
 
}
//sort based on arrival times : bubble sort which is a stable sort so that if two processes arrive at the same time, the one which is at the top of the file has higher priority
void sort_data(struct process* data, u32 n){
      
    bool swapped;
    
  
    for (u32 i = 0; i < n - 1; i++) {
        swapped = false;
        for (u32 j = 0; j < n - i - 1; j++) {
            if (data[j].arrival_time > data[j + 1].arrival_time) {
                swap(data,j,j+1);
                swapped = true;
            }
        }
      
        // If no two elements were swapped, then break
        if (!swapped)
            break;
    }


}

u32 next_int(const char **data, const char *data_end)
{
 u32 current = 0;
 bool started = false;
 while (*data != data_end)
 {
   char c = **data;


   if (c < 0x30 || c > 0x39)
   {
     if (started)
     {
       return current;
     }
   }
   else
   {
     if (!started)
     {
       current = (c - 0x30);
       started = true;
     }
     else
     {
       current *= 10;
       current += (c - 0x30);
     }
   }


   ++(*data);
 }


 printf("Reached end of file while looking for another integer\n");
 exit(EINVAL);
}


u32 next_int_from_c_str(const char *data)
{
 char c;
 u32 i = 0;
 u32 current = 0;
 bool started = false;
 while ((c = data[i++]))
 {
   if (c < 0x30 || c > 0x39)
   {
     exit(EINVAL);
   }
   if (!started)
   {
     current = (c - 0x30);
     started = true;
   }
   else
   {
     current *= 10;
     current += (c - 0x30);
   }
 }
 return current;
}


void init_processes(const char *path,
                   struct process **process_data,
                   u32 *process_size)
{
 int fd = open(path, O_RDONLY);
 if (fd == -1)
 {
   int err = errno;
   perror("open");
   exit(err);
 }


 struct stat st;
 if (fstat(fd, &st) == -1)
 {
   int err = errno;
   perror("stat");
   exit(err);
 }


 u32 size = st.st_size;
 const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
 if (data_start == MAP_FAILED)
 {
   int err = errno;
   perror("mmap");
   exit(err);
 }


 const char *data_end = data_start + size;
 const char *data = data_start;


 *process_size = next_int(&data, data_end);


 *process_data = calloc(sizeof(struct process), *process_size);
 if (*process_data == NULL)
 {
   int err = errno;
   perror("calloc");
   exit(err);
 }


 for (u32 i = 0; i < *process_size; ++i)
 {
   (*process_data)[i].pid = next_int(&data, data_end);
   (*process_data)[i].arrival_time = next_int(&data, data_end);
   (*process_data)[i].burst_time = next_int(&data, data_end);
 }


 munmap((void *)data, size);
 close(fd);
}


int main(int argc, char *argv[])
{
 if (argc != 3)
 {
   return EINVAL;
 }
 //declraing the head of an array of process structs
 struct process *data; //the very first table we have not the process queue!
 u32 size;
 init_processes(argv[1], &data, &size);

 //TODO
 u32 clock=0;
 u32 process_completed=0;
 u32 process_arrived=0;


 u32 quantum_length = next_int_from_c_str(argv[2]);


 struct process_list list; //head
 TAILQ_INIT(&list); // you will be interactiing with this list is head pointer


 u32 total_waiting_time = 0;
 u32 total_response_time = 0;


 /* Your code here */
//sort processes data based on arrival time
sort_data(data,size);
 struct process* running_process=NULL;
 while(process_completed!=size){
//printf("is this %d and %d and %d clock= %d \n",size,process_completed,process_arrived,clock);
  //collect the just arrived processes and add them to the queue
    while(data[process_arrived].arrival_time==clock){
         data[process_arrived].first_time_scheduled=-1;
         data[process_arrived].left_to_complete=data[process_arrived].burst_time;
         if(data[process_arrived].burst_time>0){
         TAILQ_INSERT_TAIL(&list,&data[process_arrived],pointers);
         }
         else{
          process_completed++;
         }
         process_arrived++;
    }
   
      if(running_process==NULL){  //if nothing is running right now
          if(!TAILQ_EMPTY(&list)){
            running_process=TAILQ_FIRST(&list);
            running_process->quantum_left=quantum_length;
            if(running_process->first_time_scheduled==-1){running_process->first_time_scheduled=clock;}
            //printf("next schedule is %d and time is : %d\n",running_process->pid,clock);
            TAILQ_REMOVE(&list,running_process,pointers);
          }

       
      }

      else{    //otherwise sth is currently running
          (running_process->quantum_left)--;
          (running_process->left_to_complete)--;
          if(running_process->left_to_complete==0){ //one process completed. 
            process_completed++;

            u32 response_time=running_process->first_time_scheduled-running_process->arrival_time; //response time of this process
            u32 wait_time=(clock-running_process->arrival_time)-running_process->burst_time;//wait time of the process

            total_response_time+=response_time;
            total_waiting_time+=wait_time;

            
            running_process=NULL;

            if(!TAILQ_EMPTY(&list)){ //schedule a new process if any
              running_process=TAILQ_FIRST(&list);
              running_process->quantum_left=quantum_length;
              if(running_process->first_time_scheduled==-1){running_process->first_time_scheduled=clock;} //record response time
              TAILQ_REMOVE(&list,running_process,pointers);

            }
          }
          else if(running_process->quantum_left==0){ //time to schedule another process
              TAILQ_INSERT_TAIL(&list,running_process,pointers);
              running_process=TAILQ_FIRST(&list);
              running_process->quantum_left=quantum_length;
              if(running_process->first_time_scheduled==-1){running_process->first_time_scheduled=clock;} //record response time
              TAILQ_REMOVE(&list,running_process,pointers);
          }
      }

      clock++;

    
 }
 /* End of "Your code here" */


 printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
 printf("Average response time: %.2f\n", (float)total_response_time / (float)size);


 free(data);
 return 0;
}


//TAILQ_insert_tail(head pointeer, pointer to new element ,links) //because it is first in first out
//liks is pointers (next, and prev,  TAILQ_ENTRY(process) pointers;
//TAILQ_remove(head_pointer,pointer_to_pr)
//for traversals: TALQ_FOREACH() => traverses the list in the forward direction , https://man7.org/linux/man-pages/man3/tailq.3.html




//edge case: new process arrives at the same time another one is leaving its quantum (eg timeslice=20; process 2 arrives at 20)
//if no process is in the queu can we assume we are done? No your processeor can ve in an idle state but more processes are about to come in the future (two cases to this ):
//example: pid=0, AT=0, BT=20, pid=1 , AT=30, BT=20 . your queue will be empty between t=20 and t=30 but that does not mean you are done
//u32 process_complemeted to keep track of stopping condition. idle period can also be at the beginning. PID=0, AT=10,
//



//TAIL1_INSERT_TAIL(&list,new,pointers);

