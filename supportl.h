/*
* TCP LogSystem 
* Created by Bernardo Meneghini, Lucas Rocha and Thiago Alexandre in 09/21/2017.
* This program sends local logs from a client to the server based of some given expression.
*/

#ifndef SUPPORTL_CPP_SUPPORTL_H
#define SUPPORTL_CPP_SUPPORTL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <linux/if_arp.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

#define NETWORK_INTERFACE "wlp1s0"
//"ethZero0 TCHARAM"

#define TRANSPORT_PORT_SERVER 63051
#define TRANSPORT_PORT_CLIENT 63041
#define PORT_NUMBER 8761
#define MIN_MSG_BUFF 256
#define MIN_MSG_SIZE MIN_MSG_BUFF - 1
#define BUFFER_SIZE 576
#define MAX_BUFFER BUFFER_SIZE - 1


#define PREAMBLE_SIZE 8
#define MAC_SIZE      6
#define PROTOCOL_SIZE 2
#define CHECKSUM_SIZE 4
#define MAX_DATA_SIZE MAX_BUFFER-PREAMBLE_SIZE-(2*MAC_SIZE)-PROTOCOL_SIZE-CHECKSUM_SIZE-2

struct Frame{
    char preamble[PREAMBLE_SIZE]; // 8 bytes for preamble
    char ethernetType[PROTOCOL_SIZE];// type of protocol - 0x0800 for Ethernet in IPv4
    char data[MAX_DATA_SIZE]; // Data has [Frame size - frame items size] size.
    char checksum[CHECKSUM_SIZE];
};


size_t frameSize(struct Frame *frame){
    return PREAMBLE_SIZE + (2*MAC_SIZE)
           + PROTOCOL_SIZE + CHECKSUM_SIZE
           + strlen(frame->data);
}

void getData(struct Frame *frame, char data[MAX_DATA_SIZE+1]){
    bzero(data, MAX_DATA_SIZE+1);
    strncpy(data, frame->data, MAX_DATA_SIZE);
    data[MAX_DATA_SIZE] = '\0';
}

void createFrame(struct Frame *frame, char *message){
    strcpy (frame->preamble, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xab"); // pre-defined preamble.
    strcpy (frame->ethernetType, "\x08\x00"); // type of protocol - 0x0800 for IPv4
    bzero  (frame->data, MAX_DATA_SIZE);
    sprintf(frame->data, "%s", message);
    strcpy (frame->checksum, "\x30\x21\x00\x00");
}

void bytesToStr(char *p, char *d, size_t len){
    char * ptr = d;
    ptr+=sprintf(ptr, "[");
    for (int i = 0; i < len-1; ++i)
        ptr+=sprintf(ptr,"%02x:",(unsigned char) p[i]);
    ptr+=sprintf(ptr,"%02x",(unsigned char)p[len-1]);
    ptr+=sprintf(ptr, "]");
    *(ptr + 1) = '\0';
}


void printFrame(struct Frame *frame){
    char aux[BUFFER_SIZE];
    bytesToStr(frame->preamble, aux, PREAMBLE_SIZE );
    printf("Frame:\n\tpreamble: %s\n",aux);
    bytesToStr(frame->ethernetType, aux, PROTOCOL_SIZE );
    printf("\tethernetType: %s\n",aux);
    getData(frame, aux);
    for(int i=0; i<strnlen(frame->data, MAX_DATA_SIZE); i++)
        if(aux[i]=='\n' || aux[i]=='\t' || aux[i]==' ')
            aux[i] = '_';
    printf("\tdata: [%s]\n",aux);
    bytesToStr(frame->checksum, aux, CHECKSUM_SIZE );
    printf("\tchecksum: %s\n",aux);
}

void receiveFrame(struct Frame *frame, int sockfd){
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    int n = read(sockfd,buffer,MAX_BUFFER);
    if (n<0)
        error("Could not read from socket");
    int offset = 0;
    strncpy (frame->preamble, buffer, PREAMBLE_SIZE);
    offset += PREAMBLE_SIZE;
    strncpy (frame->ethernetType, buffer+offset, PROTOCOL_SIZE);
    offset += PROTOCOL_SIZE;
    bzero(frame->data, MAX_DATA_SIZE);
    strncpy (frame->data, buffer+offset, MAX_DATA_SIZE);
    frame->data[MAX_DATA_SIZE] = '\0';
    offset = n -CHECKSUM_SIZE;
    strncpy (frame->checksum, buffer+offset, CHECKSUM_SIZE);
    if(n!=0){
        printf("Frame Received!!! (%d bytes) \n", (int) n);
        printFrame(frame);
    }
}

void sendFrame(struct Frame *frame, int socket, size_t frame_size){
    ssize_t sent_bytes;
    sent_bytes = send(socket, (int*)frame, frame_size, 0);
    if (sent_bytes < 0)
        error("ERROR writing to socket");
    printf("Frame sent! (%d bytes)\n", (int) sent_bytes );
    printFrame(frame);
}

void sendMessage(int sockfd, char *msg){
    char buffer[MIN_MSG_SIZE];
    ssize_t n;
    bzero(buffer,MIN_MSG_BUFF);
    strcpy(buffer, msg);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
        error("ERROR sending to socket");
    printf("%d bytes sent: %s\n", (int)n, buffer);
}

void receiveMessage(int sockfd, char *msg){
    char buffer[MIN_MSG_SIZE];
    ssize_t n;
    bzero(buffer,MIN_MSG_BUFF);
    n = read(sockfd,buffer,MIN_MSG_SIZE);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%d bytes received: %s\n", (int) n, buffer);
    strcpy(msg, buffer);
}

#endif
