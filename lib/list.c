/* 
  nodes are used to store host/port connections
  any space (thing) can belong to a node
  we have a list of nodes and can add a new one if we need to
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

int g_list_debug = 0;

int push_clist(struct list **root, struct list *citem, struct list *item)
{

  struct list *pitem = NULL;
  struct list *nitem = NULL;
  //if(iobp)ciob= *iobp;
  if(citem)
    {
      nitem = citem->next;
      pitem = citem->prev;
      if ((pitem == nitem) && (pitem == citem))
	{
	  if(g_list_debug)printf("zero insert item %p citem %p\n", item , citem); 
	  citem->next = item;
	  citem->prev = item;
	  item->next = citem;
	  item->prev = citem;
	}
      else
	{
	  if(g_list_debug)printf("before citem insert item %p citem %p\n", item , citem); 
	  item->next = citem;
	  item->prev = pitem;
	  citem->prev = item;
	  pitem->next = item;
	}
    }
  else
    {
      if(root)*root=item;
    }
  return 0;
}
  
int push_list(struct list **root ,  struct list *item)
{
  struct list *citem = NULL;
  if(root)citem= *root;
  return push_clist(root, citem, item);
}


