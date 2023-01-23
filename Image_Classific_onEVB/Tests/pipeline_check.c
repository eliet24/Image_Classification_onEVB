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

#define MAX 1024
#define PORT 5000
#define IMAGE_SIZE 1024
#define QUEUE_SIZE 50
#define SA struct sockaddr

pthread_mutex_t mutex1;   //mutex for synch
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
char img_pix[1024];
char img_pop[1024];
int images_num=0;
char *start = NULL;  //pointer to start of the queue
pthread_mutex_t lock;

void init_shm(char* start){
    for(int i = 0; i < IMAGE_SIZE*QUEUE_SIZE; i++)
        *(start+i) = -1;
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

// delete elements from queue
char* pop_image(char *start,char image[1024])
{
    int j = 0;
    for(int i = IMAGE_SIZE*(images_num-1); i<IMAGE_SIZE*images_num; i++)
    {
        image[j] = *(start+i);
        j++;
        *(start+i) = -1;
    }
    images_num--;
    printf("\n image pulled from queue\n");
    return image;
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
    pid_t pid;  //processes 
    pid_t pid2;
    pid_t pid3;
    pthread_mutex_init(&mutex1,NULL);
    pthread_mutex_init(&mutex2,NULL);
    pthread_mutex_init(&mutex3,NULL);
    int status;
    int pipefds2[2]; //pipes
    int pipefds3[2];
    int pipefds1[2];
    char write_msg[]="1";  //message for pipe

    char *read_msg1; //read from pipes
    char *read_msg2; 
    char *read_msg3; 
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
    char *class_id;
    //int connfd = socket_manager();

    // chat between client and server-->adding images data to queue-->process image-->send answer to client
	char buff[MAX];
    char to_client[]="starting image processing";
    //Queue checking////////
    for(int k = 0; k<1024; k++)
        *(img1+k) = 1;
    for(int k = 0; k<1024; k++)
        *(img2+k) =1;
    for(int k = 0; k<1024; k++)
        *(img3+k) =1;
    start = insert_image(img1,start);
    start = insert_image(img2,start);
    start = insert_image(img3,start);

    strcpy(img_pop,pop_image(start, img_pop));
        /////////////////////
        //pipeline code
    pipe(pipefds1);
    pipe(pipefds2);
    pipe(pipefds3);

        pid = fork();
        if (pid < 0) {
            perror("Pipe Error");
            exit(-1);
            }
        if (pid == 0) 
        {
            printf("FFT PROCESSING\n");
            while(1){
                sleep(1);
                while(images_num > 0)
                    {
            //Child 1                          //child1 process-FFT
            pthread_mutex_lock(&mutex1); 
            strcpy(img_pop,pop_image(start, img_pop)); //father process-pop image from queue
            close(pipefds1[0]); 
            close(pipefds2[0]); 
            close(pipefds3[0]);                 
            for(int i=0; i<1024;i++)
                img_pop[i] = (char)(((int)img_pop[i])*2);
            printf("ENTERING FFT PROCESS...\n");
            write(pipefds1[1], &write_msg, sizeof(write_msg));
            printf("Writing %sto pipe - from FFT to CONV \n", write_msg);
            //printf("%s\n",img_pop);
            pthread_mutex_unlock(&mutex1);
            sleep(2);
                    }
            }
        }
        pid2 = fork();
        if (pid2 < 0) {
            perror("Pipe Error");
            exit(1);
        }
        if (pid2 == 0) 
        {
            //Child 2 
            printf("CONV PROCESSING\n");
            printf("Yesss");            //child2 process-CONV
            while(1){
                //sleep(1);
            while(images_num > 0)
            {
                read(pipefds1[0], &read_msg1, sizeof(read_msg1));
                if(strcmp(read_msg1,"1") == 0)
                {
                    printf("ENTERING CONV PROCESS...\n");
                    pthread_mutex_lock(&mutex2);
                    close(pipefds1[0]); 
                    close(pipefds2[0]); 
                    close(pipefds3[0]);
                    for(int i=0; i<1024;i++)
                        img_pop[i] = (char)((int)img_pop[i]+1);
                    printf("Writing to pipe - from conv to ID %s\n", write_msg);
                    //printf("%s\n",img_pop);
                    write(pipefds2[1], &write_msg, sizeof(write_msg));
                    pthread_mutex_unlock(&mutex3);
                    sleep(1);
                }
            }
            }
        }
        pid3 = fork();
        if (pid3 < 0) {
            perror("Pipe Error");
            exit(-1);
        }
        if (pid3 == 0) 
        {
            printf("ID PROCESSING\n");
         
        while(1)
        {  //Child 3               ///child 3 process-ID
        while(images_num > 0)
        {
            read(pipefds2[0], &read_msg2, sizeof(read_msg2));
            if(strcmp(read_msg2,"1") == 0)
            {
                printf("ENTERING ID PROCESS...\n");
                pthread_mutex_lock(&mutex3);
                result = rand() % 3;
                printf("send from Id to uart and then to client");      //send the result
                if(result==0)
                {strcpy(class_id, "mouse");
                //send(connfd, class_id, 5,0);
                printf("%s",class_id);
                }
                if(result==1){strcpy(class_id, "cat");
                //send(connfd, class_id, 3,0);
                printf("%s",class_id);
                }
                if(result==2){strcpy(class_id, "dog");
                //send(connfd, class_id, 3,0);
                printf("%s",class_id);
                }

                pthread_mutex_unlock(&mutex3);
                sleep(1);
            }
        }
        }
    }

    close(pipefds1[0]); close(pipefds1[1]); close(pipefds2[0]); close(pipefds2[1]); close(pipefds3[1]);  
    close(pipefds3[0]);
    printf("-----------------------------------------------------");
    //printf("%s\n",img_pop);
    for(int j=0; j<200;j++)
        printf("%d  ",(int)img_pop[j]);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);
    return 0; 
}

