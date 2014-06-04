

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display();


int peertcpSocket = -1;	// peer socket

int main(int argc, char **argv)
{
	
    int tcpServ_sock;
    
    struct sockaddr_in tcpServer_addr;
    struct sockaddr_in tcpClient_addr;
    struct sockaddr_in newTcp_addr;
    
    int clnt_len;
    
    fd_set reads, temps;
    int fd_max;
    
    char command[1024];
    
    
    char *tcpport = argv[1];
    char *userid = argv[2];
    char oper[1024];
    
    // NEED TO ADD SOME VARIABLES 
    char ip[30], port[10];
    struct hostadd* hostpointer;
    char buf[BUFSIZ];
    int message;
    
    
    if(argc != 3){
        printf("Usage : %s <tcpport> <userid>\n", argv[0]);
        exit(1);
    }
    
    
    display();
	
    
	// NEED TO CREATE A SOCKET FOR TCP SERVER    
    tcpServ_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(tcpServ_sock < 0){
        perror("socket");
        exit(1);
    }
    
    
    
    
	// NEED TO bind
    memset( &tcpServer_addr, 0, sizeof(tcpServer_addr));
    tcpServer_addr.sin_family = PF_INET;
    tcpServer_addr.sin_port = htons(atoi(tcpport));
    tcpServer_addr.sin_addr.s_addr = htonl( INADDR_ANY);
    
    if(bind(tcpServ_sock, (struct sockaddr*)&tcpServer_addr, sizeof(tcpServer_addr)) < 0){
        perror("bind");
        exit(1);
    }
    
    
	// NEED TO listen
    if(listen(tcpServ_sock, SOMAXCONN) < 0){
        perror("listen");
        exit(1);
    }
    
    
	// initialize the select mask variables and set the
	// mask with stdin and the tcp server socket
    
    FD_ZERO(&reads);
    FD_SET(fileno(stdin), &reads);
    FD_SET(tcpServ_sock, &reads);
    fd_max = tcpServ_sock;
    
    printf("%s> \n", userid);
    
    while(1)
    {
        int nfound;
        
        temps = reads;
        
        nfound = select(fd_max+1, &temps, 0, 0, NULL);
        
        if(FD_ISSET(fileno(stdin), &temps)) {
            // Input from the keyboard
            fgets(command, sizeof (command), stdin);
            FD_CLR(fileno(stdin), &temps);
            
            
            // NEED TO IMPLEMENT for input from keybord
            if(strncmp(command, "@talk", 5) == 0){      // 대화 연결 하기위해 talk
                peertcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if(peertcpSocket < 0){
                    perror("peersocket");
                    exit(1);
                }
                
                sscanf(command, "@talk %s %s", ip, port); //talk ip주소 port주소를 하면 상대방과 연결
                hostpointer = gethostbyname(ip);
                if(hostpointer == 0){
                    perror("host ip");
                    exit(1);
                }
                
                memset(&tcpClient_addr, 0, sizeof(tcpClient_addr));
                tcpClient_addr.sin_family = PF_INET;
                tcpClient_addr.sin_port = htons((u_short)atoi(port));
                memcpy((void*)&tcpClient_addr.sin_addr, hostpointer->h_addr, hostpointer->h_length);
                
                if(connect(peertcpSocket, (struct sockaddr*)&tcpClient_addr, sizeof(tcpClient_addr)) < 0){
                    close(peertcpSocket);
                    exit(1);
                }
                FD_SET(peertcpSocket, &reads);
                
                if(peertcpSocket > fd_max)
                    fd_max = peertcpSocket;
                
            }            
            else if(strncmp(command, "@quit", 5) == 0){     //quit 대화 끝내기
                break;
            }
            else{       // write    메세지 보내기
                memset(oper, 0, sizeof(oper));
                strcat(oper , userid);
                char *m = strcat(oper, " : ");
                strcat(m, command);
                int nwrite = write(peertcpSocket, m, strlen(m));
            }
            printf("%s>\n", userid);
        }
        else if(FD_ISSET(tcpServ_sock, &temps))
        {
            //connect request from a peer
            FD_CLR(tcpServ_sock, &temps);
            
            clnt_len = sizeof(tcpClient_addr);
            peertcpSocket = accept(tcpServ_sock, (struct sockaddr*)&tcpClient_addr, &clnt_len);
            if(peertcpSocket < 0){
                perror("accept");
                exit(1);
            }
            
            printf("connection from host %s, port %d, socket %d\n", inet_ntoa(tcpClient_addr.sin_addr), ntohs(tcpClient_addr.sin_port), peertcpSocket);
            FD_SET(peertcpSocket, &reads);
            fd_max = (peertcpSocket > fd_max) ? peertcpSocket : fd_max;
            
        }
        else if(FD_ISSET(peertcpSocket, &temps))
        { 
            
            // message from a peer
            
            message = read(peertcpSocket, buf, sizeof(buf));
            
            if(message == 0){   // 연결끊기
                printf("Connection closed %d\n", peertcpSocket);
                close(peertcpSocket);
                FD_CLR(peertcpSocket, &reads);
                fd_max;
            }
            else{   // 메세지 받기
                buf[message] = 0;
                printf("%s", buf);
                memset(&buf, 0, sizeof(buf));
                
            }
        }
        
    }//while End
}//main End

void display()
{
	printf("Student ID : 20113343 \n");
	printf("Name : Hyun Jae Young \n");
}



