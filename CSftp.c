
#include "dir.h"
#include "usage.h"
#include "common.h"


//Create socket of specified port and start listening to it
int create_socket(int port) {
    struct sockaddr_in serv; /* socket info about our server */
    int mysocket;            /* socket used to listen for incoming connections */
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    serv.sin_port = htons(port);           /* set the server port number */

    mysocket = socket(AF_INET, SOCK_STREAM, 0);

            // Receive cmd
            /* bind serv information to mysocket */

    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

            /* start listening, allowing a queue of up to 1 pending connection */
    listen(mysocket, 1);

    return mysocket;
}

//Get the ip address of the sock
void getip(int sock, int *ip)
{
  socklen_t addr_size = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  getsockname(sock, (struct sockaddr *)&addr, &addr_size);

  char* host = inet_ntoa(addr.sin_addr);
  sscanf(host,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
}

//Generate a random port
void random_port(Port *port)
{
    srand(time(NULL));
    port->p1 = 128 + (rand() % 64);
    port->p2 = rand() % 0xff;

}


// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

int main(int argc, char **argv) {

    // This is some sample code feel free to delete it
    // This is the main program for the thread version of nc

    int i;

        // Check the command line arguments
    if (argc != 2) {
      usage(argv[0]);
      return -1;
    }

    int PORTNUM = atoi(argv[1]);

    char* msg = "Hello World !\n";

    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv; /* socket info about our server */
    int mysocket = create_socket(PORTNUM);            /* socket used to listen for incoming connections */

            // Receive cmd
    char send_buffer[BUFFERSIZE],receive_buffer[BUFFERSIZE];
    int bytes;

   socklen_t socksize = sizeof(struct sockaddr_in);

    while(1){
        int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
    
        printf("accepted a connection from client IP %s port %d \n", inet_ntoa(dest.sin_addr), (int) ntohs(dest.sin_port));
        sprintf(send_buffer,"\n==220 Welcome to Bowen and Rock's FTP site== \r\n\n");
        bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
        int pasv_socket= 0;
        int loggin=0;
        int pasv=0;
        
        while(1){
        int n=0;
            while(1){
                //Receive cmd from client byte by byte
                bytes = recv(consocket,&receive_buffer[n],1,0);
            //PROCESS REQUEST
                if ( bytes <= 0 ) break;
                if (receive_buffer[n] == '\n'){ /*end on a LF*/
                    receive_buffer[n] = '\0';
                    break;
                }
                if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
            }

            if ( bytes <= 0 ) break;
            int i = 0;
            while(i<=4) {
                receive_buffer[i] = toupper(receive_buffer[i]);
                i++;
            }
            //QUIT
            if (strncmp(receive_buffer,"QUIT", 4)==0) {
                printf("Client quits \n");
                sprintf(send_buffer,"221 Goodbye. \n");
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                if (bytes < 0) break;
                close(consocket);
            }
            
            // indicate not implemented command
            if (strncmp(receive_buffer,"USER",4)&&strncmp(receive_buffer,"QUIT",4)
                &&strncmp(receive_buffer,"TYPE",4)&&
                strncmp(receive_buffer,"MODE",4)&&strncmp(receive_buffer,"STRU",4)&&
                strncmp(receive_buffer,"RETR",4)&&strncmp(receive_buffer,"PASV",4)&&
                strncmp(receive_buffer,"NLST",4)){
                printf("Invalid command \n");
                sprintf(send_buffer,"500 Unknown command \n");
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                if (bytes < 0) break;
                // incorrect username
            }else if (strncmp(receive_buffer,"USER",4)==0 &&strncmp(&receive_buffer[5],"cs317",5)!=0){
                printf("invalid user name \n");
                sprintf(send_buffer,"530 This FTP server is cs317 only \n");
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                if (bytes < 0) break;
                // correctly loggin
            }else if (strncmp(receive_buffer,"USER",4)==0 && strncmp(&receive_buffer[5],"cs317",5)==0){
                printf("receive_buffer: %s \n",receive_buffer);
                printf("Logging in \n");
                sprintf(send_buffer,"230 login successful \r\n");
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                loggin=1;
                if (bytes < 0) break;
                //if user's command is implemented but not logged in, remind them to loggin first
            }else if (loggin==0) {
                printf("user not loggin \n");
                sprintf(send_buffer,"530 Please login with USER and PASS \n");
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                if (bytes < 0) break;
            }
            
            
            //TYPE
            if (strncmp(receive_buffer,"TYPE",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n", receive_buffer);
                printf("Type \n");
                //we only support binary and ascii types now.
                char arg = (char) toupper(receive_buffer[5]);
                if (strncmp(&arg,"I",1)==0) {
                    sprintf(send_buffer,"200 Switching to Binary mode. \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                } else if(strncmp(&arg, "A", 1) == 0) {
                    sprintf(send_buffer, "200 Switching to ASCII mode. \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                } else {
                    sprintf(send_buffer,"500 Unrecognised TYPE command. \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                }
                if (bytes < 0) break;
            }

            //MODE
            if (strncmp(receive_buffer,"MODE",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n",receive_buffer);
                printf("Mode \n");
                //we only accept S

                char arg = (char) toupper(receive_buffer[5]);
                if (strncmp(&arg,"S",1)==0) {
                    sprintf(send_buffer,"200 Mode set to S \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                } else{
                    sprintf(send_buffer,"504 Bad MODE command. \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                }
                if (bytes < 0) break;
            }

            //STRU
            if (strncmp(receive_buffer,"STRU",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n", receive_buffer);
                printf("Stru \n");
                //We only accept F

                char arg = (char) toupper(receive_buffer[5]);
                if (strncmp(&arg,"F",1)==0) {
                    sprintf(send_buffer,"200 Structure set to F \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                } else{
                    sprintf(send_buffer,"504 Bad STRU command \r\n");
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                }
                if (bytes < 0) break;
            }
            
            //PASV
            if (strncmp(receive_buffer,"PASV",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n", receive_buffer);
                int ip[4];
                Port *port = malloc(sizeof(Port));
                char *response = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n";
                random_port(port);
                getip(consocket, ip);
                close(pasv_socket);
                pasv_socket = create_socket((256*port->p1) + port->p2);
                printf("port: %d\n",256*port->p1+port->p2);
                sprintf(send_buffer,response,ip[0],ip[1],ip[2],ip[3],port->p1,port->p2);
                bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                pasv = 1;
                if (bytes < 0) break;
            }
            
            //RETR
            if (strncmp(receive_buffer,"RETR",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n", receive_buffer);
                printf("RETR \n");
                off_t offset = 0;
                
                char filename[BUFFERSIZE];
                strcpy(filename, &receive_buffer[5]);
                printf("receive_FILENAME: %s \n", filename);
                
                if (pasv!=1) {
                    sprintf(send_buffer, "425 Use PASV first \n");
                    bytes=send(consocket,send_buffer,strlen(send_buffer),0);
                } else if(pasv_socket<0) {
                    sprintf(send_buffer, "PASV server is not on \n");
                    bytes=send(consocket, send_buffer, strlen(send_buffer), 0);
                } else {
                    struct stat stat_buf;
                    int fd;
                    fd = open(filename,O_RDONLY);
                    fstat(fd,&stat_buf);
                    int pasv_consocket = accept(pasv_socket, (struct sockaddr *)&dest, &socksize);
                    sendfile(pasv_consocket, fd, &offset, stat_buf.st_size);
                    dprintf(pasv_consocket,"Process completed. Press any key to continue \npause \n\n");
                    close(pasv_socket);
                    close(pasv_consocket);
                    sprintf(send_buffer,"150 Open BINARY mode data connection for %s (%d bytes) \n226 File send OK. \n", filename, stat_buf.st_size);
                    bytes = send(consocket, send_buffer, strlen(send_buffer), 0);
                    
                }
                
                if (bytes < 0) break;
            }

            //NLST
            if (strncmp(receive_buffer,"NLST",4)==0 && loggin==1) {
                printf("receive_buffer: %s \n", receive_buffer);
                printf("NLST \n");
                
                if (pasv!=1) {
                    sprintf(send_buffer, "425 Use PASV first \n");
                    bytes=send(consocket,send_buffer,strlen(send_buffer),0);
                } else if(pasv_socket<0){
                    sprintf(send_buffer, "PASV server is not on \n");
                    bytes=send(consocket, send_buffer, strlen(send_buffer), 0);
                } else{
                    int pasv_consocket = accept(pasv_socket, (struct sockaddr *)&dest, &socksize);
                    listFiles(pasv_consocket,".");
                    close(pasv_socket);
                    close(pasv_consocket);
                    sprintf(send_buffer, "150 Here comes the listing. \n226 Directory send OK \n");
                    bytes=send(consocket, send_buffer, strlen(send_buffer),0);
                    pasv =0;
                }
                    if (bytes < 0) break;
            }
        }
    }

close(mysocket);
return EXIT_SUCCESS;

    // This is how to call the function in dir.c to get a listing of a directory.
    // It requires a file descriptor, so in your code you would pass in the file descriptor 
    // returned for the ftp server's data connection


}
