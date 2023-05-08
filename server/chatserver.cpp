/**
 * Author: Ish Mehta
 * GTID: 903531579
 * GT Email: imehta34@gatech.edu
 */

// #include <udt.h>
// #ifndef WIN32
//    #include <unistd.h>
//    #include <cstdlib>
//    #include <cstring>
//    #include <netdb.h>
// #else
//    #include <winsock2.h>
//    #include <ws2tcpip.h>
//    #include <wspiapi.h>
// #endif


#include <getopt.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <udt.h>
#include <iostream>
#include <cstdlib>
#include <netdb.h>
#include <time.h>

using namespace std;
int c; // for get opt
string port; // port number so that the thread can access it, possible race conditions 
string passcode; // since the passcode is only set once by the main thread, it can be accessed by any thread
UDTSOCKET serv;
   UDTSOCKET clientsList[5];  // keep track of the clients we have - track their sockets
   int num_clients = 0;

  

   // Function to receive messages from a socket
  int recvMessage(UDTSOCKET u, char* const &buf, int length, int flag ) {
   if (UDT::ERROR == (UDT::recv(u, buf, length, flag)))
         {
            cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
            return 0;
         }
        return 1;
  }

   // Function to send messages to a socket
  int sendMessage(UDTSOCKET u, char *msg, int length) {
    if (UDT::ERROR == UDT::send(u, msg, length, 0))
  {
    std::cout << "send: " << UDT::getlasterror().getErrorMessage() << std::endl;
    return 0;
  }
  return 1;
}

void* clientHandler(void* usocket)
{

   UDTSOCKET recver = *(UDTSOCKET*)usocket;
   delete (UDTSOCKET*)usocket;

   // check for passcode, accept connection if correct, notify client
   char passcodeRecv[1024];
   int passcodeLen;
   

   // since we don't know the length, the client will first send the lenght of the passcode
   if (recvMessage(recver, (char*)&passcodeLen, sizeof(int) , 0 )==0) return 0;
   // using this lenght, the receiver can now store the passcode
   if (recvMessage(recver, passcodeRecv, passcodeLen , 0 )==0) return 0;
   passcodeRecv[passcodeLen] = '\0';

   // cout<< "data received: "<< passcodeRecv << "\n";
   
   // once password is recevied, check if it is correct
   

   if (strcmp( passcode.c_str(), passcodeRecv)==0) {
      // cout<<"string matches\n";
      char ack[] = {'1','\0'};
      if (sendMessage(recver, (char*)ack, strlen(ack)+1) == 0) return 0;
   } else {
      // cout << "string doesnt match\n";
      char ack[] = {'0','\0'};
      if (sendMessage(recver, (char*)ack, strlen(ack)+1) == 0) return 0;
      return NULL;
   }


   // if accepted, server will now wait for the username 
   // once username is revceived, it will broadcast on the chatroom that the username has been added
   char usernameRecv[1024];
   int usernameLen;
   if (recvMessage(recver, (char*)&usernameLen, sizeof(int) , 0 )==0) return 0;
   if (recvMessage(recver, usernameRecv, usernameLen , 0 )==0) return 0;
   passcodeRecv[usernameLen] = '\0';



        // before we create threads, we want to add their socket to the clients list
      clientsList[num_clients] = recver;
      num_clients++;

   // server will also send a message to everyone saying "<username> joined the chatroom"

   std::string message = std::string(usernameRecv) + " joined the chatroom";
  int msgLen = message.length();
  std::cout << message << std::endl; // to print out on the server
     for (int i = 0; i < num_clients; i++)
  {
    UDTSOCKET curRecver = clientsList[i];
    if (curRecver != recver) { 
      if (sendMessage(curRecver, (char*)&msgLen, sizeof(int)) == 0) { return 0; };
      if (sendMessage(curRecver, (char*) message.c_str(), msgLen) == 0) { return 0; };
    } // don't send message to client that sent the message
   // cout<<"hereeeeeeee\n";
    
  }

   // we now wait to receive a message from any client
   // when we get a message, we output to the server and send it to all other clients
   while (true) {
      // listen for message
      char chatReceived[1024];
      int chatLen = 0; 
      std::fill_n(chatReceived, 1024, 0);
      
      if (recvMessage(recver, (char*)&chatLen, sizeof(int), 0) == 0) { return 0; };
      if (recvMessage(recver, chatReceived, chatLen,0) == 0) { return 0; };
      chatReceived[msgLen] = '\0';
      // cout<<usernameRecv << ": " << chatReceived << '\n';
      // send message to everyone else too
      string toSend;

      if (strcmp(chatReceived, ":)")==0) 
      {
         
         toSend = string(usernameRecv) + ": " + "[feeling happy]";
         cout<<toSend << '\n';
      } else if (strcmp(chatReceived, ":(")==0) {
         toSend = string(usernameRecv) + ": " + "[feeling sad]";
         cout<<toSend << '\n';
      } else if (strcmp(chatReceived, ":mytime")==0) {
         time_t currtime;
         time(&currtime);
         toSend = string(ctime(&currtime));
         cout<<toSend;
      //display the current time 
      } else if (strcmp(chatReceived, ":+1hr")==0) {
         // display current time + 1 hr
         time_t currtime;
         time(&currtime);
         struct tm *local_time = localtime(&currtime);
         local_time->tm_hour++;
         toSend = asctime(local_time);
         cout<<toSend;
      }
      else{
         toSend = string(usernameRecv) + ": " + string(chatReceived);
         cout<<toSend << '\n';
      }
      
      int toSendLen = toSend.length();
      for (int i = 0; i < num_clients; i++)
      {
         UDTSOCKET curRecver = clientsList[i];
         if (curRecver != recver) { 
      if (sendMessage(curRecver, (char*)&toSendLen, sizeof(int)) == 0) { return 0; };
      if (sendMessage(curRecver, (char*) toSend.c_str(), toSendLen) == 0) { return 0; };
    }
      }




   }
      return 0;
   // #endif
}





int main(int argc, char *argv[]) {
  
  // ########### process inputs #################

        // int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"port",    required_argument, NULL,  'a' },
            {"passcode",  required_argument, NULL,  'b' }
        };
        while((c = getopt_long_only(argc, argv, "a:b:",
                 long_options, &option_index))!=-1) {
          switch (c) {
          case 'a':
               port = optarg;
                break;
          case 'b':
                passcode = optarg;
                break;
          default:
                break;
            }
        }

   // ########### code to start server #################
   UDT::startup();
  // assign socket
  addrinfo hints;
  addrinfo* res;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
    if (0 != getaddrinfo(NULL, port.c_str(), &hints, &res))
  {
    std::cout << "illegal port number or port is busy.\n" << std::endl;
    return 0;
  }
  printf("Server started on port %s. Accepting connections\n", port.c_str());

 serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  // bind socket
if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen))
  {
    cout << "bind: " << UDT::getlasterror().getErrorMessage();
    return 0;
  }
  

  freeaddrinfo(res);

   // ########### Listen for connections #################

  // listen on socket created, allow a maximum of 5 connections
     if (UDT::ERROR == UDT::listen(serv, 5))
   {
      cout << "listen: " << UDT::getlasterror().getErrorMessage() << endl;
      return 0;
   }

   sockaddr_storage clientaddr;
   int addrlen = sizeof(clientaddr);

  // when you receive a connection, you accept it
   UDTSOCKET recver;
    while (true)
   {
      if (UDT::INVALID_SOCK == (recver = UDT::accept(serv, (sockaddr*)&clientaddr, &addrlen)))
      {
         cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
         return 0;
      }
      // cout<< "Accepted connection \n";
      char clienthost[NI_MAXHOST];
      char clientservice[NI_MAXSERV];
      getnameinfo((sockaddr *)&clientaddr, addrlen, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST|NI_NUMERICSERV);

         pthread_t rcvthread;
         pthread_create(&rcvthread, NULL, clientHandler, new UDTSOCKET(recver));
         pthread_detach(rcvthread);

   }
  UDT::close(serv);
   UDT::cleanup();
  return 0;
}

