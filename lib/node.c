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

extern struct node *g_node_store;
extern int g_node_debug;

struct node *seek_node(struct node **root, char *addr, int port)
{
  struct node *item = *root;
  struct node *items = *root;
  while (item)
    {
      if((item->port == port) && (strcmp(item->addr, addr)==0))
	break;
      item = item->next;
      if(item == items)
	item = NULL;
    }
  return item;
}

// get_node (&g_node_store,"127.0.0.1", 5432);
struct node *get_node(struct node**root, char *addr, int port)
{
  struct node *item = NULL;

  item = seek_node(root, addr, port); 
  if(!item)
    {
      item = (struct node *)malloc(sizeof (struct node));
      
      item->addr = strdup(addr);
      item->port = port;
      item->fd = -1;
      item->prev = item;
      item->next = item;
    }
  return item;
}


int push_cnode(struct node **root, struct node *citem, struct node *item)
{

  struct node *pitem = NULL;
  struct node *nitem = NULL;
  //if(iobp)ciob= *iobp;
  if(citem)
    {
      nitem = citem->next;
      pitem = citem->prev;
      if ((pitem == nitem) && (pitem == citem))
	{
	  if(g_node_debug)printf("zero insert item %p citem %p\n", item , citem); 
	  citem->next = item;
	  citem->prev = item;
	  item->next = citem;
	  item->prev = citem;
	}
      else
	{
	  if(g_node_debug)printf("before citem insert item %p citem %p\n", item , citem); 
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
  
int push_node(struct node **root ,  struct node *item)
{
  struct node *citem = NULL;
  if(root)citem= *root;
  return push_cnode(root, citem, item);
}

int print_node(struct node *item)
{
  printf("@%p next@%p prev@%p  port %d addr [%s] fd %d\n"
	 , item
	 , item->next
	 , item->prev
	 , item->port   // where we read from
	 , item->addr   // where we write to
	 , item->fd   // where we write to

	 );
  return 0;
}

int print_nodes(struct node *item)
{
  int rc = 0;
  //struct node *item = NULL;
  struct node *sitem = NULL;

  sitem = item;//in->iobuf;

  while(item)
    {
      rc++;
      print_node(item);
      item = item->next;
      if(item == sitem) item = NULL;
    }
  return rc;
}


int remove_nodes(struct node **root)
{
  int rc = 0;
  struct node *item = NULL;
  struct node *litem = NULL;
  struct node *sitem = NULL;

  sitem = *root;//in->iobuf;
  item = *root;//in->iobuf;

  while(item)
    {
      rc++;
      
      if(item->addr)free(item->addr);
      if(item->fd > 0)close(item->fd);
      
      litem = item;
      item = item->next;
      if(item == sitem) item = NULL;
      if(litem)free(litem);
    }
  *root = NULL;
  return rc;
}


//
int test_nodes(void)
{
  struct node *node;
  node = get_node(&g_node_store,"127.0.0.1", 5432);
  push_node(&g_node_store, node);

  node = get_node(&g_node_store,"127.0.0.1", 6000);
  push_node(&g_node_store, node);

  node = get_node(&g_node_store,"127.0.0.1", 7000);
  push_node(&g_node_store, node);

  print_nodes(g_node_store);
  remove_nodes(&g_node_store);
  return 0;
}

