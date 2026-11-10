/* Goutham C P
 * 26001B_059
 * ============================================================================
 *  Project : Trivial File Transfer Protocol (TFTP)
 *  Platform: Linux
 *  Language: C
 *
 *  Description:
 *  ------------
 *  This project implements a Trivial File Transfer Protocol (TFTP)
 *  client-server application using UDP sockets.
 *
 *  The application supports reliable file transfer between client and
 *  server by implementing acknowledgment-based communication over UDP.
 *
 *  Multiple transfer modes are supported, including:
 *      - Default 512-byte packet transfer mode
 *      - Octet Mode 1 byte Packet (Binary Transfer)
 *      - NetASCII Mode (Text Transfer)
 *
 *  Features:
 *  ---------
 *  - Client-Server architecture using UDP sockets.
 *  - Supports PUT (Upload) operation.
 *  - Supports GET (Download) operation.
 *  - Configurable transfer modes:
 *          • Default 512-byte packet mode
 *          • Octet mode
 *          • NetASCII mode
 *  - IP Address validation using inet_pton().
 *  - Filename validation and access checking.
 *  - ACK-based synchronization for reliable transfer.
 *  - End-of-file detection using dedicated EOF marker packets.
 *  - NetASCII Encoding:
 *          • '\n'   -> '\r\n'
 *          • '\r'   -> '\r\0'
 *          • '\r\n' -> '\r\n'
 *  - NetASCII Decoding:
 *          • '\r\n' -> '\n' (Linux)
 *          • '\r\0' -> '\r'
 *  - Packet boundary handling for NetASCII transfers.
 *  - Handles CR/LF conversion between different operating systems.
 *  - Robust error handling for invalid requests and file operations.
 *  - Supports transfer of text and binary files.
 *
 *  Technical Concepts Used:
 *  ------------------------
 *  - UDP Socket Programming
 *  - Client-Server Architecture
 *  - Network Byte Communication
 *  - File Handling System Calls
 *  - Packet-Based Data Transfer
 *  - NetASCII Encoding & Decoding
 *  - Processed End-of-File Detection
 *  - Error Handling Mechanisms
 *
 *  Run:
 *      Server:
 *          ./server(./a.out)
 *
 *      Client:
 *          ./client(./a.out)
 *
 * Note : Both client and server should be seperate folders
 * and not in same folder
 *
 *  Example:
 *      GET Sample.txt
 *      PUT Sample.txt
 *
 *      Mode 1 -> Default 512-byte packet mode
 *      Mode 2 -> Octet mode
 *      Mode 3 -> NetASCII mode
 *
 *  Remarks:
 *  --------
 *  Special care has been taken to correctly handle NetASCII packet
 *  boundaries, including cases where '\r' occurs at the end of one
 *  packet and '\n' or '\0' appears at the beginning of the next packet.
 *
 * ============================================================================
 */





#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include"platform.h"

char last = '\0';
int main() {
    //char command[256];
    tftp_client_t client;
    memset(&client, 0, sizeof(client));  // Initialize client structure
     char ip[INET_ADDRSTRLEN];
    int port;

   // scanf("%15s", ip);
    printf("Enter port: ");
    scanf("%d", &port);
    getchar();

   char *choicestr;

    // Main loop for command-line interface
    int choice;
    ssize_t len = 0;
    do{


        printf("------Menu -----\n");
        printf("1.Connect\n");
        printf("2.PUT\n");
        printf("3.GET\n");
        printf("4.Exit\n");
        printf("Enter the choice   ");
       getchar();
       getline(&choicestr,&len,stdin);
       choicestr[strcspn(choicestr, "\n")] = '\0';
        //scanf("%d",&choice);
        //getchar();
        choice = atoi(choicestr);

        switch(choice){

            case 1:
              connect_to_server(&client, PORT);
              break;
            case 2:
          //  getchar();
            char *filename = NULL;
            ssize_t l = 0;
            ssize_t read = getline(&filename, &l, stdin);
            if (read == -1) {
              perror("getline failed");
              return 1;
           }

    // Strip trailing newline if present
           filename[strcspn(filename, "\n")] = '\0';
            put_file(&client , filename);
            break;
            case 3:{
              char *filename = NULL;
            ssize_t l = 0;
            ssize_t read = getline(&filename, &l, stdin);
            if (read == -1) {
              perror("getline failed");
              return 1;
           }

    // Strip trailing newline if present
           filename[strcspn(filename, "\n")] = '\0';
            get_file(&client , filename);
            break;
          }

            case 4:
              exit(0);
              break;
            default:
              printf("Invalid Option\n");
              break;
        }

    }while (choice !=4);






    return 0;
}

int netascii_encode(const char *in, int in_len, char *out){

   int j = 0;
    for(int i=0;i<(in_len);i++){

      if(in[i]=='\n'){

        out[j++] = '\r';
        out[j++] = '\n';


      }else if(in[i]=='\r'){

        if(i+1<in_len && in[i+1]=='\n'){

          out[j++] = '\r';
          out[j++]= '\n';
          i++;

        }else{

           out[j++] = '\r';
           out[j++] = '\0';

        }

      }else{

        out[j++] = in[i];

      }


    }

return j;
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


void GETFile_NetASCII(tftp_client_t *client, char *filename) {
    tftp_packet pack;
    pack.opcode = RRQ;

    // Send RRQ request with mode=3
    char msg[256];
    sprintf(msg, "%s %u %d", filename, pack.opcode, 3);
    if (sendto(client->sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)&client->server_addr, client->server_len) < 0) {
        perror("sendto");
        return;
    }

    // Wait for server ACK (SUCCESS/FAILURE)
    char ack[16];
    socklen_t len = client->server_len;
    int n = recvfrom(client->sockfd, ack, sizeof(ack)-1, 0,
                     (struct sockaddr *)&client->server_addr, &len);
    if (n < 0) { perror("recvfrom"); return; }
    ack[n] = '\0';

    if (strcmp(ack, "FAILURE") == 0) {
        printf("Server denied read\n");
        return;
    }

    // Open file for writing
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) { perror("open"); return; }

    // Receive loop with NetASCII decode
    char buf[512], decoded[512];
    while ((n = recvfrom(client->sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&client->server_addr, &len)) > 0) {

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
        sendto(client->sockfd, "ACK", 3, 0,
               (struct sockaddr *)&client->server_addr, len);
    }

    close(fd);
}



// Dedicated PUT for NetASCII mode
void PUTFile_NetASCII(tftp_client_t *client, char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    tftp_packet pack;
    pack.opcode = WRQ;

    // Send WRQ request with mode=3
    char msg[256];
    sprintf(msg, "%s %u %d", filename, pack.opcode, 3);
    if (sendto(client->sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)&client->server_addr, client->server_len) < 0) {
        perror("sendto");
        close(fd);
        return;
    }

    // Wait for server ACK (SUCCESS/FAILURE)
    char ack[16];
    socklen_t len = client->server_len;
    int n = recvfrom(client->sockfd, ack, sizeof(ack)-1, 0,
                     (struct sockaddr *)&client->server_addr, &len);
    if (n < 0) { perror("recvfrom"); close(fd); return; }
    ack[n] = '\0';

    if (strcmp(ack, "FAILURE") == 0) {
        printf("Server denied write\n");
        close(fd);
        return;
    }

    // File transfer loop with NetASCII encoding
   char packet[512];
int pkt_len = 0;
char ret = 0;
char ch;
 int ack_ok = 0;
 int cn=1;

while((n = read(fd, &ch, 1)) > 0)
{
  printf("ch is %d and c : %d\n",ch,cn++);
    if(ch == '\n')
    {
        //packet[pkt_len++] = '\r';

        if(pkt_len == 512)
        {
           int sent =  sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
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
            int sent = sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
           ack_ok = 1;
           pkt_len = 0;


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
            int sent = sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
           ack_ok = 1;
           pkt_len = 0;

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
            int sent = sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
           ack_ok = 1;
           pkt_len = 0;


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
            int sent = sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
            if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
           ack_ok = 1;
           pkt_len = 0;


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
        int sent = sendto(client->sockfd, packet, pkt_len, 0,
                  (struct sockaddr *)&client->server_addr, len);
        if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
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


 sendto(client->sockfd, packet, pkt_len, 0,(struct sockaddr *)&client->server_addr, len);

char stop_msg = '\0';
    sendto(client->sockfd, &stop_msg, 1, 0,
           (struct sockaddr *)&client->server_addr, len);
    printf("NetASCII EOF sent, transfer complete\n");

}else{

    char stop_msg = '\0';
    sendto(client->sockfd, &stop_msg, 1, 0,
           (struct sockaddr *)&client->server_addr, len);
    printf("NetASCII EOF sent, transfer complete\n");



}
    close(fd);
}





void connect_to_server(tftp_client_t *client,int port){

      printf("Enter the valid IP Address  ");
      scanf("%[^\n]",client->server_ip);
      getchar();
      printf("ip : %s\n",client->server_ip);
      int successcount = 0;
      char ip_copy[INET_ADDRSTRLEN];
      strncpy(ip_copy, client->server_ip, INET_ADDRSTRLEN);
      char *tok = strtok(ip_copy , ".");
      int NonZero = 0;

      while(tok!=NULL){


            for (int i = 0; tok[i] != '\0'; i++) {
            if (!(tok[i]>='0' && tok[i]<='9')) {
                printf("Invalid: token contains non-digit characters \n");
                return;
            }
        }

           successcount++;
           tok = strtok(NULL,".");

      }

       if(successcount==4){
        printf("IP Address is valid and str %s\n",client->server_ip);
       }else {
        printf("Invalid not 4 parts found and %s\n",client->server_ip);
       }

       client->sockfd = socket(AF_INET,SOCK_DGRAM, 0);
        if(client->sockfd< 0){

            perror("socket");
            return;
       }


      memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, client->server_ip, &client->server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(client->sockfd);
        return;
    }



    client->server_len = sizeof(client->server_addr);
    printf("Connected to server %s:%d\n", client->server_ip, port);





}

// Function to process commands
void process_command(tftp_client_t *client, char *command) {

}

// This function is to initialize socket with given server IP, no packets sent to server in this function


void put_file(tftp_client_t *client, char *filename) {

    int mode;

    printf("Enter the mode of operation  1.Default(512) byte packet  2.Octet(1 byte packet) 3.Net ASCII  ");
    scanf("%d", &mode);

    while(mode!=1 && mode !=2 && mode!=3){
      fflush(stdin);
      printf("Invalid mode please enter again ");
      scanf("%d",&mode);
    }
      printf("Here filename is %s and mode %d\n",filename,mode);

      if(mode==3){
        int res = validate_netascii_file(filename);
       if(res==0){
        printf("Invalid Filename : should be .txt\n");
       }

          PUTFile_NetASCII(client, filename);
          return;

      }


      int fd = open(filename , O_RDONLY);
      if(fd<0){
        perror("File not found");
        return ;
      }

    tftp_packet pack;
    pack.opcode = WRQ;

    // Send WRQ request and send file
    char *msg = malloc(strlen(filename) + 7);
    sprintf(msg ,"%s %u %d",filename,pack.opcode, mode);
    printf("msg printed here is %s\n",msg);
    if(sendto(client->sockfd, msg, strlen(msg),0, (struct sockaddr *)&client->server_addr, client->server_len)<0){
        perror("sendto");
        return;
    }

    char ack[10];
    socklen_t len = client->server_len;
    int n = recvfrom(client->sockfd,ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr, &len);
    if(n<0){
        perror("recvfrom failed");
        return;
    }
    ack[n] = '\0';

    if(strcmp(ack,"FAILURE")==0){
        printf("Server denied write\n");
        return;
    }else if(strcmp(ack,"SUCCESS")==0){
        int size;
        if(mode==1){
          size = 512;
        }else{
          size = 1;
        }
         char read_buf[size];
    int ack_ok = 0;
    while((n = read(fd, read_buf, sizeof(read_buf)))>0){

         ack_ok = 0;
        int sent = sendto(client->sockfd, read_buf, n,0, (struct sockaddr *)&client->server_addr, len);

        if(sent==-1){

        perror("sendto");
        break;

        }

       char ack[16];

     int acklen = recvfrom(client->sockfd, ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr,&len);

     if(acklen>0){

       ack[acklen] = '\0';
       if(strcmp(ack,"ACK")==0){
        //  printf("sent... and buf is %s\n",read_buf);
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
sendto(client->sockfd, eof_msg, strlen(eof_msg), 0,
       (struct sockaddr *)&client->server_addr, len);
printf("EOF sent, stopping \n");
}else if (mode == 2) {
    // 1-byte mode → send special STOP message
    char stop_msg[] = "ST";
    sendto(client->sockfd, stop_msg, strlen(stop_msg), 0,
           (struct sockaddr *)&client->server_addr, len);
    printf("STOP sent, terminating transfer\n");
}


    }
  // close(client->sockfd);
   close(fd);



}

void get_file(tftp_client_t *client, char *filename) {

  /*printf("Here filename is %s\n",filename);
      int fd = open(filename , O_RDONLY);
      if(fd<0){
        perror("File not found");
        return ;
      }*/

       int mode;

    printf("Enter the mode of operation  1.Default(512) byte packet  2.Octet(1 byte packet) 3.Net ASCII ");
    scanf("%d", &mode);

    while(mode!=1 && mode !=2 && mode!=3){
      fflush(stdin);
      printf("Invalid mode please enter again ");
      scanf("%d",&mode);
    }

    if(mode==3){

       int res = validate_netascii_file(filename);
       if(res==0){
        printf("Invalid Filename : should be .txt\n");
       }

      GETFile_NetASCII(client,filename);
      return;
    }

    tftp_packet pack;
    pack.opcode = RRQ;

    // Send WRQ request and send file
    char *msg = malloc(strlen(filename) + 7); //+5
    sprintf(msg ,"%s %u %d",filename,pack.opcode,mode);
    if(sendto(client->sockfd, msg, strlen(msg),0, (struct sockaddr *)&client->server_addr, client->server_len)<0){
        perror("sendto");
        return;
    }

    char ack[10];
    socklen_t len = client->server_len;
    int n = recvfrom(client->sockfd,ack,sizeof(ack)-1,0,(struct sockaddr *)&client->server_addr, &len);
    if(n<0){
        perror("recvfrom failed");
        return;
    }
    ack[n] = '\0';

    if(strcmp(ack,"FAILURE")==0){
        printf("Server denied write\n");
        return;
    }else if(strcmp(ack,"SUCCESS")==0){

       int fd = open(filename , O_WRONLY|O_TRUNC|O_CREAT,0644);

               if(fd<0){
                perror("open");
                return;
               }

       char buf[512];
      socklen_t len = sizeof(client->server_addr);

       while(1){

       int n = recvfrom(client->sockfd , buf , sizeof(buf),0,(struct sockaddr*)&client->server_addr,&len);

       if(n<=0)break;

       buf[n] = '\0';
     //  printf("Received is %s\n",buf);
       printf("Received %d bytes\n", n);

        if(mode==1 && strcmp(buf , "0")==0){

            printf("EOF Received stopping\n");
            break;
       }else if(mode==2 && n==2){
        printf("EOF Received Stopping\n");
        break;
       }


        if (write(fd, buf, n) < 0) {
            perror("write failed");
            break;
        }


      char ack[] = "ACK";

      sendto(client->sockfd , ack, strlen(ack), 0, (struct sockaddr *)&client->server_addr ,len);

     //   close(fd);

      }

         close(fd);



       }



    }
    // Send RRQ and recive file



    int validate_netascii_file(const char *filename)
{
     int dot_count = 0;
    const char *ext = NULL;

    for(int i = 0; filename[i] != '\0'; i++)
    {
        if(filename[i] == '.')
        {
            dot_count++;
            ext = &filename[i];
        }
    }

    if(dot_count != 1)
        return 0;

    if(ext && strcmp(ext, ".txt") == 0)
        return 1;

    return 0;
}



void disconnect(tftp_client_t *client) {
    // close fd

}
/*void send_request(int sockfd, sockaddr_in server_addr, char *filename, int opcode)
{

}

void receive_request(int sockfd, sockaddr_in server_addr, char *filename, int opcode)
{
}*/
