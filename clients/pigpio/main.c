
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

#include <sys/stat.h>
#include <fcntl.h>


#include "../inc/pithings.h"

//int num_socks=0;
int g_num_socks=0;
int g_debug = 0;

int g_lsock = 0;
int g_quit_one = 0;
struct iosock g_iosock[NUM_SOCKS];
struct cmds g_cmds[NUM_CMDS];
struct cmds h_cmds[NUM_CMDS];
struct hands g_hands[NUM_HAND];
#define NUM_IDX 1024
struct space *space=NULL;
struct space *g_space=NULL;
int g_space_idx = 1;
struct space *g_spaces[NUM_IDX];
struct iobuf *g_iob_store = NULL;
int g_debug_term = 0;
int g_count = 0;

//struct node *g_node_list = NULL;
struct list *g_node_list = NULL;
struct list *g_conn_list = NULL;
struct list *g_space_list = NULL;

int g_node_debug= 0;
int g_conn_debug= 0;
int g_list_debug = 0;
int g_space_debug = 0;
int g_group_debug = 0;
int g_port_no = 5432;
char *g_myname = NULL;
char *g_myaddr = NULL;

/*
  NOTE on port num myname (command line also specifies myname )
 g_port_no

  NOTES on CONN
  CONN addr port name
  sets up a connection under name
  on set up we'll send a NODE commans to the CONN (NODE myaddr port myname 
  add <space> will also set up  and send an ADD to the conn as name/<space>
  all cons 

  sh ./runme (port 5432)
  sh ./runme port 4321 pigpios
     CONN pigpios1 127.0.0.1 5432 foo
   Subsequent ADD commands will add a duplicate tot eh conn node 
   if the names match
   ADD pigpios1/bank1/pgpio1
   will add the name to both systems

   SET pigpios1/bank1/pgopi1 on
  on either system will send the same commands to the remote node 
  GET locally will just get the local value
  GET from the remote will fetch the remote value 

When talking to a remote we should set up a context and use "send_command" to 
encapsulate the command and the response
CMD <clen> <id>\n\n<actual command>
The REMOTE will respond REP <rlen> <id><actual response>
The handler registered with <id> will get the response.
.. more later

So how does a remote set up.

It opens a port and listens.. We go that
It will also register commands it want to listen to and repsonses

So we get uri xyz and we do abc


Simple case for starters
set a gpio output
  lot of stuff here http://abyz.co.uk/rpi/pigpio/
Set up and listen for connections
possible command

GPIO name type dir(in/out/pwm/pulse/servo) pin
POST name value
GET name

We'll help ourselves by using a config file "./config.txt"
Lets see if the standard parser can cope
 
O_RDONLY
*/

int test_file(char *name)
{
  int fd;
  fd = open(name, O_RDONLY);
  if (fd >=0)
    {
      add_socket(fd);
    }
  return fd;
}

int def_config(void)
{
   char buf[2048];
   struct space *sp1;
   struct space *sp2;
   struct list *lp1;
   struct list *lp2;
  //add_space_in(&g_space_list, "ADD uav1", NULL);
  //add_space_in(&g_space_list, "ADD uav2", NULL);
  //add_space_in(&g_space_list, "ADD uav3", NULL);
  add_space_in(&g_space_list, "ADD uav1/motor1", NULL);
  add_space_in(&g_space_list, "ADD uav1/motor2", NULL);
  sp1 = find_space_new(&g_space_list, "ADD uav1");
  printf ("UAV1 spacep %p", sp1);
  if(sp1)
    printf(" g_space_list data %p match %d uav1->child %p\n"
	   , g_space_list->data
	   , (g_space_list->data == sp1)
	   , sp1->child
	   );
  if(sp1->child)
    {
      lp1 = sp1->child;
      sp2 = lp1->data;
      sp1 = lp1->next->data;
      if (sp2)
	{
	  lp2 = sp2->child;
	  printf("sp2 %p idx %d name [%s] c %p lp2 %p\n"
		 , sp2 , sp2->idx, sp2->name, sp2->child, lp2 );
	}
      if (sp1)
	{
	  printf("sp1 %p idx %d name [%s] c %p\n"
		 , sp1 , sp1->idx, sp1->name, sp1->child);
	}
    }
  //add_space_in(&g_space_list, "ADD uav1/motor1/speed", NULL);
  printf ("spaces from root\n");
  show_spaces_new(NULL,&g_space_list, buf, 2048, buf);
  
  //printf ("spaces from sp2\n");
  //show_spaces_new(NULL,&g_space_list[1], buf, 2048, buf);
  //add_space
  test_groups();
  printf ("spaces from groups\n");
  show_spaces_new(NULL,&g_space_list, buf, 2048, buf);
  
  return 0;
}



int main (int argc, char *argv[])
{

   int rc = 1;
   //struct space *sp3;
   //char buf[2048];
   //char *sp;
   //char *vals[64];
   struct iosock ins;
   struct iosock *in = &ins;

   g_myaddr = strdup("127.0.0.1");
   init_g_spaces();
   init_iosocks();
   init_iosock(in);
   init_hands(NUM_HAND);
   init_cmds(g_cmds, NUM_CMDS);
   init_cmds(h_cmds, NUM_CMDS);
   set_up_new_cmds();
   g_list_debug =1;
   g_debug = 0;
   g_debug_term = 0;
   rc = test_file("config.txt");
   if(rc< 0)
     {
       def_config();
     }
   g_debug = 0;

   in->fd = 1;

#if 1
   //return 0;
   //test_nodes();
   //test_node_list();
   //return 0;
   //help_new_gcmds(NULL, NULL, NULL);
  

   //test_find_parents();
   //return 0;
   
   if(argc > 1)
     {
       if (strcmp(argv[1], "port") == 0)
	 {
	   if(argc > 3)
	     {
	       g_port_no = atoi(argv[2]);
	       g_myname = strdup(argv[3]);
	       printf(" Set up port to %d myname [%s] \n" 
		      , g_port_no
		      , g_myname
		      );
	     }
	   else
	     {
	       printf(" \"port\" needs number and name \n");
	       return 0;
	     }
	   //test_iob_out();
	   //return 0;
	 }
       else if (strcmp(argv[1], "test_iob_out") == 0)
	 {
	   test_iob_out();
	   return 0;
	 }
       else if (strcmp(argv[1], "test_iob") == 0)
	 {
	      test_iob();
	      return 0;
	 }

       else if (strcmp(argv[1], "send") == 0)
	 {

	   //run_test_send( in, argc, argv, buf, sizeof(buf));
	 }
     
       else if (strcmp(argv[1], "test") == 0)
	 {
	   //run_test_main(in, argc, argv, buf, sizeof(buf));
	 }
     }
#endif

#if 1
   if(g_lsock == 0)
     {
       accept_socket(STDIN_FILENO);
       g_lsock = listen_socket(g_port_no);
     }
   rc = 1;
   while(rc>0 && g_count < 10)
   {
       rc = poll_sock(g_lsock);
       //g_count++;
   }
#endif
   return 0;
}
