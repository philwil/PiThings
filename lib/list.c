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

extern int g_list_debug;

struct list *foreach_item(struct list **start, struct list **item)
{
  struct list *ret;
  ret = *item;

  if(*start == NULL)
    {
      *start = *item;
    }
  else
    {
      *item = ret->next;
      if(*item == *start) ret = NULL;
    }
  return ret;
}

struct list *new_list(void *data)
{
  struct list *item;
  item = (struct list *)malloc(sizeof(struct list));
  item->next = item;
  item->prev = item;
  item->data = data;

  return item;
}
// citem = current item
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
	  if(g_list_debug)printf("%s zero insert item %p citem %p\n"
				 , __FUNCTION__
				 , item , citem); 
	  citem->next = item;
	  citem->prev = item;
	  item->next = citem;
	  item->prev = citem;
	}
      else
	{
	  if(g_list_debug)printf(" %s before citem insert item %p citem %p\n"
				 , __FUNCTION__
				 , item , citem); 
	  //	  item->next = citem;
	  //item->prev = pitem;
	  //citem->prev = item;
	  //pitem->next = item;
	  //citem->next = item;
	  citem->prev = item;
	  if(citem->next == citem)
	    citem->next = item;
	  item->next = citem;
	  item->prev = pitem;
	  pitem->next = item;
	}
    }
  else
    {
      if(root)
	{
	  if(g_list_debug)
	    printf(" %s null root insert item %p citem %p\n"
		   , __FUNCTION__
		   , item , citem); 
	  *root=item;
	  item->next = item;
	  item->prev = item;

	}
    }
  return 0;
}
  
int push_list(struct list **root,  struct list *item)
{
  struct list *citem = NULL;
  if(*root)citem = *root;
  return push_clist(root, citem, item);
}


struct list *pop_clist(struct list **root, struct list *citem, struct list *item)
{

  struct list *pitem = NULL;
  struct list *nitem = NULL;
  //if(iobp)ciob= *iobp;
  if(citem)
    {
      nitem = item->next;
      pitem = item->prev;
      if ((pitem == nitem) && (pitem == item))
	{
	  if(g_list_debug)printf("zero rem item %p citem %p\n", item , citem); 
	  if(root)*root=NULL;
	  item =  item;
	}
      else
	{
	  if(g_list_debug)printf("before citem rem item %p citem %p\n", item , citem); 
	  nitem->prev = item->prev;
	  pitem->next = item->next;

	  item->next = item;
	  item->prev = item;
	  if(root)*root=nitem;

	}
    }
  else
    {
      if(root)*root=NULL;
    }
  return item;
}

struct list *pop_list(struct list **root ,  struct list *item)
{
  struct list *citem = NULL;
  if(root)citem= *root;
  if(!citem) return NULL;   // nothing on the list
  return pop_clist(root, citem, item);
}


struct list *find_list_item(struct list *master, void *data)
{
  struct list *item = master;
  struct list *sitem = master;
  while(item)
    {
      if(item->data == data)
	break;
      item = item->next;
      if (item == sitem) item =  NULL;
    }

  return item;
}
