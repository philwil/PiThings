/* from
http://abyz.me.uk/rpi/pigpio/ex_motor_shield.html
cc -o pimotor pimotor.c -lpigpio -lpthread -lrt
*/

/*
 for now forget the motor stuff
 we aim to build up a repository of things
 a thing has a name, 
 belongs to a class 
 has a type
 has an id
 has some text value int value or float value
 things can be classes, lists or types
 ADD foo data float 2.3456
 ADD foo1 data string  "this is foo 1"
 SHOW foo
 SHOW data
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
//#include <pigpio.h>

#define NUM_THINGS 1024
#define CLASS_VAR 1
#define CLASS_CLASS 2
#define CLASS_TYPE 3

struct thing 
{
  char name[64];
  char desc[64];
  int idx;
  int id;
  int type;
  int class;
  int last_idx;
  int next_idx;

  char str[64];
  int ival;
  double fval;
};


struct thing things[NUM_THINGS];
struct thing t_types[NUM_THINGS];

// main
int init_things(void)
{
  int i;
  struct thing *item;
  item = &things[0];

  for (i=0; i< NUM_THINGS; i++)
    {
      item->idx = i;
      item->name[0] = 0;
      item++;
    }  
  item = &t_types[0];

  for (i=0; i< NUM_THINGS; i++)
    {
      item->idx = i;
      item->name[0] = 0;
      item++;
    }  
  return 0;
}
//     name  class   type  value
// ADD foo   data    float 2.3456

struct thing *new_thing(struct thing *base, char *name, int class, int type, int y)
{
  int i;
  struct thing *item;
  item = &things[0];
  if(base)
    item = base;

  for (i=0; i< NUM_THINGS; i++)
    {
      if(item->name[0] == 0)
	{
	  strncpy(item->name,name,64);
	  item->class = class;
	  item->type = type;
	  break;
	}
      item++;
    }
  if(i ==  NUM_THINGS)
    item = NULL;
  printf("new_thing name[%s] item %p\n"
	 , name
	 , item
	 );

  return item;

}

struct thing *get_thing(struct thing *base, char *name, int class, int type, int y)
{

  int i;
  struct thing *item;
  item = &things[0];

  if(base)
    item = base;
  printf("get_thing 1 name[%s] class %d base%p\n", name, class, base);


  for (i=0; i< NUM_THINGS; i++)
    {
      if((item->class == class ) && (strcmp(item->name,name) == 0))
	{
	  break;
	}
      item++;
    }  
  printf("get_thing 2 name[%s] class %d base%p\n", name, class, base);

  if(i ==  NUM_THINGS)
    item = new_thing(base, name, class, type, y);
  printf("get_thing name[%s] class %d item %p\n"
	 , name
	 , class
	 , item
	 );

  return item;
}


// ADD foo data float 2.3456
// add_thing("ADD foo data float 2.3456")

int add_thing(struct thing *base, char *stuff)
{
  int rc;
  char cmd[64];
  char v1[64];
  char v2[64];
  char v3[64];
  char v4[64];
  char v5[64];
  struct thing *item_1;
  struct thing *item_2;
  struct thing *item_3;

  rc = sscanf(stuff, "%s %s %s %s %s %s"
	      , cmd
	      , v1
	      , v2
	      , v3
	      , v4
	      , v5
	      );
  printf(" cmd = [%s] v1=[%s] rc = %d\n", cmd, v1, rc );
  item_1 = get_thing(base, v1, CLASS_VAR ,0,0);
  item_2 = get_thing(base, v2, CLASS_CLASS,0,0);
  item_3 = get_thing(&t_types[0],v3, CLASS_TYPE,0,0);
  item_1->class = item_2->idx;
  item_1->type = item_3->idx;

  return 0;
}


int show_things(void)
{
  int i;
  struct thing *item;
  item = &things[0];

  for (i=0; i< NUM_THINGS; i++)
    {
      if(item->name[0] != 0)
	{
	  printf("idx %d item name [%s] class %d type %d \n"
		 , item->idx
		 , item->name
		 , item->class
		 , item->type
		 );
	}
      item++;
    }  
  return 0;
}


/*
   This code may be used to drive the Adafruit (or clones) Motor Shield.

   The code as written only supports DC motors.

   http://shieldlist.org/adafruit/motor

   The shield pinouts are

   D12 MOTORLATCH
   D11 PMW motor 1
   D10 Servo 1
   D9  Servo 2
   D8  MOTORDATA

   D7  MOTORENABLE
   D6  PWM motor 4
   D5  PWM motor 3
   D4  MOTORCLK
   D3  PWM motor 2

   The motor states (forward, backward, brake, release) are encoded using the
   MOTOR_ latch pins.  This saves four gpios.
*/

/* psw added stdin  and network interfaces
   commands are:
           M n +/-speed[, M n +/-speed]...
                Drive motors +0 = brake -0 = release
           S n +/- pos speed 
              Servo Drive +/- position or aps position plus optional speed 
           D n on or off
*/

typedef unsigned char uint8_t;

#define BIT(bit) (1 << (bit))

/* assign gpios to drive the shield pins */

/*      Shield      Pi */

#define MOTORLATCH  14
#define MOTORCLK    24
#define MOTORENABLE 25
#define MOTORDATA   15

#define MOTOR_3_PWM  7
#define MOTOR_4_PWM  8

/*
   The only other connection needed between the Pi and the shield
   is ground to ground. I used Pi P1-6 to shield gnd (next to D13).
*/

/* assignment of motor states to latch */

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD  1
#define BACKWARD 2
#define BRAKE    3
#define RELEASE  4

static uint8_t latch_st;

#define NUM_SOCKS 16
#define NUM_CMDS 16
#define INSIZE 1024
#define OUTSIZE 1024

int num_socks=0;

struct insock
{
  int fd;
  char inbuf[INSIZE];
  int inptr;
  int inlen;
  int insize;
  char outbuf[OUTSIZE];
  int outptr;
  int outlen;
  int outsize;
};

struct cmds
{
  char *key;
  char *cmd;
  int (*handler)(void *key, int n, char *cmd, int speed, int time);
};
  



struct insock insock[NUM_SOCKS];
struct cmds cmds[NUM_CMDS];

int init_cmds(void)
{
  int i;
  for (i = 0; i< NUM_CMDS; i++)
    {
      cmds[i].key = NULL;
      cmds[i].handler = NULL;
    }
  return i;
}
		 
int init_cmd(char *key, int (*hand)(void *key, int n, char *data, int speed, int time))
{
  int i;
  for (i = 0; i< NUM_CMDS; i++)
  {
    if(cmds[i].key == NULL)
      {
	cmds[i].key =  key;
	cmds[i].handler = hand;
	break;
      }
  }
  if(i == NUM_CMDS) i = -1;
  return i;
}


int run_cmd (char *key, int n, char *data, int speed, int time)
{
  int rc=-1;
  int i;
  for (i = 0; i< NUM_CMDS; i++)
    {
      if(cmds[i].key && (strcmp(cmds[i].key, key) == 0))
	{
	  rc = cmds[i].handler(&cmds[i], n, data, speed, time);
	  break;
	}
    }
  return rc;
}


int init_insocks(void)
{
  int i;
  for (i = 0; i< NUM_SOCKS; i++)
  {
      insock[i].fd = -1;
      insock[i].inptr = 0;
      insock[i].inlen = 0;
      insock[i].insize = INSIZE;
      insock[i].outsize = OUTSIZE;
      insock[i].outptr = 0;
      insock[i].outlen = 0;
  }
  return i;
}

int listen_socket(int portno)
{
     int sockfd, newsockfd, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int data;

     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons( portno );
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
	 printf("ERROR on binding");
	 close(sockfd);
	 return -1;
     }
     listen(sockfd,5);
     printf( "listen sock #%d\n", sockfd );

     return sockfd;
} 


int accept_socket(int sockfd)
{
     int newsock, clilen;
     struct sockaddr_in cli_addr;
     int i;
     if (sockfd != STDIN_FILENO)
     {
	 clilen = sizeof(cli_addr);
	 if ( ( newsock = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
	   {
	     printf("ERROR on accept");
	     return -1;
	   }
	 printf( "opened new communication with client \n" );
     }
     else
     {
	 newsock = sockfd;
     }
     for (i = 0; i< NUM_SOCKS; i++)
     {
	 if (insock[i].fd < 0)
	 {
	     insock[i].fd = newsock;
	     insock[i].inptr = 0;
	     insock[i].inlen = 0;
	     insock[i].outptr = 0;
	     insock[i].outlen = 0;
	     num_socks++;
	     break;
	 }
     }
     return newsock;
}

struct insock *find_fd(int fsock)
{
    int i;
    struct insock *in = NULL;
    for (i = 0; i< NUM_SOCKS; i++)
    {
        if (insock[i].fd == fsock)
        {
	    in = &insock[i];
	    break;

        }
    }
    return in;
}

int close_fds(int fsock)
{
    int rc = -1;
    struct insock *in = find_fd(fsock);
    if (in)
    {
	rc = 0;
	in->fd = -1;
	in->inptr = 0;
	in->inlen = 0;
	in->outptr = 0;
	in->outlen = 0;
	num_socks--;
    }
    return rc;
}

/*
  commands are 
  fwd [speed] [time]
  back [speed] [time]
  stop
  right [speed] [time]
  left [speed] [time]
*/

int handle_input(struct insock *in)
{
    int rc;
    int len;
    int n;
    char cmd[64];
    char *sp;
    int speed=0;
    int time=0;

    len = read(in->fd,&in->inbuf[in->inptr],in->insize-in->inptr);

    if(len > 0)
      {
	in->inlen += len;
	in->inbuf[in->inlen] = 0;
	sp = &in->inbuf[in->inptr];
	n = sscanf(sp,"%s %d %d", cmd, &speed, &time);
	rc = snprintf(&in->outbuf[in->outlen], in->outsize-in->outptr
		      ," message received [%s] ->"
		      " n %d cmd [%s] speed %d time %d\n"
		      , &in->inbuf[in->inptr]
		      , n, cmd , speed, time);
	in->outlen =+ rc;
	run_cmd (cmd, n, sp, speed, time);

	printf(" rc %d n %d cmd [%s] speed %d time %d\n"
	       , rc, n, cmd , speed, time);
	in->outlen =+ rc;
	in->inptr= 0;
	in->inlen= 0;
      }
    return len;
}

int handle_output(struct insock *in)
{
    int rc;
    rc = write(in->fd,&in->outbuf[in->outptr],in->outlen-in->outptr);
    if(rc >0)
    {
	in->outptr += rc;
	if (in->outptr == in->outlen)
	{
	    in->outptr = 0;
	    in->outlen = 0;
	}
    }
    return rc;
}


// read stdin, listen_sock and all accept_socks
int poll_sock(int lsock)
{
    struct pollfd fds[NUM_SOCKS+2];
    int idx = 0;
    int i;
    int timeout = 1000;
    int ret;
    int n;
    char str[1024];
    struct insock *in = NULL;
    int rc = 1;    
    //fds[idx].fd = STDIN_FILENO;
    //fds[idx].events = POLLIN;
    //fds[idx].revents = 0;
    //idx++;
    
    fds[idx].fd = lsock;
    fds[idx].events = POLLIN;
    fds[idx].revents = 0;
    idx++;
    
    for (i = 0; i< NUM_SOCKS; i++)
      {
	if (insock[i].fd >= 0)
	  {
	    fds[idx].fd = insock[i].fd;
	    fds[idx].events = POLLIN;
	    fds[idx].revents = 0;
	    if(insock[i].outptr != insock[i].outlen)
	      {
		fds[idx].events |= POLLOUT;
	      }
	    idx++;
	  }
      }

    printf("poll start idx %d\n", idx);
    ret =  poll(fds, idx, timeout);
    printf("poll done ret = %d idx %d\n", ret, idx);

    if(ret > 0) 
    {
	printf("poll ret = %d idx %d\n", ret, idx);
	
	for( i = 0; i < idx; i++) 
	{
	    printf(" idx %d fd %d revents 0x%08x\n",i, fds[i].fd, fds[i].revents);  
	    if (fds[i].revents & POLLOUT) 
	    {
		in = find_fd(fds[i].fd);
		if(in)
		{
		    n = handle_output(in);
		    if (n <= 0) 
		    {
			printf("error writing (n=%d), closing fd %d \n"
			       ,n,fds[i].fd);
			close(in->fd);
			close_fds(in->fd);
			fds[i].revents = 0;
		    }
		}
	    }
	    if (fds[i].revents & POLLIN) 
	    {
		if(fds[i].fd == lsock)
		{ 
		    ret = accept_socket(lsock);
		    printf("accept ret = %d \n", ret);
		    
		}
		else
		{
		    in = find_fd(fds[i].fd);
		    if(in)
		    {
		        n = handle_input(in);

			if (n <= 0) 
			{
			    printf("error reading (n=%d), closing fd %d \n",n,fds[i].fd);
			    close(fds[i].fd);
			    close_fds(fds[i].fd);
			}
			else 
			{
			    printf("message len %d\n", n);
			    
			}
		    }
		}
	    }
	}
    }
    return rc;
}


#if 0
void latch_tx(uint8_t latch_state)
{
   unsigned char i;

   gpioWrite(MOTORLATCH, PI_LOW);

   gpioWrite(MOTORDATA, PI_LOW);

   for (i=0; i<8; i++)
   {
      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_LOW);

      if (latch_state & BIT(7-i)) gpioWrite(MOTORDATA, PI_HIGH);
      else                        gpioWrite(MOTORDATA, PI_LOW);

      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_HIGH);
   }

   gpioWrite(MOTORLATCH, PI_HIGH);
}

void init(uint8_t latch_state)
{
  //latch_state = 0;

   latch_tx(latch_state);

   gpioWrite(MOTORENABLE, PI_LOW);
}

uint8_t DCMotorInit(uint8_t num, uint8_t latch_state)
{

   switch (num)
   {
      case 1: latch_state &= ~BIT(MOTOR1_A) & ~BIT(MOTOR1_B); break;
      case 2: latch_state &= ~BIT(MOTOR2_A) & ~BIT(MOTOR2_B); break;
      case 3: latch_state &= ~BIT(MOTOR3_A) & ~BIT(MOTOR3_B); break;
      case 4: latch_state &= ~BIT(MOTOR4_A) & ~BIT(MOTOR4_B); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

uint8_t DCMotorRun(uint8_t motornum, uint8_t cmd, uint8_t latch_state)
{
   uint8_t a, b;

   switch (motornum)
   {
      case 1: a = MOTOR1_A; b = MOTOR1_B; break;
      case 2: a = MOTOR2_A; b = MOTOR2_B; break;
      case 3: a = MOTOR3_A; b = MOTOR3_B; break;
      case 4: a = MOTOR4_A; b = MOTOR4_B; break;
      default: return latch_state;
   }
 
   switch (cmd)
   {
      case FORWARD:  latch_state |=  BIT(a); latch_state &= ~BIT(b); break;
      case BACKWARD: latch_state &= ~BIT(a); latch_state |=  BIT(b); break;
      case RELEASE:  latch_state &= ~BIT(a); latch_state &= ~BIT(b); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

#endif

int fwd_cmd(void *key, int n, char *data, int speed, int time)
{
    struct cmds *cmd = (struct cmds*)key;

    printf("running [%s] n %d data [%s]  speed %d time %d\n",
	   cmd->key, n, data, speed, time);
    return 0;
}

int count = 0;
int main (int argc, char *argv[])
{
   int i;
   int lsock;
   int rc = 1;

   init_things();
   add_thing(NULL, "ADD foo data float 2.3456");
   add_thing(NULL, "ADD foo1 data int 234");
   add_thing(NULL, "ADD foo3 data str \"val 234\"");
   show_things();

   init_cmds();
   init_cmd("FWD", fwd_cmd);
   run_cmd ("FWD", 2, "Some data", 100, 50);

   init_insocks();
   accept_socket(STDIN_FILENO);
   lsock = listen_socket(5432);
   while(rc>0 && count < 10)
   {
       rc = poll_sock(lsock);
       //count++;
   }
#if 0
   if(0)
     {
       if (gpioInitialise()<0) return 1;

       gpioSetMode(MOTORLATCH,  PI_OUTPUT);
       gpioSetMode(MOTORENABLE, PI_OUTPUT);
       gpioSetMode(MOTORDATA,   PI_OUTPUT);
       gpioSetMode(MOTORCLK,    PI_OUTPUT);
       
       gpioSetMode(MOTOR_3_PWM, PI_OUTPUT);
       gpioSetMode(MOTOR_4_PWM, PI_OUTPUT);
       
       gpioPWM(MOTOR_3_PWM, 0);
       gpioPWM(MOTOR_4_PWM, 0);
       
       init(latch_st);
       
       for (i=60; i<160; i+=20)
	 {
	   gpioPWM(MOTOR_3_PWM, i);
	   gpioPWM(MOTOR_4_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, FORWARD, latch_st);
	   latch_st = DCMotorRun(4, BACKWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	   
	   gpioPWM(MOTOR_4_PWM, i);
	   gpioPWM(MOTOR_3_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, BACKWARD, latch_st);
	   latch_st = DCMotorRun(4, FORWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	 }
       
       gpioPWM(MOTOR_4_PWM, 0);
       gpioPWM(MOTOR_3_PWM, 0);
       
       latch_st = DCMotorRun(3, RELEASE, latch_st);
       latch_st = DCMotorRun(4, RELEASE, latch_st);
       
       gpioTerminate();
     }
#endif

}
