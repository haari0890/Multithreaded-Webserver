#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<pthread.h>
#include<dirent.h>
#include<pwd.h>


char *filename;
int listenfd = 0, a=0, flag_head = 0, time_wait = 60, log_flag = 0, num_thread = 4, dir_flag = 0;
int scheduler_flag = 0;
char debugger_flag[10]="N"; 
char *directory_work;
struct ready_details
{
int accept_desc;
int file_size;
char file_name[100];
char request_time[50];
char buffer_client[100];
char req[100];
char client_add[200];
}ready;

struct ready_queue
{
struct ready_details queue;
struct ready_queue *next;
} *new, *temp, *front = NULL, *rear = NULL;



time_t time_now;
struct tm *local;
char buff[200], client_address[200];
char req_rec_time[100], req_exe_time[100];

void insert_queue(int a, int b, char filename[100], char time_req[100], char buff[100], char req_type[100], char cli_addr[200])
{
new = (struct ready_queue*) malloc(sizeof(struct ready_queue));

new->queue.accept_desc = a;
new->queue.file_size = b;
strcpy(new->queue.file_name, filename);
strcpy(new->queue.request_time, time_req);
strcpy(new->queue.buffer_client, buff);
strcpy(new->queue.req, req_type);
strcpy(new->queue.client_add, cli_addr);
if(front == NULL)
{
front = (struct ready_queue*) malloc(sizeof(struct ready_queue));
front = new;
rear = front;
}
else
{
rear->next = new;
rear = new;
}
}

struct ready_details remove_queue_FCFS()
{
struct ready_details item;
item.accept_desc = front->queue.accept_desc;
item.file_size = front->queue.file_size;
strcpy(item.file_name,front->queue.file_name);
strcpy(item.request_time,front->queue.request_time);
strcpy(item.buffer_client,front->queue.buffer_client);
strcpy(item.req,front->queue.req);
strcpy(item.client_add,front->queue.client_add);
struct ready_queue *queue_item;
queue_item = (struct ready_queue*) malloc(sizeof(struct ready_queue));
queue_item = front;
if(front != rear)
front = front->next;
else if(front == rear)
{
front = NULL;
}
//free(queue_item);
return item;
}

struct ready_details remove_queue_SJF(int input_desc)
{
struct ready_details item;
struct ready_queue *SJF_items, *SJF_items1;
SJF_items = (struct ready_queue*) malloc(sizeof(struct ready_queue));
SJF_items1 = (struct ready_queue*) malloc(sizeof(struct ready_queue));
SJF_items = front;
if(front == rear)
{
item = remove_queue_FCFS();
return item;
}
else
{
while(SJF_items)
{
if(SJF_items->queue.accept_desc == input_desc)
{
if(SJF_items == front)
front = SJF_items->next;
else
SJF_items1->next=SJF_items->next;
item.accept_desc = SJF_items->queue.accept_desc;
item.file_size = SJF_items->queue.file_size;
strcpy(item.file_name,SJF_items->queue.file_name);
strcpy(item.request_time,SJF_items->queue.request_time);
strcpy(item.buffer_client,SJF_items->queue.buffer_client);
strcpy(item.req,SJF_items->queue.req);
strcpy(item.client_add,front->queue.client_add);
if(SJF_items == rear)
rear = SJF_items1;
return item;
}
else
{
SJF_items1 = SJF_items;
SJF_items = SJF_items->next;
}
}
}
}

void logging(char flag[10], int conn, int status, struct ready_details item1)
{
//printf("logging");
char text[1000];
char buff_time[100];
time_t time_now;
struct tm *local;
time(&time_now);
local = gmtime(&time_now);
strftime(buff_time, 25 , "%d/%b/%Y:%H:%M:%S", local);
//struct ready_details item1;
struct ready_queue *items;
items = (struct ready_queue*) malloc(sizeof(struct ready_queue));
items = front;
//printf("%d!!!!!!!!!!!",conn);
/*while(items)
{
//printf("%d$$$$$$$$$$$$44",items->queue.accept_desc);
if(items->queue.accept_desc == conn)
{
item1.accept_desc = items->queue.accept_desc;
strcpy(item1.file_name,items->queue.file_name);
item1.file_size = items->queue.file_size;
strcpy(item1.request_time, items->queue.request_time);
strcpy(item1.buffer_client, items->queue.buffer_client);
strcpy(item1.client_add,items->queue.client_add);
printf("%d!!!!%d",conn, item1.accept_desc);
//break;
}
items = items->next;
}*/
if(strcmp(flag,"D") == 0)
{
printf("\n");
printf("%s - [%s -600] [%s -600] \n \"%s  HTTP/1.0\" %d %d", item1.client_add, item1.request_time, buff_time, item1.buffer_client, status, item1.file_size);
//printf("\n%s \t %s",buff_time,text);
printf("\n");
}
else
{
//char filename[100] = "/home/trinity/Desktop/Log.txt";
FILE *fp;
fp = fopen(filename,"a");
if(fp<0)
fp = fopen(filename,"w+");
fprintf(fp,"\n%s - [%s -600] [%s -600] \n \" %s  HTTP/1.0\" %d %d\n", item1.client_add, item1.request_time, buff_time, item1.buffer_client, status, item1.file_size);
fflush(fp);
fclose(fp);
}
}

void display_directory(char name[100], int connfd)
{
FILE *fp;
char message_client[10000];
DIR *directory = opendir(name);
struct dirent **files;
int n = scandir(name,&files, 0, alphasort);
int flag1 = 0;
char message[100000];
memset(message,'\0',100000);
if(n<=0)
{
strcpy(message,"404 The File is not found");
}
if(n>0)
{
int i=0,f=0;
for(i=0;i<n;i++)
{
strcpy(message_client,files[i]->d_name);
strcat(message_client, "\n");
for(f=0;f<3;f++)
{
if(message_client[f]!='.')
{
if(strstr(message_client,"."))
{
if(message_client[0] == '.' || message_client[1] == '.')
continue;
strcat(message,files[i]->d_name);
strcat(message,"\n");
}
//printf("~~~%s~~~\n",message_client);
break;

}

}

flag1 = 0;
free(files[i]);
}
write(connfd,message, strlen(message));
free(files);
}
}

//Queue printing
void printing()
{
temp = front;
while(temp)
{
printf("%d\n%d\n%s\n%s\n%s\n%s\n%s",temp->queue.accept_desc, temp->queue.file_size, temp->queue.file_name, temp->queue.request_time, temp->queue.buffer_client, temp->queue.req, temp->queue.client_add);
temp=temp->next;
}
}



void *thread_queue()
{
struct sockaddr_in client_addr;
socklen_t client_len;
if(listen(listenfd, 10) == -1)
{
 printf("Failed to listen\n");
 exit(1);
}
int n = 0;

int i = 0;
int l;
int connfd;
char string1[1200],string2[1200];
char string3[1200];
while(1)
{
connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);
//printf("%d",connfd);
time(&time_now);
local = gmtime(&time_now);
strftime(req_rec_time, 25 , "%d/%b/%Y:%H:%M:%S", local);
if (connfd < 0)
error("ERROR on accept");
strcpy(client_address,inet_ntoa(client_addr.sin_addr));
n = read(connfd,buff,200);
sscanf(buff,"%s %s",string1,string2);
fflush(stdout);
char string5[2000];
strcpy(string5, " ");
strcat(string5,string1);
strcat(string5, " ");
strcat(string5,string2);
//printf("%s, %s", string1, string2);
//printf("%s\n",client_address);
//int dir_flag = 0;
//char *login_name;
//login_name = getlogin();
l = strlen(string2);
if(strstr(string2,"~")>0)
{
char user[2000];
memset(user,'\0',2000);
//strcpy(user, " ");
strcpy(user,string2);
int i;
int pos1 =0, pos2 = 0;
int len =strlen(user);
if(user[0] == '/')
{
for(i=0;i<len;i++)
{
user[i] = user[i+1];
}
}
for(i=0;i<len;i++)
{
if(user[i]=='~')
{
pos1 = i;
}
if(user[i]=='/')
{
pos2 = i;
if((pos1+1) == pos2 || (pos1-1) == pos2)
{
pos1++;
continue;
}
break;
}
pos2 = i+1;
}
//printf("%d,%d",pos1,pos2);
char file[100];
int j=0;
memset(file,'\0',100);
for(i=(pos1+1);i<pos2;i++)
{
file[j] = user[i];
j++;
}
file[j] = '\0';
//printf("%s",file);
char *username;
struct passwd *pwd;

username = file;
pwd = getpwnam((const char*) username);
char user_name[1000];
memset(user_name,'\0',1000);
strcpy(user_name,pwd->pw_dir);
strcat(user_name,"/myhttpd");
//printf("%s",user_name);
int l = strlen(user_name);

for(i=pos2;i<len;i++)
{
//strcat(user_name,file[i]);
//printf("%d%c\t",pos2,user[i]);
user_name[l] = user[i];
l++;
}
strcpy(string3,user_name);
printf("%s",string3);

/*
char directory_info[100] = "/home/";
//strcat(directory_info, login_name);
//strcat(directory_info,"/httpd/");
for(i=1;i<=l;i++)
{
string2[i-1] = string2[i];
}

//strcat(directory_info,"/");
strcat(directory_info,string2);
strcat(directory_info,"/Desktop/Read1.html");
printf("%s",directory_info);
strcpy(string3,directory_info);*/


/*int ret = 0;
struct stat buf;
ret = stat(string3, &buf);
int debugger_listen=0;
if(ret<0)
{
insert_queue(connfd,0,string3,req_rec_time,string5, string1,client_address);
debugger_listen++;
continue;
}
else
{
insert_queue(connfd,buf.st_size,string3,req_rec_time,string5,string1,client_address);
debugger_listen++;
continue;
}*/
}else
{
if(strstr(string2,"/")>0)
{
for(i=1;i<=l;i++)
{
string3[i-1] = string2[i];
}
//strcpy(string2[i-1],string3[i]);
}
else
{
strcpy(string3,string2);
}
if(strstr(string3,"/")>0)
{
//int len = 0;
//len = strlen(string3);
strcpy(string3,string2);
//if(strcmp(string3[len],"/") == 0)
//{
//dir_flag = 1;
//printf("%d",dir_flag);
//}
}
}
//insert into the queue
int ret = 0;
struct stat buf;
ret = stat(string3, &buf);
int debugger_listen=0;
if(ret<0)
{
insert_queue(connfd,0,string3,req_rec_time,string5, string1,client_address);
debugger_listen++;
}
else
{
if(!strcasecmp(string1,"get"))
{
insert_queue(connfd,buf.st_size,string3,req_rec_time,string5,string1,client_address);
debugger_listen++;
}else
{
insert_queue(connfd,0,string3,req_rec_time,string5,string1,client_address);
debugger_listen++;
}
}
//printing();
a++;
/*if(!strcmp(debugger_flag, "D"))
{
if(debugger_listen == 1)
pthread_exit(0);
}*/

}
}

int c=1;
int b=0;
//int values = 0;
//int threads = 1;

void *thread_execute()
{
sleep(time_wait);
//printf("%d",(int)pthread_self());
char string1[1200], string3[1200];
int connfd;
struct ready_details it;
FILE *fp;
char message_client[1000];
//char message1[10000];
while(1)
{
//memset(message1,'\0',100000);
while(b==0);
//{
//printf("%d",(int)pthread_self());
//if(threads==0)
//{
//continue;
//}
//}
//threads--;
b--;
it = ready;
c++;
//num_thread--;
//threads++;
//printf("%s",it.file_name);
strcpy(string3,it.file_name);
strcpy(string1,it.req);
connfd = it.accept_desc;
int len = 0;
len = strlen(it.file_name);
int debugger_execute = 0;
char something[1000];
int i = 0;
for(i=0;i<len;i++)
{
if(i==(len-1))
something[0] = it.file_name[i];
}
something[1] = '\0';
//printf("%s",something);
if(strcmp(something,"/") == 0)
{
//printf("in here");
//printing();
char some_file[1000];
strcpy(some_file,string3);
strcat(some_file,"index.html");
fp = fopen(some_file,"r");
if(fp == 0)
{
//printf("in here no file index");
display_directory(it.file_name, connfd);
if(log_flag == 1 || !strcmp(debugger_flag,"D"))
logging(debugger_flag,connfd,404,ready);
debugger_execute++;
}
else
{
//printf("i m here !!%s", some_file);
if(log_flag == 1 || !strcmp(debugger_flag,"D"))
logging(debugger_flag,connfd,200,ready);
int n = 0;
if(!strcasecmp(string1,"get"))
{
char message[2000];
time(&time_now);
local = gmtime(&time_now);
strftime(message, 25 , "%d/%b/%Y:%H:%M:%S\n", local);
write(connfd,message,strlen(message));
char hostname[1024];
hostname[1023] = '\0';
gethostname(hostname, 1023);
strcpy(message,hostname);
write(connfd,message,strlen(message));
strcpy(message," HTTP /1.0 \n");
write(connfd,message,strlen(message));
char *filename_dot;
filename_dot = string3;
if(strstr(filename_dot, ".txt") != NULL || strstr(filename_dot, ".html") != NULL)
{
strcpy(message, "Text/Html\n");
write(connfd,message,strlen(message));
}else
{
strcpy(message, "Image/jpg\n");
write(connfd,message,strlen(message));
}
int r = 0;
struct stat file_info;
r = stat(some_file, &file_info);
sprintf(message, "%d\n ", it.file_size);
write(connfd, message, strlen(message));
strcpy(message, ctime(&(file_info.st_mtime)));
write(connfd, message, strlen(message));
while(fgets(message_client,200,fp))
{
	//printf("%s\na.txt",str2);
	//strcat(buffer2,str2);
	n = write(connfd,message_client,strlen(message_client));
}

debugger_execute++;
}
else
{
char message[200];
time(&time_now);
local = gmtime(&time_now);
strftime(message, 25 , "%d/%b/%Y:%H:%M:%S", local);
write(connfd,message,strlen(message));
strcpy(message," HTTP /1.0 ");
write(connfd,message,strlen(message));
char *filename_dot;
filename_dot = string3;
if(strstr(filename_dot, ".txt") != NULL || strstr(filename_dot, ".html") != NULL)
{
strcpy(message, "Text/Html\n");
write(connfd,message,strlen(message));
}else
{
strcpy(message, "Image/jpg\n");
write(connfd,message,strlen(message));
}
int r = 0;
struct stat file_info;
r = stat(string3, &file_info);
sprintf(message, " %d ", it.file_size);
write(connfd, message, strlen(message));
strcpy(message, ctime(&(file_info.st_mtime)));
write(connfd, message, strlen(message));
debugger_execute++;
}

}
}
else
{
//printf("\n\n\n\n%s",string3);
fp = fopen(string3,"r");

if(fp == 0)
{
char no_file[1000];
for(i=0;i<len;i++)
{
no_file[i] = it.file_name[i];
}
no_file[i+1] = '\0';
int pos = 0;
for(i=len;i>=0;i--)
{
//printf("%s",&no_file[i]);
if(no_file[i] == '/')
{
pos = i;
break;
}
}
if(pos == 0 || pos == 1)
{
display_directory(".", connfd);
if(log_flag == 1 || !strcmp(debugger_flag,"D"))
logging(debugger_flag,connfd,404,ready);
close(connfd);
debugger_execute++;
if(!strcmp(debugger_flag, "D"))
{
if(debugger_execute == 1)
exit(0);
}
continue;
}
pos++;
char dir_name[1000];
for(i = 0; i<pos;i++)
{
dir_name[i] = no_file[i];
}
char index_file[1000];
strcpy(index_file,dir_name);
strcat(index_file,"/index.html");
//printf("%s",index_file);
//fp = fopen(index_file,"r");
//if(fp == 0)
//{
display_directory(dir_name, connfd);
if(log_flag == 1 || !strcmp(debugger_flag,"D"))
logging(debugger_flag,connfd,404,ready);
debugger_execute++;
//}
}
else
{
if(log_flag == 1 || !strcmp(debugger_flag,"D"))
logging(debugger_flag,connfd,200,ready);
int n = 0;
if(!strcasecmp(string1,"get"))
{
char message[2000];
time(&time_now);
local = gmtime(&time_now);
strftime(message, 25 , "%d/%b/%Y:%H:%M:%S\n", local);
write(connfd,message,strlen(message));
char hostname[1024];
hostname[1023] = '\0';
gethostname(hostname, 1023);
strcpy(message,hostname);
write(connfd,message,strlen(message));
strcpy(message," HTTP /1.0 \n");
write(connfd,message,strlen(message));
char *filename_dot;
filename_dot = string3;
if(strstr(filename_dot, ".txt") != NULL || strstr(filename_dot, ".html") != NULL)
{
strcpy(message, "Text/Html\n");
write(connfd,message,strlen(message));
}else
{
strcpy(message, "Image/jpg\n");
write(connfd,message,strlen(message));
}
int r = 0;
struct stat file_info;
r = stat(string3, &file_info);
FILE *f;
f = fopen(string3,"r");
int size_file =0;
fseek(f,0L,SEEK_END);
size_file = ftell(f);
sprintf(message, "%d\n ", size_file);
write(connfd, message, strlen(message));
strcpy(message, ctime(&(file_info.st_mtime)));
write(connfd, message, strlen(message));
while(fgets(message_client,200,fp))
{
	//printf("%s\na.txt",str2);
	//strcat(buffer2,str2);
	n = write(connfd,message_client,strlen(message_client));
}

debugger_execute++;
}
else
{
char message[200];
time(&time_now);
local = gmtime(&time_now);
strftime(message, 25 , "%d/%b/%Y:%H:%M:%S", local);
write(connfd,message,strlen(message));
strcpy(message," HTTP /1.0 ");
write(connfd,message,strlen(message));
char *filename_dot;
filename_dot = string3;
if(strstr(filename_dot, ".txt") != NULL || strstr(filename_dot, ".html") != NULL)
{
strcpy(message, "Text/Html\n");
write(connfd,message,strlen(message));
}else
{
strcpy(message, "Image/jpg\n");
write(connfd,message,strlen(message));
}
//char *e = strchr(filename_dot,".");
//strcpy_s(message,1000,e);
//strcpy(message,e);
//printf("!!!!!!!!%s!!!!!!!!!%s",message,e);
//strcpy(message,e);

int r = 0;
struct stat file_info;
r = stat(string3, &file_info);
FILE *f;
f = fopen(string3,"r");
int size_file =0;
fseek(f,0L,SEEK_END);
size_file = ftell(f);
sprintf(message, " %d ", size_file);
write(connfd, message, strlen(message));
strcpy(message, ctime(&(file_info.st_mtime)));
write(connfd, message, strlen(message));
debugger_execute++;
}
}
}
//num_thread++;
close(connfd);
if(!strcmp(debugger_flag, "D"))
{
if(debugger_execute == 1)
exit(0);
}
}
}

int debugger_schedule = 1;
void *thread_schedule()
{
//printing();
//printf("in scheduler");
sleep(time_wait);
//printf("%s",ready.file_name);
//printf("in here");
//int debugger_schedule = 0;
while(1)
{
if(scheduler_flag == 0)
{
//FCFS
if(a>0)
{
a--;
//while(num_thread == 0);
while(c==0);
strcpy(ready.buffer_client," ");
ready = remove_queue_FCFS();
c--;
b++;
debugger_schedule++;
}
}
else
{
//printf("sjf");
if(a>0)
{
a--;
struct ready_queue *find_min;
find_min = (struct ready_queue*) malloc(sizeof(struct ready_queue));
find_min = front;
int min;
int desc_min = 0;
min = find_min->queue.file_size;
desc_min = find_min->queue.accept_desc;
//printf("%d",min);
if(front != rear)
{
while(find_min)
{
//printf("in loop");
if(min>find_min->queue.file_size)
{
//printf("in if");
desc_min = find_min->queue.accept_desc;
min = find_min->queue.file_size;
printf("%d",min);
}
find_min = find_min->next;
}
}//printf("i m here");
while(c==0);
ready = remove_queue_SJF(desc_min);
c--;
b++;
debugger_schedule++;

}
//values++;
}

if(!strcmp(debugger_flag, "D"))
{
if(debugger_schedule == 2)
sleep(500);
}
}
}






void main(int argc, char *args[])
{
int i = 1;
char *schedule = "FCFS";
int port_num = 8080;
char logging_text[100];
for (i=1; i<argc; i++)
{
//printf("%s\n",args[i]);
if(strcmp(args[i], "-d") == 0 || strcmp(args[i], "-D") == 0)
{
strcpy(debugger_flag,"D");
//logging("Debugger mode started \n Logging disabled", debugger_flag);
}
else if(strcmp(args[i], "-h") ==0 || strcmp(args[i], "-H") ==0)
{
printf("Displaying the help menu\n");
//strcpy(logging_text,"Help requested. Displaying help to the user");
printf("Help: \n Options are \n -d: To enter Debugging Mode \n -h: For help \n -l Filename: to log in the file \n -p portnum: To listen to the port default 8080\n -r dir: to change the working directory to dir \n -t time: Time to wait for scheduling default 60 Sec \n -n Threadnum: Number of threads to create default 4 \n -s sched: Scheduling policy default FCFS \n");
exit(1);
}
else if(strcmp(args[i], "-l") == 0 || strcmp(args[i], "-L") ==0)
{
//printf("File Description\n");
filename = args[i+1];
i++;
log_flag = 1;
printf("%s",filename);
//logging("Logging into File", debugger_flag);
}
else if(strcmp(args[i], "-p") ==0 || strcmp(args[i], "-P") ==0)
{
//printf("Getting the port number\n");
port_num = atoi(args[i+1]);
i++;
}
else if(strcmp(args[i], "-r") ==0 || strcmp(args[i], "-R") ==0)
{
//printf("The working directory is %s \n", args[i+1]);
directory_work = args [i+1];
dir_flag = 1;
i++;
chdir(directory_work);
}
else if(strcmp(args[i], "-t") ==0 || strcmp(args[i], "-T") ==0)
{
//printf("The time for the queue to wait\n");
time_wait = atoi(args[i+1]);
i++;
printf("%d",time_wait);
}
else if(strcmp(args[i], "-n") ==0 || strcmp(args[i], "-N") ==0)
{
num_thread = 4;
num_thread = atoi(args[i+1]);
printf("%d",num_thread);
}
else if(strcmp(args[i], "-s") ==0 || strcmp(args[i], "-S") ==0)
{
//printf("Scheduling type\n");
schedule = args[i+1];
if(!strcasecmp(schedule,"sjf"))
scheduler_flag = 1;
//printf("\n%d\nflag",scheduler_flag);
//logging("Scheduling", debugger_flag);
}
}
pid_t pid1, pid2;
if(strcmp(debugger_flag,"D"))
{
pid1 = fork();
if(pid1>0)
{
exit(0);
}
umask(0);
pid2 = setsid();
close(0);
close(1);
close(2);
//int j = 0;
//for(j = sysconf(_SC_OPEN_MAX);j>0;j--)
//close(j);
}

pthread_t thread_server[num_thread], scheduler_thread, queuing_thread;
for(i =0;i<num_thread;i++)
{
pthread_create(&thread_server[i],NULL,&thread_execute,NULL);
}
//pthread_create(&thread_server,NULL,&thread_execute,NULL);
pthread_create(&scheduler_thread,NULL,&thread_schedule,NULL);
pthread_create(&queuing_thread,NULL,&thread_queue,NULL);

//Socket Code taken from http://www.codeproject.com/Articles/586000/Networking-and-Socket-programming-tutorial-in-C 
  struct sockaddr_in serv_addr, client_addr;
  socklen_t client_len;
  char sendBuff[1025];  
  int numrv;  
  client_len = sizeof(client_addr);
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket retrieve success\n");
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(&client_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));
  serv_addr.sin_family = AF_INET;    
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  serv_addr.sin_port = htons(port_num); 
  bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
// End of code 
pthread_join(queuing_thread, NULL);
//pthread_join(thread_server, NULL);
pthread_join(scheduler_thread, NULL);
for(i =0;i<num_thread;i++)
{
pthread_join(thread_server[i], NULL);
}
}


