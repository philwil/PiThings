
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

extern int g_quit_one;
extern int g_lsock;
extern struct list *g_space_list;

int run_test_send( struct iosock * in,int argc, char * argv[], char *buf, int blen)
{
  int csock;
  int csize;
  int rc;
  // send arg 2 3 and maybe 4  to local port and listen for reply
  if(argc <4 )
    {
      snprintf(buf, blen,"%s %s", argv[2], argv[3]);
    }
  else
    {
      snprintf(buf, blen,"%s %s %s", argv[2], argv[3], argv[4]);
    }	       
  csock = connect_socket(5432, NULL);
  printf ("sending [%s] to server %s res %d \n", buf, "localhost", csock);
  if(csock > 0)
    {
      //int len = sizeof(buf);
      //int dummy_hand(int fd, char *id, char *buf, int len)
      init_new_hand("some_id", "Dummy Handler",  dummy_handler);
      // run_new_gcmd
      add_socket(csock);
      in->fd = -1;
      csize = snprintf(buf, blen,"%s %s", argv[2], argv[3]);
      //TODO check buf csize
      // register_handler("some_id", dummy_handler);
      snprintf(buf, blen,"CMD %s %d\n\n", "some_id", csize);
      rc = write(csock, buf, strlen(buf)); 
      snprintf(buf, blen,"%s %s", argv[2], argv[3]);
      rc = write(csock, buf, strlen(buf)); 
      //snprintf(buf, sizeof(buf),"QUIT\n\n");
      //rc = write(csock, buf, strlen(buf)); 
      rc = 1;
      g_debug_term = 0;
      while(rc)
      {
      //  rc = read(csock, buf , sizeof(buf)-1);
      //  if(rc>0)
      //    {
      //      dummy_handler(csock, "some_id", buf , rc);
      //    }
	rc = 0;
      }
      //close(csock);
      //return 0;
      // quit after one _REP
      g_quit_one = 1;		 
      g_lsock = -1;
      
    }
  return 0;
}

int run_test_main(struct iosock * in, int argc, char * argv[], char *buf, int blen)
{
  int rc;
  char sbuf[4096];
  char *vals[64];
  char *spz;
  //struct space *sp0;
  struct space *sp1;
  struct space *sp2;

  rc = parse_stuff(' ', 64, (char **)vals, "this is a bunch/of/stuff to parse",0);
  printf (" rc = %d\n", rc );
  show_stuff(rc, vals);
  
  sp1 = add_space(&g_space_list, "ADD uav1/motor1/speed");
  show_spaces(in, &g_space_list, "All Spaces 1 ",0);
  
  sp1 = add_space(&g_space_list, "ADD uav3/motor2/speed");
  sp1->onset = speed_onset;
  sp1->onget = speed_onget;
  rc  = set_space(&g_space_list, "SET uav3/motor2/speed 3500");
  spz =  get_space(&g_space_list,"GET uav3/motor2/speed");
  printf(" >> %s value [%s]\n","uav3/motor2/speed", spz?spz:"no value");
  return 0;
	   
  sp1 = add_space(&g_space_list, "ADD uav1/motor1/size");
  show_spaces(in, &g_space_list, "All Spaces 1 ",0);
  sp1 = add_space(&g_space_list, "ADD uav1/motor2/speed");
  show_spaces(in,&g_space_list, "All Spaces 2 ",0);
  sp1 = add_space(&g_space_list, "ADD uav2/motor2/speed");
  show_spaces(in,&g_space_list, "All Spaces 3 ",0);
  sp1 = add_space(&g_space_list, "uav3/motor2/speed");
	   
  sp1 = find_space_new(&g_space_list, "uav3/motor2");
  sp1->onset = motor_onset;
  sp1->onget = motor_onget;
  
  show_spaces_new(in, &g_space_list, sbuf, 4096, sbuf);
  sp1 = find_space_new(&g_space_list, "uav1/motor3");
  printf(" found %s \n", sp1?sp1->name:"no uav1/motor3");
  sp1 = find_space_new(&g_space_list, "uav1/motor1");
  printf(" found %s \n", sp1?sp1->name:"no uav1/motor1");
  sp2  = copy_space_new(sp1, "new_motor1");
  rc  = set_space(&g_space_list, "SET uav3/motor2/speed 3500");
  spz =  get_space(&g_space_list,"GET uav3/motor2/speed");
  printf(" >> %s value [%s] rc %d \n","uav3/motor2/speed"
	 , spz?spz:"no value"
	 , rc);
	   
  show_spaces_new(in,&sp2->child, sbuf, 4096, sbuf);
	   
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

