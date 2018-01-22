/* 
  A group is a list of things
  Set the value in one member the same value is set in all the others
  Get the value of one group member you get the lot
  any thing can have a group list 
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

#include "../inc/pithings.h"

extern int g_group_debug;
extern struct list *g_space_list;

//add group member
int add_group_member(char *master, char *member)
{
  struct space *gmaster;
  struct space *gmember;
  struct list * item;
  int rc = -1;
  gmaster = get_space_in(&g_space_list, master, NULL);
  gmember = get_space_in(&g_space_list, member, NULL);

  if(gmaster && gmember)
    {
      rc = 0;
      item = new_list((void*)gmember);
      push_list(&gmaster->group, item);
    }
  return rc;
}

int rem_group_member(char *master, char *member)
{
  struct space *gmaster;
  struct space *gmember;
  struct list * item;
  int rc = -1;
  gmaster = get_space_in(&g_space_list, master, NULL);
  gmember = get_space_in(&g_space_list, member, NULL);

  if(gmaster && gmember)
    {
      rc = 0;
      item = find_list_item(gmaster->group, (void*)gmember);
      if (item)
	pop_list(&gmaster->group, item);
    }
  return rc;

}


int print_group_item(struct list *item)
{
  struct space *space = NULL;
  space = (struct space*)item->data;

  printf("@%p next@%p prev@%p  idx %d name [%s]\n"
	 , item
	 , item->next
	 , item->prev
	 , space->idx   // where we read from
	 , space->name   // where we write to
	 );
  return 0;
}

int print_group_list(char *master)
{
  int rc = 0;
  struct space *gmaster;

  struct list *item = NULL;
  struct list *sitem = NULL;
  printf(" %s [%s]\n"
	 , __FUNCTION__
	 , master
	 );
  gmaster = get_space_in(&g_space_list, master, NULL);
  item = gmaster->group;
  sitem = item;//in->iobuf;

  while(item)
    {
      rc++;
      print_group_item(item);
      item = item->next;
      if(item == sitem) item = NULL;
    }
  return rc;
}

int set_group_value(char *master, char *value, char *name)
{
  int rc = 0;
  struct space *gmaster;
  struct space *gitem;

  struct list *item = NULL;
  struct list *sitem = NULL;
  if(g_group_debug)
    printf(" %s [%s]\n"
	   , __FUNCTION__
	   , master
	   );
  gmaster = get_space_in(&g_space_list, master, NULL);
  item = gmaster->group;
  sitem = item;//in->iobuf;

  while(item)
    {
      rc++;
      gitem = (struct space *) item->data;
      set_space_value(gitem, value, name);

      //print_group_item(item);
      item = item->next;
      if(item == sitem) item = NULL;
    }
  return rc;
}


//
int test_groups(void)
{
  int rc=0;
  struct space * sp1;
  //struct list *item;
  printf(" test_groups\n");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio1");
  //return 0;
  //sp1 = add_space(&g_space, "ADD pine1/gpios/gpio1");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio2");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio3");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio4");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio5");
  sp1 = add_space(&g_space_list, "ADD pine1/gpios/gpio6");
  sp1 = add_space(&g_space_list, "ADD red_leds");
  sp1 = add_space(&g_space_list, "ADD green_leds");
  sp1 = add_space(&g_space_list, "ADD blue_leds");
  add_group_member("red_leds", "pine1/gpios/gpio1");
  add_group_member("red_leds", "pine1/gpios/gpio3");
  add_group_member("green_leds", "pine1/gpios/gpio2");
  add_group_member("green_leds", "pine1/gpios/gpio3");
  printf(" test_group red_leds\n");
  print_group_list("red_leds");
  printf(" test_group green_leds\n");
  print_group_list("green_leds");
  printf(" test_group blue_leds\n");
  print_group_list("blue_leds");
  set_group_value("red_leds","on", "test code");
  return 0;

  add_group_member("red_leds", "pine1/gpios/gpio3");
  add_group_member("green_leds", "pine1/gpios/gpio2");
  add_group_member("green_leds", "pine1/gpios/gpio4");
  add_group_member("blue_leds", "pine1/gpios/gpio4");
  print_group_list("blue_leds");
  print_group_list("red_leds");
  print_group_list("green_leds");
  rem_group_member("red_leds", "pine1/gpios/gpio3");
  add_group_member("blue_leds", "pine1/gpios/gpio3");
  print_group_list("blue_leds");
  print_group_list("red_leds");
  print_group_list("green_leds");
  rem_group_member("green_leds", "pine1/gpios/gpio2");
  rem_group_member("green_leds", "pine1/gpios/gpio4");
  print_group_list("green_leds");
  if(sp1) rc = 0;
  return rc;
}

