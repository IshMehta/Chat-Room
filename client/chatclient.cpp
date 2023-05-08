/**
 * Author: Ish Mehta
 */

#include <getopt.h>
#include <iostream>
#include <udt.h>
#include <cstdlib>
#include <netdb.h>
#include <string.h>

using namespace std;

  int c;
  int join = 0;
  string port;
  string hostname;
  string passcode;
  char* str;
  string username;
  UDTSOCKET client;

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
    cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
    return 0;
  }
  return 1;
}


// thread which constantly listens to the server
void* serverListen(void* agr)
{
  
  //  UDTSOCKET recver = *(UDTSOCKET*)usocket;
  //  delete (UDTSOCKET*)usocket;
   char messageRecv[1024];
   int msgLen;
   while (true) 
   {
    msgLen = 0;
    std::fill_n(messageRecv, 1024, 0);
    // std::fill_n(messageRecv, 1024, 0);
    // memset(messageRecv, 0, sizeof(messageRecv));
    
    // int messageLen;
    
    // if (recvMessage(client, (char*)&messageLen, sizeof(int) , 0 )==0) return 0;
    // cout<<messageLen<<'\n';
    // if (recvMessage(client, messageRecv, 1024 , 0 )==0) return 0;
    if (recvMessage(client, (char*)&msgLen, sizeof(int),0) == 0) { return 0; };
    if (recvMessage(client, messageRecv, msgLen,0) == 0) { return 0; };
    messageRecv[msgLen] = '\0';
    cout<< messageRecv << "\n";
   }

   UDT::close(client);
  UDT::cleanup();

      return NULL;
}

// thread which constantly listens for user input to send to server 
void* chatService(void* usocket)
{
  
  //  UDTSOCKET recver = *(UDTSOCKET*)usocket;
  //  delete (UDTSOCKET*)usocket;

  while (1) {
  string chatMessage;
  getline (std::cin, chatMessage);
    // cin >> chatMessage;
    
    int msgLen = chatMessage.length();

    if (sendMessage(client, (char*)&msgLen, sizeof(int)) == 0) { return 0; };
    if (sendMessage(client, (char*) chatMessage.c_str(), msgLen) == 0) { return 0; };

    if (strcmp(chatMessage.c_str(), ":Exit") == 0)
    {
      UDT::close(client);
      exit(3);
      return NULL;
    }

}
  //  UDT::close(client);
  // UDT::cleanup();
  //  #ifndef WIN32
  //     return NULL;
  //  #else
      // return 0;
  //  #endif
}

int main(int argc, char *argv[]) {

  // ########### process inputs #################

        // int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"join",    no_argument, NULL,  'a' },
            {"host",  required_argument, NULL,  'b' },
            {"username",  required_argument, NULL,  'c' },
            {"passcode",  required_argument, NULL,  'd' },
            {"port",  required_argument, NULL,  'e' }
        };

       
        while((c = getopt_long_only(argc, argv, "a:b:",
                 long_options, &option_index))!=-1) {

          switch (c) {
          case 'a':
            join = 1;
            break;
          case 'b':
                hostname = optarg;
                
                break;
          case 'c':
                username = optarg;
                break;
          case 'd':
                passcode = optarg;
                // cout << passcode << '\n';
                break;
          case 'e':
                port = optarg;
                break;
          default:
                break;
            }
        }

  // ########### connect client socket to server #################


  UDT::startup();
  struct addrinfo hints, *peer;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  client = UDT::socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (0 != getaddrinfo(hostname.c_str(), port.c_str(), &hints, &peer))
  {
    std::cout << "incorrect server/peer address. " << hostname << ":" << port << std::endl;
    return 0;
  }


  if (UDT::ERROR == UDT::connect(client, peer->ai_addr, peer->ai_addrlen))
{
  cout << "connect: " << UDT::getlasterror().getErrorMessage();
  return 0;
}
  freeaddrinfo(peer);

// ########### initialise handshake for password verificaiton #################

// send passcode to server, first send passcode length
str = (char*) passcode.c_str();
int passcodeLen = strlen(passcode.c_str()) + 1;
if (sendMessage(client, (char*)&passcodeLen , sizeof(int)) == 0) return 0;
if (sendMessage(client, (char*)str, strlen(str)) == 0) return 0;



// if reply from server is a 1, then it means that the password is correct
char buf[1024];
if (recvMessage(client, buf, 2, 0)==0) return 0;
// check value of ack returned in order to 
char ack[] = {'1','\0'};
if (strcmp(buf, ack)==0) {
    cout<<"Connected to "<<hostname<< " on port "<<port<<'\n';
} else {
    cout<< "Incorrect passcode\n";
    return 0;
}

// if password is correct, then chat client is still active
str = (char*) username.c_str();
int usernameLen = strlen(username.c_str()) + 1;
if (sendMessage(client, (char*)&usernameLen , sizeof(int)) == 0) return 0;
if (sendMessage(client, (char*)str, strlen(str)+1) == 0) return 0;

// ########### Once addmitted to chatroom, create threads and begin infinite loop #################

// create thread for listening to server 
// #ifndef WIN32
    pthread_t servthread;
    pthread_create(&servthread, NULL, serverListen, NULL);
    pthread_detach(servthread);
// #else
//     CreateThread(NULL, 0, serverListen, new UDTSOCKET(client), 0, NULL);
// #endif

// create thread for user chat service
// #ifndef WIN32
    pthread_t sendthread;
    pthread_create(&sendthread, NULL, chatService, NULL);
    pthread_detach(sendthread);
// #else
//     CreateThread(NULL, 0, chatService, new UDTSOCKET(client), 0, NULL);
// #endif


while (1) {
 
}
  return 0;
}


