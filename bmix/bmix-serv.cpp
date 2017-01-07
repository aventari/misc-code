#include <iostream.h>
#include <fstream.h>
#include <sys/types.h>
#include <sys/stat.h> //to get file size
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 32768
#define PORT_NUM 4242


int serve_files(int);
char menu();

int main(int argc, char *argv[])
{
  int sockd, rc, newsd, cliLen;
  struct sockaddr_in cliAddr, servAddr;
  socklen_t addrsize;
  pid_t child_pid;
  
  sockd=socket(AF_INET, SOCK_STREAM, 0);
  if(sockd<0)
  {
    cout<<argv[0]<<": cannot open socket\n";
    return(1);
  }

  servAddr.sin_family=AF_INET;  //bind any port number
  servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servAddr.sin_port=htons(PORT_NUM);
  rc=bind(sockd, (struct sockaddr*)&servAddr,sizeof(servAddr));
  if(rc<0)
  {
    cout<<argv[0]<<": cannot bind TCP port "<<PORT_NUM<<endl;
    return(1);
  }
  listen(sockd,5);
  while(1)
  {
    cout<<argv[0]<<": Waiting for connection on port "<<PORT_NUM<<endl;
    cliLen=sizeof(cliAddr);

    newsd=accept(sockd,(struct sockaddr*)&cliAddr,(socklen_t*) &cliLen);
    child_pid=fork();
    if(child_pid !=0) //we are the paren-need to wait for another connection
    {
      cout<<argv[0]<<" Accepted connection. Spawned proccess "<<child_pid<<endl; 
    }
    else // we are the child proccess and need to maintain connection
    {
      if(newsd<0)
      { 
        cout<<argv[0]<<": cannot accept connection\n";
        return(1);
      }
      serve_files(newsd);
      return(0);
    }
  }
}
    
    
int serve_files(int cliSd)
{
  unsigned char msg[1500];
  unsigned char buf[BUFFER];
  ifstream mp3;
  struct stat results;
  int size;
  
  if(stat("./mp3.mp3",&results)==0)
    size=(int)results.st_size;
  else
    cout<<"error reading filesize\n";
  cout<<"size: "<<size<<endl; 
  

  while(1){
  mp3.open("mp3.mp3",ios::in|ios::binary);
  memset(msg,0x0,1500);
  if(recv(cliSd,msg, 1500, 0)<0)
    cout<<" Error recieving from client\n";
  if(msg[0]=='K' && msg[1]=='e' && msg[2]=='V')
    cout<<"Client sent good string!\n";
  else
    cout<<"Client sent: "<<msg<<endl;
  if(send(cliSd,"KeV",3,0)<0)
    cout<<"error sending test response to client\n";
  
  /*should fork a proccess here on a new connection to send the mp3*/
  /*leave this one open for control messages*/
  
  if(recv(cliSd,msg, 1500, 0)<0)
    cout<<" Error recieving from client\n";
  if(msg[0]=='G' && msg[1]=='o' && msg[2]=='G')
    cout<<"Client sent good string!\n";
  else
    cout<<"Client sent: "<<msg<<endl;
  while(mp3.read(buf, BUFFER)>0)
  {
    send(cliSd,buf,BUFFER,0);
  }
  mp3.close();
}
    
}


