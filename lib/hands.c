
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

extern struct hands g_hands[];
int num_hands_alloc;
int num_hands_used;

int init_hands(int num)
{
  int i;
  for (i = 0; i< num; i++)
  {
      g_hands[i].key = NULL;
      g_hands[i].desc = NULL;
      g_hands[i].handler = NULL;
      g_hands[i].new_hand = NULL;
  }
  return num;
}

//int dummy_handler(int fd, char *id, char *buf, int len)
//init_new_hand("some_id", "Dummy Handler",  dummy_handler);
int init_new_hand(char *key, char *desc, int(*handler)
		  (int fd, char *id,char *buf, int len))
{
  int i;
  struct hands *myhand;
  for (i = 0; i< NUM_HAND; i++)
    {
      myhand = &g_hands[i];
      if(!myhand->key)
	{
	  myhand->key =  key;
	  myhand->desc =  desc;
	  myhand->new_hand = handler;
	  myhand->handler = NULL;
	  break;
	}
    }
  if(i == NUM_HAND)
    {
      i = -1;
    }
  return i;
}

//int dummy_handler(int fd, char *id, char *buf, int len)
int run_new_hand(char *key, int fd, char *buf, int len)
{
  int rc=-1;
  int i;
  struct hands *myhand;
  //myhand = g_hands[0];

  for (i = 0; i< NUM_HAND; i++)
    {
      myhand = &g_hands[i];

      if(myhand->key && (strcmp(myhand->key, key) == 0))
	{
	  rc = myhand->new_hand(fd, key, buf, len);
	  break;
	}
    }
  rc = i;
  if(i == num_hands_used)
    rc = -1;
  return rc;
}
