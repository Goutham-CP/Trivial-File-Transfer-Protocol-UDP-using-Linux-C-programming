#include "tftp.h"
#include"tftp_client.h"
#include"platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include<errno.h>


char last = '\0';
void handle_client(int sockfd, struct sockaddr_in client, socklen_t client_len, tftp_packet *packet);
 void WRQ_Fn(char *collect,struct sockaddr_in server_addr,struct sockaddr_in client,int sockfd,socklen_t client_len, int mode);
 void RRQ_Fn(char *collect,struct sockaddr_in server_addr,struct sockaddr_in client,int sockfd,socklen_t client_len,int mode);

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client;
    socklen_t client_len = sizeof(client);
    tftp_packet packet;


    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        perror("bind");
        return 0;
    }
    printf("UDP Server listening in port %d\n",PORT);

    char buffer[50];
    socklen_t len = sizeof(client);

    int n = recvfrom(sockfd , buffer , sizeof(buffer)-1,0,(struct sockaddr *)&client , &len);

    if(n>0){
        buffer[n] = '\0';
        int count;

        int l = strlen(buffer);
        char collect[l-3];
        int mode;

        if(sscanf(buffer,"%s %d %d",collect , &count , &mode)==3){
            printf("Filename : %s\n",collect);
            printf("Opcode : %d\n",count);
        }else{
            printf("parsing failed\n");
            return 0;
        }


        if(count==2){

            WRQ_Fn(collect , server_addr,client,sockfd,client_len, mode);


        }else if(count==1){

            RRQ_Fn(collect , server_addr, client,sockfd,client_len,mode);

        }


    }

    }



    void WRQ_Fn(char *collect,struct sockaddr_in server_addr,struct sockaddr_in client,int sockfd,socklen_t client_len, int mode){

      if(mode==3){

         GETFile_NetASCII(sockfd, client, client_len, collect);
         return;

      }



        int fd = open(collect, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) {
       if (errno == EACCES) {
        char *ack = "FAILURE";
        sendto(sockfd, ack, strlen(ack), 0,
               (struct sockaddr *)&client, client_len);
        return;
    } else {
        perror("open failed");
        return;
      }
    }



             char *ack = "SUCCESS";
            sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);
        int size;
         if(mode==1){
           size = 512;
         }else if(mode==2){
            size = 2;
         }

        char buf[size];


         while(1){

       int n = recvfrom(sockfd , buf , sizeof(buf),0,(struct sockaddr*)&client,&client_len);

       if(n<=0)break;
      if(mode==1){
       buf[n] = '\0';
      }
      // printf("Received is %s\n",buf);
       printf("Received %d bytes\n", n);

        if(mode == 1 && strcmp(buf , "0")==0){

            printf("EOF Received stopping\n");
            break;
       }else if(mode==2 && n==2){
            printf("EOF Received stopping\n");
            break;
       }


        if (write(fd, buf, n) < 0) {
            perror("write failed");
            break;
        }


      char ack[] = "ACK";

      sendto(sockfd , ack, strlen(ack), 0, (struct sockaddr *)&client ,client_len);

     //   close(fd);

      }



     close(fd);
     // Create UDP socket


    // Set socket timeout option
    //TODO Use setsockopt() to set timeout option


    // Set up server address


    // Bind the socket


    }

    void RRQ_Fn(char *collect,struct sockaddr_in server_addr,struct sockaddr_in client,int sockfd,socklen_t client_len,int mode){

        if(mode==3){

             PUTFile_NetASCII( sockfd, client, client_len,collect);
             return;
        }

          int fd = open(collect, O_RDONLY);

          if(fd<0){
             perror("open");
             char *ack = "FAILURE";
               sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);
               return;
          }

          char *ack = "SUCCESS";
            sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);

               int size;
                if(mode==1){
                  size = 512;
                }else if(mode==2){
                    size = 1;
                }
                char read_buf[size];

    int ack_ok = 0;
    int n;
    while((n = read(fd, read_buf, sizeof(read_buf)))>0){

         ack_ok = 0;
        int sent = sendto(sockfd, read_buf, n,0, (struct sockaddr *)&client, client_len);

        if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }

  }
  if(mode==1){

  char eof_msg[] = "0";
sendto(sockfd, eof_msg, strlen(eof_msg), 0,
       (struct sockaddr *)&client, client_len);
printf("EOF sent, stopping \n");
  }else if(mode==2){
      char eof_msg[] = "ST";
      sendto(sockfd, eof_msg, strlen(eof_msg), 0,
       (struct sockaddr *)&client, client_len);
printf("EOF sent, stopping \n");
  }




    }





void handle_client(int sockfd, struct sockaddr_in client, socklen_t client_len, tftp_packet *packet)
{
    // Extract the TFTP operation (read or write) from the received packet
    // and call send_file or receive_file accordingly
}




void GETFile_NetASCII(int sockfd,struct sockaddr_in client,socklen_t client_len, char *filename) {

    // Wait for server ACK (SUCCESS/FAILURE)


    // Open file for writing
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
       if (errno == EACCES) {
        char *ack = "FAILURE";
        sendto(sockfd, ack, strlen(ack), 0,
               (struct sockaddr *)&client, client_len);
        return;
    } else {
        perror("open failed");
        return;
      }
    }

    char *ack = "SUCCESS";
            sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);


    // Receive loop with NetASCII decode
    char buf[512], decoded[512];
    int n;
    while ((n = recvfrom(sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&client, &client_len)) > 0) {

        // EOF marker check
        if (n == 1  && buf[0]=='\0') {
            printf("NetASCII EOF received, stopping\n");
            break;
        }

        // Decode NetASCII into local format
        int write_len = netascii_decode(buf, n, decoded, fd);



        if (write(fd, decoded, write_len) < 0) {
            perror("write failed");
            break;
        }

        // Send ACK
        sendto(sockfd, "ACK", 3, 0,
               (struct sockaddr *)&client, client_len);
    }

    close(fd);
}



// Dedicated PUT for NetASCII mode
void PUTFile_NetASCII(int sockfd,struct sockaddr_in client,socklen_t client_len, char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
             perror("open");
             char *ack = "FAILURE";
               sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);
               return;
          }

    char *ack = "SUCCESS";
   sendto(sockfd, ack, strlen(ack), 0,(struct sockaddr *)&client, client_len);


    // File transfer loop with NetASCII encoding
   char packet[512];
int pkt_len = 0;
char ret = 0;
char ch;
int n;
char  ack_ok = 0;
while((n = read(fd, &ch, 1)) > 0)
{
    if(ch == '\n')
    {
        //packet[pkt_len++] = '\r';

        if(pkt_len == 512)
        {
         int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }

        }


        if(ret==1){
           if(pkt_len>0){
           packet[pkt_len-1] = '\n';
           }else{
            packet[pkt_len++] = '\n';
          }
           ret = 0;
       }else{
           packet[pkt_len++] = '\r';
           if(pkt_len == 512)
        {
            int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }

        }

           packet[pkt_len++] = '\n';
      }
    }
    else if(ch == '\r')
    {

        if(pkt_len == 512)
        {
            int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }


        }

        packet[pkt_len++] = '\r';
        ret = 1;

        if(pkt_len == 512)
        {
            int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }

        }

         packet[pkt_len++] = '\0';
    }
    else
    {

       if(pkt_len == 512)
        {
            int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }
        }

       if(ret==1){
         ret = 0;
       }
        packet[pkt_len++] = ch;
    }

    if(pkt_len == 512)
    {
        int sent =  sendto(sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client,client_len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client,&client_len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
       //   printf("sent... and buf is %s\n",read_buf);
          printf("sent... %d bytes\n", n);
           ack_ok = 1;
           pkt_len = 0;
           continue;

       }else{

          lseek(fd , -n,SEEK_CUR);

      }


    }else{

     lseek(fd , -n,SEEK_CUR);

    }
    }
}





if(pkt_len>0){


 sendto(sockfd, packet, pkt_len, 0,(struct sockaddr *)&client, client_len);

char stop_msg = '\0';
    sendto(sockfd, &stop_msg, 1, 0,
           (struct sockaddr *)&client, client_len);
    printf("NetASCII EOF sent, transfer complete\n");

}else{

    char stop_msg = '\0';
    sendto(sockfd, &stop_msg, 1, 0,
           (struct sockaddr *)&client,client_len);
    printf("NetASCII EOF sent, transfer complete\n");



}
    close(fd);
}



int netascii_decode(const char *in, int in_len, char *out, int fd) {
    int j = 0;

    for (int i = 0; i < in_len; i++) {

        if(i==0 && last=='\r'){

            if(in[i]=='\n'){

               #ifdef TARGET_OS_LINUX

                    out[j++] = '\n';   // collapse CRLF → LF
                    last = '\0';
                    continue;
            #elif defined (TARGET_OS_WINDOWS)
                    //out[j++] = '\r';   // keep CRLF

                    char ch = '\r';
                    write(fd, &ch, 1);
                    out[j++] = '\n';
                    last = '\0';
              continue;

            #endif

           }else if(in[i]=='\0'){

               #ifdef TARGET_OS_LINUX
                   // out[j++] = '\r';   // collapse CRLF → LF

                    char ch = '\r';
                    write(fd, &ch, 1);
                    last = '\0';
              continue;
            #elif defined(TARGET_OS_WINDOWS)
                    //out[j++] = '\r';   // keep CRLF

                    char ch = '\r';
                    write(fd, &ch, 1);
                    last = '\0';
               continue;

            #endif
          }

        }




        if (in[i] == '\r') {

            if(i==in_len-1){
              last = in[i];
              continue;
            }

            if (i+1 < in_len && in[i+1] == '\n') {
                #ifdef TARGET_OS_LINUX

                    out[j++] = '\n';   // collapse CRLF → LF
                #elif defined(TARGET_OS_WINDOWS)

                    out[j++] = '\r';   // keep CRLF
                    out[j++] = '\n';
                #elif defined(TARGET_OS_MAC)

                    out[j++] = '\r';   // collapse CRLF → CR
                #endif
                i++; // skip next char
            } else if (i+1 < in_len && in[i+1] == '\0') {
                out[j++] = '\r';       // CRNUL → CR
                i++;
            } else {
                out[j++] = '\r';       // lone CR
            }
        } else {
            out[j++] = in[i];
        }
    }
    return j;
}
