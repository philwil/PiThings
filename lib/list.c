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
	  if(0&&g_list_debug)printf(" %s before citem insert item %p citem %p\n"
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
	  if(0&&g_list_debug)printf("before citem rem item %p citem %p\n", item , citem); 
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

struct list *del_list(struct list **root ,  struct list *item)
{
  struct list *nitem = item->next;
  struct list *pitem = item->prev;
  struct list *ritem = *root;
  if((nitem==pitem) && (nitem==item))
    {
      *root = NULL;
    }
  else
    {
      pitem->next = nitem;
      nitem->prev = pitem;
      if(ritem==item)
	*root = nitem;
    }
  return *root;
}

struct list *add_list(struct list **root ,  struct list *item)
{
  struct list *ritem = *root;
  struct list *nitem = NULL;
 struct list *pitem = NULL;
  if(ritem == NULL)
    {
    *root = item;
    }
  else
    {
      nitem = ritem->next;
      pitem = ritem->prev;
      
      if(pitem == nitem)
	{
	  pitem->next = item;
	  item->prev = pitem;
	  item->next = ritem;
	  ritem->prev = item;
	}
      else
	{
	  pitem->next = item;
	  item->prev = pitem;
	  item->next = ritem;
	  ritem->prev = item;
	}
    }
  return *root;
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

int show_list(struct list *root, char *msg)
{
  int rc = 0;
  struct list *item = root;
  struct list *ritem = NULL;

  if(msg)printf("show_list %s\n", msg); 
  while (foreach_item(&ritem, &item))
    {
      printf("item [%d] %p\n", rc, item->data); 
    }
  return rc;
}

struct list * xpop_list(struct list **root)
{
  struct list *item;
  struct list *pitem;
  struct list *nitem;
  item = *root;
  if(item)
    {
      if ((item->next == item->prev) && (item->next == item))
	{
	  *root = NULL;
	}
      else
	{
	  pitem = item->prev;
	  nitem = item->next;
	  *root = nitem;
	  nitem->prev = pitem;
	  pitem->next = nitem; 
	  item->prev = item;
	  item->next = item;
	}
    }
  return item;
}

int test_lists(void)
{
  struct list *root = NULL;
  struct list *item = NULL;
  item= new_list((void *)1);
  add_list(&root, item);

  //return 0;

 add_list(&root,new_list((void *)2));
 show_list(root, "after two adds");
  //return 0;
  add_list(&root,new_list((void *)3));
  add_list(&root,new_list((void *)4));
  add_list(&root,new_list((void *)5));
  show_list(root,"after six adds");

  item = find_list_item(root,(void *)3);
  del_list(&root,item);
  show_list(root,"after del 3");
  item = xpop_list(&root);
  show_list(root,"after xpop 1");
  item = xpop_list(&root);
  show_list(root,"after xpop 2");
  item = xpop_list(&root);
  show_list(root,"after xpop 3");
  item = xpop_list(&root);
  show_list(root,"after xpop 4");


  return 0;
}
