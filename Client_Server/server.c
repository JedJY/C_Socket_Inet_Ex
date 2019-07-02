/*//////////////////////////////////////////////////////////////////////////////////////////
 *	Name: Jed Yeung
 *	File: server.c
 *	Desc: server processes requests from more than one client concurrently.
 *	 This means different clients may request either the same service or a total 
 *	 different one. The services supported by your server will include: 
 *	 (1) changing all characters of a text file to uppercase, and 
 *   (2) counting the number of a supplied character in a text file. 
/*//////////////////////////////////////////////////////////////////////////////////////////

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

/*****************************************************************
 *    Function: error
 *    Return: void
 *    Param: msg to perror
 *    Desc: perror with msg and exit
 ****************************************************************/
void error(const char *msg){
	perror(msg);
    exit(0);
}

/*****************************************************************
 *	Function: toUpper
 *	Return: 0 success
 *	Param: sockfd, buf
 *	Desc: receives file and changes all characters in the file
 *     into caps
 ****************************************************************/
int toUpper(int sockfd, char* file){

    char c;
    int SIZE = 256;
    char buffer[SIZE];
    int n; //written bytes to socket
    int ind = 0; //str index

    // read for passed file contents in buffer
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading file from socket");
    else
        printf("File: %s has been read from client\n", file);


    while((c = buffer[ind]) != '\0'){
        if(ind > SIZE){
            error("ERROR out of bounds");
        }
        if((c >= 'a') && (c <= 'z')){ //isAlpha
            c -= 32; // convert to caps
        }
        buffer[ind++] = c;
    }
    buffer[ind] = 0;

    n = write(sockfd, buffer, sizeof(buffer));
            if (n < 0)
                error("ERROR writing to socket");

    printf("SERVICE COMPLETE\n");
	return 0;
}

/*****************************************************************
 *	Function: count
 *	Return: 0 success
 *	Param: sockfd, buf
 *	Desc: counts given character in file received
 ****************************************************************/
int count(int sockfd, char *cChar, char* file){
 
    int charCount = 0; //counter for given char
    int ind = 0; //index for loop
    int n; //written bytes to socket, read bytes from file;
    char buffer[256];
    char *filepath;
	char *character; // stores character that is counted

	character = strtok(cChar, ",");
    strtok(NULL, ",");
    filepath = strtok(file, " \n");
    strtok(NULL, " \n");

	char c; // stores char

    // read for passed file contents in buffer
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading file from socket");
    else
        printf("File: %s has been read from client\n", filepath);

    while((c = buffer[ind++]) != '\0'){
        if(c == character[0]){
            charCount++;
        }
    }

    sprintf(buffer, "%d", charCount); //int to string into buffer

    //write buffer to socket
    n = write(sockfd, buffer, sizeof(buffer));
            if (n < 0)
                error("ERROR writing to socket");

    printf("SERVICE COMPLETE\n");
	return 0;
}

int main(int argc, char *argv[])
{ // port 16 bits
    // addr 32 bits
    // sockfd to store file descriptor for open socket
     int sockfd, newsockfd, portno;
     socklen_t clilen;  //address size for client
     char buffer[256];
     char *token;
     char **tokens = malloc(sizeof(char *) * 5);
     int ind; //tokens index

     //sockaddr_in struct stores internet protocol addresses
     struct sockaddr_in serv_addr, cli_addr;
     int n; //stores number of bytes read/written from socket connection

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     
     //open socket domain: ipv4(family name space), protocol type: SOCK_STREAM(full duplex)
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
    //bzeroclears buffer, sets to \s0
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);

    /*
        INADDR_ANY (0.0.0.0) means any address for binding
        http://man7.org/linux/man-pages/man7/ip.7.html
        bind call with INADDR_ANY specified binds all local interfaces
     */
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     //htons and htonl converts bytes between big endian and little endian
     //network byte order is big endian(MSB)
     //ethernet frames and ipv4 are also MSB
     serv_addr.sin_port = htons(portno); //sets port to network byte order

     //binds local address to open socket (where to receive connections)
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    //set sockfd as passive socket , 
    //i.e., listening for incomming connection requests
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
      

     //to accept multiple clients
     while(1){
        fprintf(stdout, "Run client by providing host and port\n");
         /*accept - extracts the first connection
       request on the queue of pending connections for the listening socket,
       sockfd, creates a new connected socket, and returns a new file
       descriptor referring to that socket.*/
            //blocks until a client connects with the server
        ind = 0;
        tokens[0] = 0;
         newsockfd = accept(sockfd, 
                     (struct sockaddr *) &cli_addr, 
                     &clilen);
         if (newsockfd < 0) 
            error("Error: Connection not made with client.\n");
         bzero(buffer,256);

         //read from accepted socket connection into buffer
         n = read(newsockfd,buffer,255);
         if (n < 0) 
            error("Error: unable to read from socket.\n");

        //get first token of buffer to determine given option
         token = strtok(buffer, "<");
         while(token != NULL){
            tokens[ind++] = token;
            token = strtok(NULL, ",>\n");
         }
         tokens[ind] = 0;
        

        //check option
        if(strcmp(tokens[0], "toUpper ") == 0){
            toUpper(newsockfd, tokens[1]);
        }
        else if (strcmp(tokens[0], "count ") == 0){
            count(newsockfd, tokens[1], tokens[2]);
        }
        else{ //invalid option
            n = write(newsockfd, "Error: Invalid Service Option Provided",31);
            if (n < 0)
                error("ERROR writing to socket");
            bzero(buffer,256);
        }

        
        close(newsockfd); // close accepted connection
     }
    
    close(sockfd);// close open socket

    return 0; 
}
