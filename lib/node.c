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

extern struct list *g_node_list;
extern int g_node_debug;

struct list *seek_node_list(struct list **root, char *addr, int port)
{
  struct list *item = *root;
  struct list *items = *root;
  struct node *node;
  while (item)
    {
      node = (struct node*)item->data;
      if((node->port == port) && (strcmp(node->addr, addr)==0))
	break;
      item = item->next;
      if(item == items)
	item = NULL;
    }
  return item;
}

// get_node (&g_node_store,"127.0.0.1", 5432);
struct list *get_node_list(struct list **root, char *addr, int port)
{
  struct list *item = NULL;
  struct node *node = NULL;

  item = seek_node_list(root, addr, port); 
  if(!item)
    {
      item = (struct list *)malloc(sizeof (struct list));
      node = (struct node *)malloc(sizeof (struct node));
      
      node->addr = strdup(addr);
      node->port = port;
      node->fd = -1;
      item->data = node;
      item->prev = item;
      item->next = item;
    }
  return item;
}




int print_node_item(struct list *item)
{
  struct node *node = NULL;
  node = (struct node*)item->data;

  printf("@%p next@%p prev@%p  port %d addr [%s] fd %d\n"
	 , item
	 , item->next
	 , item->prev
	 , node->port   // where we read from
	 , node->addr   // where we write to
	 , node->fd   // where we write to

	 );
  return 0;
}

int print_node_list(struct list *item)
{
  int rc = 0;
  //struct node *item = NULL;
  struct list *sitem = NULL;

  sitem = item;//in->iobuf;

  while(item)
    {
      rc++;
      print_node_item(item);
      item = item->next;
      if(item == sitem) item = NULL;
    }
  return rc;
}


int remove_node_list(struct list **root)
{
  int rc = 0;
  struct list *item = NULL;
  struct list *litem = NULL;
  struct list *sitem = NULL;
  struct node *node = NULL; 

  sitem = *root;//in->iobuf;
  item = *root;//in->iobuf;

  while(item)
    {
      node = (struct node *)item->data; 
      rc++;
     
      if(node->addr)free(node->addr);
      if(node->fd > 0)close(node->fd);
      free(node);
      litem = item;
      item = item->next;
      if(item == sitem) item = NULL;
      if(litem)free(litem);
    }
  *root = NULL;
  return rc;
}


//
int test_node_list(void)
{
  struct list *item;
  printf(" test_node_list\n");

  item = get_node_list(&g_node_list,"127.0.0.1", 5432);
  push_list(&g_node_list, item);

  item = get_node_list(&g_node_list,"127.0.0.1", 6000);
  push_list(&g_node_list, item);

  item = get_node_list(&g_node_list,"127.0.0.1", 7000);
  push_list(&g_node_list, item);

  print_node_list(g_node_list);
  remove_node_list(&g_node_list);
  return 0;
}

