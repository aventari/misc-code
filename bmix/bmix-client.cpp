#include <iostream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include "SDL.h"
#include "SDL_sound.h"

#define BUFFER    16384 //try to keep same as decodebuf
#define PACKET_SIZE 512
#define MAX_RECV  1500
#define DECODEBUF 16384 //was 16384
#define AUDIOBUF  4096

static int sockd, rc;
static struct sockaddr_in localAddr, servAddr;
static socklen_t addrsize;


//music variables
static volatile int done_flag=0;
static Uint8 *decoded_ptr=NULL;
static Uint32 decoded_bytes=0;
unsigned char buffer[BUFFER];

int playmp3(SDL_RWops *rw);
static void audio_callback(void *, Uint8 *,int);
static int read_more_data(Sound_Sample *);
void sigint_catcher(int);

SDL_RWops * SDL_RWFromNet(unsigned char *);
//these are the callbacks for the RWops that gets the data from the network
static int _net_read(SDL_RWops *context, void *ptr, int size, int maxnum)
{
  int bytes_received=0,togo,i;
  unsigned char * buf_ptr;
  //bad hack to move already streamed data into buffer if 'here' is 0
  //this only occurs on the first recieved 'packet'.
  //otherwise pull it off the network
  if(context->hidden.mem.here==0x0)
  {
    memcpy(ptr,context->hidden.unknown.data1, (maxnum*size));
    context->hidden.mem.here=(Uint8*)0x1;
    return(maxnum*size);
  }
  memset(context->hidden.unknown.data1, 0x0, (maxnum*size));
  togo=(size*maxnum);
  buf_ptr=(unsigned char*)context->hidden.unknown.data1;
  while(togo > PACKET_SIZE)
  {
    i=recv(sockd,buf_ptr,PACKET_SIZE,0);
    bytes_received+=i;
    if(!i) {
      cout<<"needed network data, didn't get any\n";
      break;
    }
    togo -= PACKET_SIZE;
    buf_ptr += PACKET_SIZE;
  }
  i=recv(sockd,buf_ptr,togo,0);
  bytes_received+=i;

//  bytes_received=recv(sockd, (unsigned char *)context->hidden.unknown.data1, (size*maxnum),0);
  memcpy(ptr,context->hidden.unknown.data1,(size*maxnum));
  return(bytes_received);  
} 
static int _net_seek(SDL_RWops *context, int offset, int whence)
{
  return(0);
}
static int _net_write(SDL_RWops *context, const void *ptr, int size, int num)
{  return 0; }
static int _net_close(SDL_RWops *context)
{
  if(context)
    SDL_FreeRW(context);
  return 0;
}

char menu();
void play_song();


int main(int argc, char *argv[])
{
  char menuChoice='t';
  ushort port;

  char testMsg[1500];

  struct hostent *host;

  if(argc<3)
  {
    cout<<"\nusage: "<<argv[0]<<" <server> <port>\n";
    return (1);
  }
  host=gethostbyname(argv[1]);
  if(host==NULL)
  {
    cout<<argv[0]<<": unknown host '" <<argv[1]<<"'\n";
    return(1);
  }
  port = (ushort) strtoul(argv[2],NULL,0);
  if(!(port>0&&port<65536))
  {
    cout<<argv[0]<<": invalid port '" <<argv[2]<<"'\n";
    return(1);
  }
  servAddr.sin_family=host->h_addrtype;
  memcpy((char*)&servAddr.sin_addr.s_addr,host->h_addr_list[0],host->h_length);
  servAddr.sin_port=htons(port);

  sockd=socket(AF_INET, SOCK_STREAM, 0);
  if(sockd<0)
  {
    cout<<argv[0]<<": cannot open socket\n";
    return(1);
  }
  localAddr.sin_family=AF_INET;  //bind any port number
  localAddr.sin_addr.s_addr=htonl(INADDR_ANY);
  localAddr.sin_port=htons(0);
  
  rc=bind(sockd, (struct sockaddr*)&localAddr,sizeof(localAddr));
  if(rc<0)
  {
    cout<<argv[0]<<": cannot bind TCP port "<<port<<endl;
    return(1);
  }
  rc=connect(sockd, (struct sockaddr*) &servAddr, sizeof(servAddr));
  if(rc<0)
  {
    cout<<argv[0]<<": cannot connect\n";
    return(1);
  }
 
  while(menuChoice!='q')
  {
    menuChoice=menu();
    switch(menuChoice)
    {
      case '1':   //connect to server 
        rc=send(sockd,"KeV",3,0);
        if(recv(sockd, testMsg, MAX_RECV, 0)<0)
          cout<<argv[0]<<": Error receiving response from server\n";
        else
        {
          if(testMsg[0]=='K'&&testMsg[1]=='e'&&testMsg[2]=='V')
            cout<<"-= Server responding A-OK\n";
          else 
            cout<<"-= Server responce incorrect: "<<testMsg<<endl;
        }
        break;    
      case '2':   //connect to server 
        cout<<"you chose 2 \n";
        break;
      case '3':
        play_song();
        break;
      case 'q':
      case 'Q':
        cout<<"exiting...\n";
        break;
      default: 
        cout<<"Try a real choice, assmast\n";
        break;
    }//swtich/case
  }//while
}

void play_song()
{
  SDL_RWops* rw;
  int sent=0;
  unsigned char * buf_ptr = buffer;
  int i=BUFFER;

  send(sockd,"GoG",3,0);
  
  while(i>PACKET_SIZE)
  {
    recv(sockd,buf_ptr,PACKET_SIZE,0);
    i -= PACKET_SIZE;
    buf_ptr += PACKET_SIZE;
  }
  recv(sockd,buf_ptr,i,0);
  
  rw=SDL_RWFromNet(buffer);
  playmp3(rw);
//  SDL_FreeRW(&rw); done by freesample
}

SDL_RWops * SDL_RWFromNet(unsigned char *buf)
{
  register SDL_RWops* rw;
  rw=SDL_AllocRW();
  if(!rw)
  {
    cout<<"Error allocating RWop\n";
    return 0;
  }
  rw->hidden.unknown.data1=(unsigned char*)buf; 
  rw->read  = _net_read;
  rw->write = _net_write;
  rw->seek  = _net_seek;
  rw->close = _net_close;
  return rw;
} 

int playmp3(SDL_RWops *rw)
{
    Sound_AudioInfo sound_desired;
    Uint32 audio_buffersize = AUDIOBUF;
    Uint32 decode_buffersize =DECODEBUF;
    SDL_AudioSpec sdl_desired;
    SDL_AudioSpec sdl_actual;
    Sound_Sample *sample;
    int i;
    int delay;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if(SDL_Init(SDL_INIT_AUDIO)<0)
    { 
        cout<<"Error initializing sound, SDL error "<<SDL_GetError()<<endl;
        return(1);
    }
    atexit(SDL_Quit);
    if(!Sound_Init())
    {
        cout<<"Error initializing sound, SDL_sound error "<<Sound_GetError()<<endl;
        return(1);
    }
    signal(SIGINT, sigint_catcher);

    sound_desired.rate=44100;
    sound_desired.format=AUDIO_S16SYS;
    sound_desired.channels=2;
    /* this opens the MP3, we will change it to Sound_NewSample() to get from the network stream */

    sample=Sound_NewSample(rw,"mp3", &sound_desired,DECODEBUF);
    if(!sample)
    {
        cout<<"loading sound failed\n"<<Sound_GetError();
        return(1);
    }
    
    sdl_desired.freq=sample->actual.rate;
    sdl_desired.format= sample->actual.format;
    sdl_desired.channels= sample->actual.channels;
    sdl_desired.samples= AUDIOBUF;
    sdl_desired.callback=audio_callback;
    sdl_desired.userdata=sample;
    if(SDL_OpenAudio(&sdl_desired, NULL)<0)
    {
      cout<<"Couldn't open SDL audio device: "<<SDL_GetError()<<endl;
      Sound_Quit();
      SDL_Quit();
      return(42);
    }
    cout<<"Playing..."<<endl;
    done_flag=0;
    SDL_PauseAudio(0);
    while(!done_flag)
    {
      SDL_Delay(10);  // wait 50 milliseconds 
    }
    SDL_PauseAudio(2); 
//sleep 2 buffers worth of audio before closing
 //this allows the playback to finish
    delay=2*1000 * sdl_desired.samples / sdl_desired.freq;
    SDL_Delay(delay);
    if(sample->flags & SOUND_SAMPLEFLAG_ERROR)
      cout<<"Error decoding: "<<Sound_GetError()<<endl;

    SDL_CloseAudio();
    Sound_FreeSample(sample); //free the pointer..
    Sound_Quit();
    SDL_Quit();
    return(0);

} 


//  Sound_Decode(sample);
/* This function takes deacoded mp3 data and put it's in the stream until 
 * we have len data. read more data() gets the data and decodes it
 */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
  Sound_Sample *sample=(Sound_Sample*) userdata;
  int bytesWritten=0;
 
  while(bytesWritten < len)
  {
    Uint32 copysize;
    
    if(!read_more_data(sample)) /*read more if needed*/
    {
      /*not more to read, could be end of song, or stream buffer empty*/
      memset(stream+bytesWritten, '\0', len-bytesWritten);
      done_flag=1;
      return;
    }
    copysize=len - bytesWritten;
    if(copysize>decoded_bytes)
      copysize=decoded_bytes;

    if(copysize>0)
    {
      memcpy(stream+bytesWritten, decoded_ptr, copysize);
      bytesWritten += copysize;
      decoded_ptr+= copysize;
      decoded_bytes-=copysize;
    }
  }
}
//This updates decoded_bytes and decoded_ptr w/ more audio data
static int read_more_data(Sound_Sample *sample)
{
  if(done_flag)//probably a sigint; stop trying to read
    decoded_bytes=0;
  if(decoded_bytes>0)
    return(decoded_bytes); //dont need more yet
  //need more
  
  if(!(sample->flags &(SOUND_SAMPLEFLAG_ERROR | SOUND_SAMPLEFLAG_EOF)))
  {
    decoded_bytes=Sound_Decode(sample);
    decoded_ptr=(unsigned char *) sample->buffer;
    return(read_more_data(sample));
  }
  return(0); //end of file or error
}
void sigint_catcher(int signum)
{
  static Uint32 last_sigint=0;
  Uint32 ticks = SDL_GetTicks();

  assert(signum==SIGINT);
  if((last_sigint!=0)&&(ticks-last_sigint<500))
  {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    Sound_Quit();
    SDL_Quit();
    exit(1);
  }
  else
  {
    last_sigint=ticks;
    done_flag=1;
  }
}



char menu()
{
  char select;
  int selectGood=0;
  
  while(!selectGood)
  {
    cout<<"\n\nBisquikMixer Client\n\n";
    cout<<"1. Connect to server to test connection\n";
    cout<<"2. Get playlist\n";
    cout<<"3. Play random song\n";
    cout<<"q. Quit BisquiMixer\n";
    cout<<"Choose wisely: ";
    cin>>select;
    switch(select)
    {
      case '1':
      case '2':
      case '3':
      case 'q':
        selectGood=1;
        break;
      default:
        cout<<"\n'"<<select<<"' invalid, dumbass\n";
    }
  }
  return(select);
}
