///////////////**************linux socket*********/////
// #include <ros/ros.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <stdio.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <string.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <sys/shm.h>

// #define MYPORT  9999
// #define BUFFER_SIZE 1024
// int main(int argc, char* argv[])
// {
//     int sock_cli;
//     fd_set rfds;
//     struct timeval tv;
//     int retval, maxfd;

//     ros::init(argc, argv, "rcclient");
//     ros::NodeHandle nh;

//     /// Define sockfd
//     sock_cli = socket(AF_INET,SOCK_STREAM, 0);
//     /// Define sockaddr_in
//     struct sockaddr_in servaddr;
//     memset(&servaddr, 0, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(MYPORT);  /// Server Port
//     servaddr.sin_addr.s_addr = inet_addr("192.168.0.6");  /// server ip

//     //Connect to the server, successfully return 0, error return - 1
//     if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
//     {
//         perror("connect");
//         exit(1);
//     }
//     char recvbuf[BUFFER_SIZE];
//     char sendbuf[BUFFER_SIZE];
//     memset(sendbuf, 0, sizeof(sendbuf));
//     memset(recvbuf, 0, sizeof(recvbuf));
//     while (ros::ok())
//     {
//         /*Clear the collection of readable file descriptors*/
//         FD_ZERO(&rfds);
//         /*Add standard input file descriptors to the collection*/
//         FD_SET(0, &rfds);
//         maxfd = 0;
//         /*Add the currently connected file descriptor to the collection*/
//         FD_SET(sock_cli, &rfds);
//         /*Find the largest file descriptor in the file descriptor set*/    
//         if(maxfd < sock_cli)
//             maxfd = sock_cli;
//         /*Setting timeout time*/
//         tv.tv_sec = 0.001;
//         tv.tv_usec = 0;
//         /*Waiting for chat*/
//         retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
//         if(retval == -1){
//             printf("select Error, client program exit\n");
//             break;
//         }
//         // else if(retval == 0){
//         //     //printf("The client does not have any input information, and the server does not have any information coming. waiting...\n");
//         //     continue;
//         // }
//         else{
//             //printf("sock_cli:%d",sock_cli);
//             /*The server sent a message.*/
//             if(FD_ISSET(sock_cli,&rfds)){
                
//                 int len;
//                 len = recv(sock_cli, recvbuf, BUFFER_SIZE,0);
//                 printf("%s", recvbuf);
                
//             }
//             /*When the user enters the information, he begins to process the information and send it.*/
//             if(FD_ISSET(0, &rfds)){
//                 printf("sock_cli:%d",sock_cli);
//                 //gets_s(sendbuf);
//                 fgets(sendbuf, sizeof(sendbuf), stdin);
//                 send(sock_cli, sendbuf, strlen(sendbuf) + sizeof(char),0); //Send out
                
//             }
//         }
//     }

//     close(sock_cli);
//     ros::spin();
//     return 0;
// }

///////******************************linux client 
#include <ros/ros.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
//#include <pthread.h> //for threading , link with lpthread
using namespace std;
//Client side

//void *connection_handler(void *);
int main(int argc, char *argv[])
{
    printf("set ip & port");
    ros::init(argc, argv, "rcclient");
    ros::NodeHandle nh;
    printf("set ip & port");
    //we need 2 things: ip address and port number, in that order
    // if(argc != 3)
    // {
    //     cerr << "Usage: ip_address port" << endl; exit(0); 
    // } //grab the IP address and port number 
    // char *serverIp = argv[1]; int port = atoi(argv[2]); 
    char *serverIp = "192.168.0.6"; 
    int port = 9999; 
    printf("set ip & port");
    //create a message buffer 
    //char msg[1500]; 
    char msg[1024]; 
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
        ros::shutdown();
        return 0;  
        //break;
    }
    // pthread_t thread_id;
    // if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
    // {
    //     perror("could not create thread");
    //     return 1;
    // }

    cout << "Connected to the server!" << endl;
    int bytesRead, bytesWritten = 0;
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    while (ros::ok())
    {
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, data.c_str());
        if(data == "exit")
        {
            send(clientSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        bytesWritten += send(clientSd, (char*)&msg, strlen(msg)+ sizeof(char), 0);
        cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(clientSd, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, "exit"))
        {
            cout << "Server has quit the session" << endl;
            break;
        }
        cout << "Server: " << msg << endl;
    }
    gettimeofday(&end1, NULL);
    close(clientSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << 
    " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec) 
      << " secs" << endl;
    cout << "Connection closed" << endl;
    ros::spin();
    return 0;    
}