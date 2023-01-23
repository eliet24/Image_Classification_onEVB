/*************  includes     *****************/
#include <fcntl.h>           
#include <sys/stat.h>        
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h> // read(), write(), close()
#include <netdb.h>
#include "uart.h"

#define MAX 1024
#define PORT 5000
#define IMAGE_SIZE 1024
#define QUEUE_SIZE 50
#define SA struct sockaddr
#define SERIAL_DEV "/dev/ttyS2"

pthread_mutex_t mutex1;   //mutex for synch
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
//char img_pop[1024];
int images_num = 0;
char *start = NULL;  //pointer to start of the queue
pthread_mutex_t lock;
static char queue[1024*3];
void init_shm(char* start){
    for(int i = 0; i < IMAGE_SIZE*QUEUE_SIZE; i++)
        *(start+i) = 0;
} 
void print_img_inqueue(char *start, int num)
{
    for (int i = 0; i<IMAGE_SIZE*num; i++){
            printf("%c",*(start+i));  //printing 1024 char values of img
        }
}

char* insert_image(char image[],char* start)
{
    if(images_num<50)
    {
        for(int i = images_num*IMAGE_SIZE; i>0;i--)
        {
            *(start+i+IMAGE_SIZE)=*(start+i-1);
        }
        for(int i=0; i<1024;i++){
            *(start+i)=image[i];
        }
        printf("image inserted to queue\n");
        images_num++;
        return start;
    } 
    else{
        printf("Queue is full");
        char to_client[15];
        strcpy(to_client,"Queue is Full");
        //write(connfd, to_client, sizeof(to_client));
        return 0; 
    }
}

// pop elements from queue
int pop_image(char *start)
{
    int image_off;
    if(images_num == 0)
        printf("queue is empty\n");
    else
    {
        images_num--;
        printf("\n image pulled from queue\n");
        image_off = IMAGE_SIZE*(images_num);  
        return image_off;
    }
    return -1;
}

/// creating shared memory   //make function!
char * creat_shared_memory()
{
    int shmid;
    key_t key = 6678;
    char *shm;
    char *mem_start;
    //Image* end; 
    
    printf("main started\n");
    
    //Create the segment.
    if ((shmid = shmget(key, IMAGE_SIZE*QUEUE_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    //Now we attach the segment to our data space.
    if ((shm =shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    printf("server attached to memory %p\n",shm);
    mem_start =shm;
    return mem_start;
}


//creating socket and waiting to connection of clients
int socket_manager()
{
    int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        herror("setsockopt(SO_REUSEADDR) failed");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        herror("setsockopt(SO_REUSEADDR) failed");
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");
    return connfd;
}

int main()
{
    // img_pop[1024];
    int image_offset;
    pid_t pid1;  //processes 
    pid_t pid2;
    pid_t pid3;
    pthread_mutex_init(&mutex1,NULL);
    pthread_mutex_init(&mutex2,NULL);
    pthread_mutex_init(&mutex3,NULL);
    int status;
    int pipefds2[2]; //pipes
    int pipefds3[2];
    int pipefds1[2];
    char write_msg='1';  //message for pipe
    char close_entry ='0';

    char read_msg1; //read from pipes
    char read_msg2; 
    char * read_msg3; 

    srand(time(0));
    int result;
    time_t rawtime;             //time varibles
    struct tm * timeinfo;
    FILE *log;                 //creating log file
    if (log == NULL){
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        }  
    start = creat_shared_memory();     //creating shared memory
    init_shm(start);
    char img1[1024]; //created for self check of the queue
    char img2[1024]; 
    char img3[1024]; 
    char class_id[6];
    //int connfd = socket_manager();

    // chat between client and server-->adding images data to queue-->process image-->send answer to client
	char buff[MAX];
    char to_client[]="starting image processing";
    ////Queue checking////
    for(int k = 0; k<1024; k++)
        *(img1+k) = '1';
    for(int k = 0; k<1024; k++)
        *(img2+k) = '2';
    for(int k = 0; k<1024; k++)
        *(img3+k) ='3';
    start = insert_image(img1,start);
    start = insert_image(img2,start);
    start = insert_image(img3,start);
    ////pipeline code////
    pipe(pipefds1);
    pipe(pipefds2);
    pipe(pipefds3);
    write(pipefds1[1], &close_entry, sizeof(close_entry));
    write(pipefds2[1], &close_entry, sizeof(close_entry));
    write(pipefds3[1], &close_entry, sizeof(close_entry));

    pid1 = fork();
    if (pid1 < 0) {
        perror("Pipe Error");
        exit(-1);
        }
    if (pid1 == 0) 
    {
        while(images_num > 0)
        {
            //Child 1                          //child1 process-FFT
            pthread_mutex_lock(&mutex1); 
            image_offset = pop_image(start); //fft process-pop image from queue 
            for(int j=0; j<1024;j++)  
                printf("%c ",*(start+image_offset+j));       
            for(int i=0; i<1024;i++){
                *(start+image_offset+i) = (char)((int)(*(start+image_offset+i)-'0')*2); 
                printf("%d ", *(start+image_offset+i));
            }           
            printf("ENTERING FFT PROCESS...\n");
            write(pipefds1[1], &write_msg, sizeof(write_msg));
            printf("Writing %c to pipe - from FFT to CONV \n", write_msg);
            pthread_mutex_unlock(&mutex1);
        }
    }
    else
    {
        pid2 = fork();
        if (pid2 < 0) {
            perror("Pipe Error");
            exit(1);
        }
        if (pid2 == 0) 
        {
            //Child 2 process-CONV
            printf("CONV PROCESSING\n");
            while(images_num > 0)
            {
                read(pipefds1[0], &read_msg1, sizeof(read_msg1));
                printf(" CONV read msg::::: %c\n",read_msg1);
                if(read_msg1=='1')
                {
                    write(pipefds1[1], &close_entry, sizeof(close_entry));
                    printf("ENTERING CONV PROCESS...\n");
                    pthread_mutex_lock(&mutex2);
                    for(int i=0; i<1024;i++){
                        *(start+image_offset+i) = ((int)(*(start+image_offset+i)-'0')+1);
                        //printf("%d ", *(start+image_offset+i));
                    }
                    printf("Writing %c to pipe - from conv to ID \n", write_msg);
                    write(pipefds2[1], &write_msg, sizeof(write_msg));
                    pthread_mutex_unlock(&mutex2);
                }
            }   
        }
        else
        {
            pid3 = fork();
            if (pid3 < 0) {
            perror("Pipe Error");
            exit(-1);
            }
            if (pid3 == 0) 
            {
                printf("ID PROCESSING\n");
                ///child 3 process-ID
                while(images_num > 0)
                {
                    read(pipefds2[0], &read_msg2, sizeof(read_msg2));
                    printf(" ID read msg::::: %c\n",read_msg2);
                    if(read_msg2=='1')
                    {
                        write(pipefds2[1], &close_entry, sizeof(close_entry));
                        printf("ENTERING ID PROCESS...\n");
                        pthread_mutex_lock(&mutex3);
                        result = rand() % 3;
                        printf("send from Id to client through uart the identification is: ");      //send the result
                        if(result==0){strcpy(class_id, "mouse");
                        //send(connfd, class_id, 6,0);
                        printf("%s\n",class_id);
                        }
                        if(result==1){strcpy(class_id, "cat");
                        //send(connfd, class_id, 4,0);
                        printf("%s\n",class_id);
                        }
                        if(result==2){strcpy(class_id, "dog");
                        //send(connfd, class_id, 4,0);
                        printf("%s\n",class_id);
                        }
                        //for(int j=0; j<1024;j++)
                            //printf("%d  ",(int)img_pop[j]);
                            //printf("%d ",(img_pop[j]));
                        write(pipefds3[1],  &write_msg, sizeof(write_msg));
                        pthread_mutex_unlock(&mutex3); 
                        //uart_sender(class_id);
                    }
                }
            }
            else
            {
                //for(int j=0; j<1024;j++)
                    //printf("%d  ",(int)img_pop[j]);
                    //printf("%c ",img_pop[j]);
                //printf("%s", img_pop);
                pthread_mutex_destroy(&mutex1);
                pthread_mutex_destroy(&mutex2);
                pthread_mutex_destroy(&mutex3);
            }

        } 
    }
    return 0; 
}


