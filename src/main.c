
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
int g_debug_term = 1;
int g_count = 0;

struct node *g_node_store = NULL;
struct list *g_node_list = NULL;
int g_node_debug;

int init_g_spaces(void)
{
  int i;
  for (i=0 ; i< NUM_IDX; i++)
    g_spaces[i]=NULL;
  return 0;

}
int init_iosocks(void)
{
  int i;
  struct iosock *in;
  for (i = 0; i< NUM_SOCKS; i++)
  {

      in = &g_iosock[i];
      init_iosock(in);
  }
  return i;
}

int main (int argc, char *argv[])
{
  //int i;
   int csock;
   int csize;
   int rc = 1;
   //int depth=0;
   //struct space * sp1 = new_space("Space1", struct space *parent, struct space** root, char *node)
   struct space *sp1;
   struct space *sp2;
   //struct space *sp3;
   char buf[2048];
   char *sp;
   char *vals[64];
   struct iosock ins;
   struct iosock *in = &ins;

   //test_nodes();
   test_node_list();
   //eturn 0;
   
   init_hands(NUM_HAND);
   init_cmds(g_cmds, NUM_CMDS);
   init_cmds(h_cmds, NUM_CMDS);
   set_up_new_cmds();
   struct space *help_new_gcmds(struct space **base, char *name, struct iosock *in);
   help_new_gcmds(NULL, NULL, NULL);
  
   
   init_g_spaces();
   init_iosocks();
   init_iosock(in);

   in->fd = 1;
   test_find_parents();
   //return 0;
   
   if(argc > 1)
     {
       if (strcmp(argv[1], "test_iob_out") == 0)
	 {
	   test_iob_out();
	   return 0;
	 }
       if (strcmp(argv[1], "test_iob") == 0)
	 {
	      test_iob();
	      return 0;
	 }

       if (strcmp(argv[1], "send") == 0)
	 {
	   // send arg 2 3 and maybe 4  to local port and listen for reply
	   if(argc <4 )
	     {
	       snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	     }
	   else
	     {
	       snprintf(buf, sizeof(buf),"%s %s %s", argv[2], argv[3], argv[4]);
	     }	       
	   csock = connect_socket(5432, NULL);
	   printf ("sending [%s] to server %s res %d \n", buf, "localhost", csock);
	   if(csock> 0)
	     {
	       //int dummy_hand(int fd, char *id, char *buf, int len)
	       init_new_hand("some_id", "Dummy Handler",  dummy_handler);
	       // run_new_gcmd
	       add_socket(csock);
	       in->fd = -1;
	       csize = snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	       //TODO check buf csize
	       // register_handler("some_id", dummy_handler);
	       snprintf(buf, sizeof(buf),"CMD %s %d\n\n", "some_id", csize);
	       rc = write(csock, buf, strlen(buf)); 
	       snprintf(buf, sizeof(buf),"%s %s", argv[2], argv[3]);
	       rc = write(csock, buf, strlen(buf)); 
	       //snprintf(buf, sizeof(buf),"QUIT\n\n");
	       //rc = write(csock, buf, strlen(buf)); 
	       rc = 1;
	       g_debug_term = 0;
	       //while(rc)
	       //{
	       //  rc = read(csock, buf , sizeof(buf)-1);
	       //  if(rc>0)
	       //    {
	       //      dummy_handler(csock, "some_id", buf , rc);
	       //    }
	       //}
	       //close(csock);
	       //return 0;
	       // quit after one _REP
	       g_quit_one = 1;		 
	       g_lsock = -1;

	     }

	 }
     
       else if (strcmp(argv[1], "test") == 0)
	 {

	   char sbuf[4096];

	   rc = parse_stuff(' ', 64, (char **)vals, "this is a bunch/of/stuff to parse",0);
	   printf (" rc = %d\n", rc );
	   show_stuff(rc, vals);
	   
	   sp1 = add_space(&g_space, "ADD uav1/motor1/speed");
	   show_spaces(in, g_space, "All Spaces 1 ",0);
	   
	   sp1 = add_space(&g_space, "ADD uav3/motor2/speed");
	   sp1->onset = speed_onset;
	   sp1->onget = speed_onget;
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
	   sp =  get_space(g_space,"GET uav3/motor2/speed");
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   return 0;
	   
	   sp1 = add_space(&g_space, "ADD uav1/motor1/size");
	   show_spaces(in,g_space, "All Spaces 1 ",0);
	   sp1 = add_space(&g_space, "ADD uav1/motor2/speed");
	   show_spaces(in,g_space, "All Spaces 2 ",0);
	   sp1 = add_space(&g_space, "ADD uav2/motor2/speed");
	   show_spaces(in,g_space, "All Spaces 3 ",0);
	   sp1 = add_space(&g_space, "uav3/motor2/speed");
	   
	   sp1 = find_space_new(g_space, "uav3/motor2");
	   sp1->onset = motor_onset;
	   sp1->onget = motor_onget;
	   
	   show_spaces_new(in, &g_space, sbuf, 4096, sbuf);
	   sp1 = find_space_new(g_space, "uav1/motor3");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor3");
	   sp1 = find_space_new(g_space, "uav1/motor1");
	   printf(" found %s \n", sp1?sp1->name:"no uav1/motor1");
	   sp2  = copy_space_new(sp1, "new_motor1");
	   rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
	   sp =  get_space(g_space,"GET uav3/motor2/speed");
	   printf(" >> %s value [%s]\n","uav3/motor2/speed", sp?sp:"no value");
	   
	   show_spaces_new(in,&sp2, sbuf, 4096, sbuf);
	   
	   //   return 0;
#if 0
	   new_space("Space1", NULL, NULL, "127.0.0.1");
	   sp1 =  g_space;
	   show_space(in, sp1, 0, NULL , 0);
	   sp1 = new_space("Space2", sp1, NULL, "127.0.0.1");
	   show_space(in, g_space, 0, NULL , 0);
	   show_space(sp1, 0, NULL , 0);
	   sp1 = new_space("Space3", sp1, NULL, "127.0.0.1");
	   show_space(in, g_space, 0, NULL , 0);
	   show_space(in,sp1, 0, NULL , 0);
	   
	   //   sp1 = g_space;
	   show_spaces(in, g_space, "Global Spaces 1",0);
	   //           attr        space  class type  value 
	   
	   sp1 = g_space;
	   //struct space *new_space_attr_float(char *name , struct space *parent,1.234)
	   
	   sp2 = new_space_attr_float("foo_float", sp1, 1.2345);
	   sp2 = new_space_attr_int("foo_int", sp1, 2345);
	   sp2 = new_space_attr_str("foo_str", sp1, "x2345");
	   
	   show_spaces(in, sp1->attr, "Sp1 attr",0);
	   
#endif
	 }
     }
   
   if(g_lsock == 0)
     {
       accept_socket(STDIN_FILENO);
       g_lsock = listen_socket(5432);
     }
   rc = 1;
   while(rc>0 && g_count < 10)
   {
       rc = poll_sock(g_lsock);
       //g_count++;
   }
   return 0;
}
