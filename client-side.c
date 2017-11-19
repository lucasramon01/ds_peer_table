/*
* Peer Table System 
* Created by Bernardo Meneghini, Lucas Ramon, Thiago Alexandre and Victor Moraes.
* This program sends to the server side the execution request of the find and store functions in the peer table.
*/

#include "supportl.h"

char buffer[BUFFER_SIZE];
char *ip_address;
char *filename;
char *expression;
char *address;

int  key;
int  getKey;

int connectSocket(char *ip_address){
    int sockfd; // socket file descriptor
    struct hostent *server;
    struct sockaddr_in serv_addr;
    struct ifreq ifr;
    char msg[25];

    // Opening socket to start connection:
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // resolving host:
    server = gethostbyname(ip_address);
    if (server == NULL)
        error("ERROR, no such host\n");
    bzero((char *) &serv_addr, sizeof(serv_addr)); // cleans serv_addr

    // configuring and connecting socket:
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr,
           (char *)&serv_addr.sin_addr.s_addr,
           server->h_length);
    serv_addr.sin_port = htons(PORT_NUMBER);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    strcpy(buffer,"Testing connection...");
    sendMessage(sockfd, buffer);
    receiveMessage(sockfd, msg);
	
    return sockfd;
}

void parse_options(int argc, char *argv[])
{
    if (argc < 1) {
       fprintf(stderr,"Usage %s -i ip_address\n", argv[0]);
       puts("Optional flags:\n-f file_name\n-e expression\n");
       exit(0);
    }
    puts("----- initial information -----");
    int i;
    for(i = 1; i < argc; ++i) {
        if(strcmp("-i",argv[i]) == 0) {
        	ip_address = argv[++i];
            printf("ip: %s\n",ip_address);            
        }
        if(strcmp("-k",argv[i]) == 0) {
			key = atoi(argv[++i]);
            printf("key: %i\n", key);
        }
        if(strcmp("-f",argv[i]) == 0) {
        	filename = argv[++i];
            printf("filename: %s\n", filename);            
        }
        if(strcmp("-e",argv[i]) == 0) {
        	expression = argv[++i];
            printf("expression: %s\n", expression);            
        }
        if(strcmp("-find",argv[i]) == 0) {
            key = atoi(argv[++i]);             
            printf("Find< %i >\n", key);            
        }
        if(strcmp("-store",argv[i]) == 0) {
            key = atoi(argv[++i]);
            address = argv[++i];
            printf("Store< %i, %s >\n", key, address);            
        }
        if(strcmp("-p",argv[i]) == 0) {
        	getKey = atoi(argv[++i]);
            printf("Get Previous of: %i\n", getKey);            
        }

    }
    puts("-------------------------------");
}


int main(int argc, char *argv[])
{
	long SIZE;
    int sockfd,  n;
    struct Frame frame = {};

    parse_options(argc, argv);

    sockfd = connectSocket(ip_address);

    // Connection stabilished. Sending message requesting frame size:
    strcpy(buffer,"Requesting frame size...");
    sendMessage(sockfd, buffer);

    // Reading message from server:
    receiveMessage(sockfd, buffer);
    printf("Message size: %s\n", buffer);
    SIZE = strtol (buffer,NULL,10);

    if(key > 0) {

        if (address) {
            // send the store operation:
            strcpy(buffer,"store");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving confirmation:
            receiveFrame(&frame, sockfd);
            strcpy(buffer, frame.data);
            printf("Confirmation from server: %s \n", buffer);

            // send the key:
            printf("Key: %i\n", key);
            strcpy(buffer,(char*) &key);
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving confirmation:
            receiveFrame(&frame, sockfd);
            strcpy(buffer, frame.data);
            printf("Confirmation from server: %s \n", buffer);

            // send the address:
            strcpy(buffer,address);
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the value:
            receiveFrame(&frame, sockfd);
			strcpy(buffer, frame.data);           
            printf("Return from server: %s (%d bytes)\n", buffer, (int) frameSize(&frame));


        } else if (getKey){
            // send the find operation:
            strcpy(buffer,"get node");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving confirmation:
            receiveFrame(&frame, sockfd);
            strcpy(buffer, frame.data);
            printf("Confirmation from server: %s \n", buffer);

            // send the key:
            strcpy(buffer,(char*) &key);
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the value:
            receiveFrame(&frame, sockfd);
			getKey = atoi(frame.data);
            printf("Return from server: %i (%d bytes)\n", getKey, (int) frameSize(&frame));

			return getKey;

        } else {
            // send the find operation:
            strcpy(buffer,"find");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving confirmation:
            receiveFrame(&frame, sockfd);
            strcpy(buffer, frame.data);
            printf("Confirmation from server: %s \n", buffer);

            // send the key:
            strcpy(buffer,(char*) &key);
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the value:
            receiveFrame(&frame, sockfd);
            key = atoi(frame.data);
            printf("Return from server: %i (%d bytes)\n", key, (int) frameSize(&frame));

			return key;

        }

    }
	
	if (filename) {
	    // send the store operation:
        strcpy(buffer,"send");
        createFrame(&frame, buffer);
        sendFrame(&frame, sockfd, frameSize(&frame));

         // receiving confirmation:
        receiveFrame(&frame, sockfd);
        strcpy(buffer, frame.data);
        printf("Confirmation from server: %s \n", buffer);

        // with socket size negotiated, send filename:
        strcpy(buffer,filename);
        createFrame(&frame, buffer);
        sendFrame(&frame, sockfd, frameSize(&frame));

        // And receiving confirmation:
        receiveFrame(&frame, sockfd);
        strcpy(buffer, frame.data);
        printf("Return from server: %s (%d bytes)\n", buffer, (int) frameSize(&frame));

        // Creating the file
        char comand[] = "cat /var/log/*.log | grep ";
        strcat(comand, expression);
        strcat(comand, " > ");
        strcat(comand, filename);
        system(comand);

        // Opening message file to read bytes and send them:
        FILE* msgFile;
        msgFile = fopen(filename,"rb");
        int msgFd = fileno(msgFile);
        if( !msgFile | msgFd < 0)
            error("File doesn't exist");

        // Finally, Sending files:
        int c;
        bzero(buffer,BUFFER_SIZE);
        size_t nbytes = fread(buffer, sizeof(char), MAX_DATA_SIZE-1, msgFile);
        while (nbytes > 0){
            createFrame(&frame, buffer);
            printf("\nsending message of %li bytes to server...\n", strlen(frame.data));
            sendFrame(&frame, sockfd, frameSize(&frame));
            bzero(buffer,BUFFER_SIZE);
            nbytes = fread(buffer, sizeof(char), MAX_DATA_SIZE, msgFile);
        }
        printf("File sent.\n");
        fclose(msgFile);
    }
    close(sockfd);
    return 0;
}












