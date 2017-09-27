//Joe Shvedsky
//April 30, 2017
//rather "simple" tcp/udp listener
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
char** sort_dir() 
{}*/



int main(int argc, char *argv[])
{
    unsigned char listl[5] = {'L', 'I', 'S', 'T', '\n'};
    unsigned char savel[5] = {'S', 'A', 'V', 'E', ' '};
    unsigned char readl[5] = {'R', 'E', 'A', 'D', ' '};
   // char dirname[41] = "storage";
  
     //mkdir(dirname, 0700); //create the god directory

    if(argc != 3)
    {
       
    }
    
    unsigned short tcp_port = atoi(argv[1]);
    unsigned short udp_port = atoi(argv[2]);
    
 
    
    int tcp_listener, udp_socket;
    
    struct sockaddr_in tcp_addr;
    struct sockaddr_in udp_addr;
   // struct sockaddr_in dest;


    fd_set readfds;

    tcp_listener = socket(AF_INET, SOCK_STREAM, 0);
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    /*if( setsockopt(tcp_listener, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }*/
    
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(tcp_port);
    
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(udp_port);
   
    printf("Started server\n");
    fflush(stdout);
    printf("Listening for TCP connections on port: %u\n", tcp_port);
    fflush(stdout);
    printf("Listening for UDP datagrams on port: %u\n", udp_port);    
    fflush(stdout);
    
    
    
    //memset(&serv_addr, '0', sizeof(serv_addr));
    //memset(sendBuff, '0', sizeof(sendBuff)); 



    if (bind(tcp_listener, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
     
    if (bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }  
    
    if (listen(tcp_listener, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    

    pid_t pid;
    struct sockaddr_in client;
    int fromlen = sizeof( client );
    int n;
    unsigned char buffer[500];

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(tcp_listener, &readfds);
        FD_SET(udp_socket, &readfds);
        
        select( FD_SETSIZE, &readfds, NULL, NULL, NULL );
        
      if(FD_ISSET(udp_socket, &readfds)) //udp port
        {
          n = recvfrom( udp_socket, buffer, 6, 0, (struct sockaddr *) &client,
              (socklen_t *) &fromlen );
  
          printf("Rcvd incoming UDP datagram from: %s\n", 
                  inet_ntoa( (struct in_addr)client.sin_addr ));   
          fflush(stdout);
          if(n != 5 || memcmp(listl, buffer, 5) != 0)
          {
            fprintf(stderr, "ERROR UDP clients may only request LIST\n");
          }
          else
          {
            printf("Received LIST\n");
            fflush(stdout);
            struct dirent** namelist;
            int num = scandir("./storage/", &namelist, NULL, alphasort);

            if(num < 0)
            {
              fprintf(stderr, "ERROR scandir failed\n");
              if((sendto(udp_socket, "0\n", 2, 0, (struct sockaddr*) &client, (socklen_t) fromlen)) == 2)
              {
                printf("Sent 0\n");
                fflush(stdout);
              }
              else
                fprintf(stderr, "ERROR sendto failed\n");
            }
            else
            {
              char message[10];
              sprintf(message, "%d", num - 2);
              int i;
              for(i = 2; i < num; ++i)
              {
                strcat(message, " ");
                strcat(message, namelist[i] -> d_name);
                free(namelist[i]);
              }
              strcat(message, "\n");
              if(num > 0)
                free(namelist);
              if(sendto(udp_socket, message, strlen(message), 0, (struct sockaddr*) &client, (socklen_t) fromlen) > 0)
              {
                printf("Sent %s", message);
                fflush(stdout);
              }
            }            
          }
          
        }        
        if(FD_ISSET(tcp_listener, &readfds))    //incoming request on tcp port
        {
          int newsock = accept( tcp_listener,
                            (struct sockaddr *)&client,
                            (socklen_t *)&fromlen );
             /* this accept() call we know will not block */    
          printf("Rcvd incoming TCP connection from: %s\n", 
                  inet_ntoa( (struct in_addr)client.sin_addr ));   
          fflush(stdout);
          pid = fork();
          if(pid > 0)
          {
            
            close(newsock);
          }
          else
          {
            int byte_iterator;
            int save = 0;
            int read = 0;
            int list = 0;
            int bytes_read = -1;
            int bytes_to_save = -1; // program knows how many bytes to save
           // unsigned char* file_contents;
            FILE* fp;
             char fp_name[32] = "";
            int file_charcount = 0;
  
            unsigned char keyword[6] = "";
            int keysize = 0;
            int fd;
            while( (n = recv( newsock, buffer, 500 , 0 ) )> 0)
            {

              if(save == 0 && bytes_to_save > 0)
              {
                  bytes_read+=n;

                  if(bytes_read == bytes_to_save)
                  
                  {

                    send(newsock, "ERROR FILE EXISTS\n", 18, 0);
                    printf("[child %d] Received SAVE %s %d\n[child %d] Sent ERROR FILE EXISTS\n", getpid(), fp_name, bytes_to_save, getpid());
                    fflush(stdout);    
                    bytes_to_save = -1;
                    bytes_read = 0;
                  }
                continue;
              }
              unsigned char total_bytes[27]; //will definitely not be this big but to be safe
              byte_iterator = 0;

              if (save == 2)//reading file
              {
                //printf("HERE5\n");
                //fflush(stdout);
                int written = write(fd, buffer, n);
                bytes_read+=written;
                if(bytes_read == bytes_to_save)
                {
                  close(fd);
                  send(newsock, "ACK\n", 4, 0);
                  printf("[child %d] Stored file \"%s\" (%d bytes)\n[child %d] Sent ACK\n", getpid(), fp_name, bytes_read, getpid());
                  fflush(stdout);
                  save = 0;
                  bytes_to_save = 0;
                  continue;
                } 
              }
              else if(!save && !read && !list)
              {
                for(; keysize < 5; ++keysize)
                {
                  keyword[keysize] = buffer[byte_iterator++];
                }
                keysize = 0;
                
                if(!memcmp(keyword, savel, 5))
                {
                    save = 1;
                }
                else if(!memcmp(keyword, listl, 5))
                {
                  list = 1;

                }
                else if(!memcmp(keyword, readl, 5))
                {
                  read = 1;
                }
                else
                {
                  continue;
                }
              
              }
              
              if (save == 1)
              {
                char path[60] = "./storage/";
                //printf("HERE\n");
                //fflush(stdout);
                for(file_charcount = 0; buffer[byte_iterator]!= ' '; ++byte_iterator)
                {
                  fp_name[file_charcount++] = buffer[byte_iterator];
                }
                fp_name[file_charcount] = '\0';
                strcat(path, fp_name);
                fd = open(path, O_CREAT|O_WRONLY|O_EXCL, S_IRUSR|S_IWUSR);
               // printf("%d %d\n", fd, errno == EEXIST);

                //printf("HERE1\n");
                //fflush(stdout);
                ++byte_iterator;
                bytes_read = 0;
                
                memset(total_bytes, 0, 27);
                
                while(buffer[byte_iterator] != '\n')
                {
                  total_bytes[bytes_read++] = buffer[byte_iterator++];
                }
                
               // printf("HERE2\n");
                //fflush(stdout);
                bytes_to_save = atoi((char*)total_bytes);
                if(fd < 0 && errno == EEXIST)
                {

                 bytes_read += (strlen(fp_name) + 7);
                  if(bytes_to_save == n - bytes_read)
                  {
                    send(newsock, "ERROR FILE EXISTS\n", 18, 0);
                    printf("[child %d] Received SAVE %s %d\n[child %d] Sent ERROR FILE EXISTS\n", getpid(), fp_name, bytes_to_save, getpid());
                    fflush(stdout);    
                    bytes_to_save = -1;
                    save = 0;
                    bytes_read = 0;  
                    continue;
                  }
           
                 bytes_read = n - bytes_read ;
                  //fprintf(stderr,"ERROR\n");
                  save = 0;
                  continue;
                }
                
                //printf("HERE3\n");
                //fflush(stdout);                
                printf("[child %d] Received SAVE %s %d\n", getpid(), fp_name, bytes_to_save);
                fflush(stdout);
                save = 2;
                bytes_read = 0;
  
                if(byte_iterator == n)
                  continue;
                ++byte_iterator;
                int written = write(fd, (buffer+byte_iterator), n - byte_iterator);
               // printf("HERE4\n");
                //fflush(stdout);
                bytes_read+=written;
                if(bytes_read == bytes_to_save)
                {
                  close(fd);
                   send(newsock, "ACK\n", 4, 0);
                   printf("[child %d] Stored file \"%s\" (%d bytes)\n[child %d] Sent ACK\n", getpid(), fp_name, bytes_read, getpid());
                   fflush(stdout);
                   bytes_to_save = 0;
                  save = 0;
                }          
              }
              else if(read)
              {
                
                char path[60] = "./storage/";
                for(file_charcount = 0; buffer[byte_iterator] != ' '; ++byte_iterator)
                {
                    fp_name[file_charcount++] = buffer[byte_iterator];
                }
                fp_name[file_charcount] = '\0';
                strcat(path, fp_name);
                fp = fopen(path, "r");

                ++byte_iterator;
                memset(total_bytes, 0 ,27);
                for(bytes_read = 0; buffer[byte_iterator] != ' '; ++byte_iterator)
                {
                    total_bytes[bytes_read++] = buffer[byte_iterator];
                }
                int offset = atoi((char*) total_bytes);
                
                memset(total_bytes, 0, 27);
                
                ++byte_iterator;
                
                for(bytes_read = 0; buffer[byte_iterator] != '\n'; ++byte_iterator)
                {
                    total_bytes[bytes_read++] = buffer[byte_iterator];
                }
                int length = atoi((char*) total_bytes);   
                  
                  if(fp == NULL)
                {
                    send(newsock, "ERROR NO SUCH FILE\n", 19, 0);
                    printf("[child %d] Received READ %s %d %d\n[child %d] Sent ERROR NO SUCH FILE\n", getpid(), fp_name, offset, length, getpid());
                    fflush(stdout);    
                    read = 0;
                    continue;
                }              
                struct stat st;
                stat(path, &st);
                int file_size = st.st_size;
                
                if(length + offset > file_size)
                {
                    send(newsock, "ERROR INVALID BYTE RANGE\n", 25, 0);
                    printf("[child %d] Received READ %s %d %d\n[child %d] Sent ERROR INVALID BYTE RANGE\n", getpid(), fp_name, offset, length, getpid());
                    fflush(stdout);
                    read = 0;
                    continue;
                }
                  printf("[child %d] Received READ %s %d %d\n", getpid(), fp_name, offset, length);
                fflush(stdout);
                char msg_num[20] = "";
                char msg_ack[20] = "ACK";
                sprintf(msg_num, " %d\n", length);
                strcat(msg_ack, msg_num);
                 send(newsock, msg_ack, strlen(msg_ack), 0);
                unsigned char c;
                int i;
                for(i = 0; i < offset; ++i)
                {

                    c = fgetc(fp);
                    if(c == EOF)
                      break;
                }
         
                unsigned char message[length];
               
                for(i = 0; i < length; ++i)
                {
                    message[i] = fgetc(fp);
                }
                fclose(fp);
                read = 0;
                send(newsock, message, length, 0);
                //printf("\n");
                
               printf("[child %d] Sent ACK %d\n", getpid(), length);
               fflush(stdout);
               printf("[child %d] Sent %d bytes of \"%s\" from offset %d\n", getpid(), length, fp_name, offset);
               fflush(stdout);
              }
              else if(list)
              {
                  
                  printf("[child %d] Received LIST\n", getpid());
                  fflush(stdout);
                  struct dirent** namelist;
                  int num = scandir("./storage/", &namelist, NULL, alphasort);
  
                  if(num >= 0)
                  {
                     char message[10];
                    sprintf(message, "%d", num - 2);
                    int i;
                    for(i = 2; i < num; ++i)
                    {
                      strcat(message, " ");
                      strcat(message, namelist[i] -> d_name);
                      free(namelist[i]);
                    }
                    strcat(message, "\n");
                    if(num > 0)
                      free(namelist);
                    if(send(newsock, message, strlen(message), 0) > 0)
                    {
                      printf("[child %d] Sent %s", getpid(), message);
                      fflush(stdout);
                    }
                  }
                  list = 0;
                  
              }
            }
                if ( n == -1 )
                {
                  perror( "recv() failed" );
                  return EXIT_FAILURE;
                }
                else if ( n == 0 )
                {
                  printf( "[child %d] Client disconnected\n",
                          getpid() );
                          fflush(stdout);
                }
                      close(newsock);
                      exit(EXIT_SUCCESS);

            }

          }
        
       
    }

     

return EXIT_SUCCESS;
}
