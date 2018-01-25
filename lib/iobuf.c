/* 
  iobufs are used to store a sequence of input or output bufers
  we build up a list of unused ones and grab from that list when needed  
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

extern struct iobuf *g_iob_store;
extern struct list *g_iob_list;
extern int g_new_iob_size;

int g_debug_iob_list=0;

// foreach neat list cycle function
// look in the list for a suitable item based on a size match (len)
// int push_list(struct list **listp, struct list*item);
// struct list *pop_list(struct list **listp, struct list*item);
// used in new_iobuf in cmds.c:in_snprintf
// test_niob for full test
struct list *seek_new_iob_item(struct list **listp, int len)
{
  struct iobuf *iob = NULL;
  struct list *slist;
  struct list *item;
  struct list *fitem=NULL;

  item = *listp;

  if(g_debug_iob_list)
    printf(" test item [%p]\n", item);
  slist = NULL;
  while(foreach_item(&slist, &item))
    {
      
      iob = item->data;
      if( g_debug_iob_list)
	printf(" iob %p data len/size %d/%d->[%s]\n"
	       , iob
	       , iob->outlen
	       , iob->outsize
	       , iob->outbuf
	     );
      if(iob->outsize>len)
	{
	  fitem=item;
	  break;
	}
    }
  if(fitem)
    pop_list(listp, fitem);
  return fitem;
}
// returns a list object
struct list *new_iobuf_item(int len)
{
  struct iobuf *iob = NULL;
  if(g_debug_iob_list)
    printf(" %s running 1 len %d\n" , __FUNCTION__, len );
  struct list *item = seek_new_iob_item(&g_iob_list, len);

  if(item)
    {
      iob = item->data;
      // for now we just need the IOB TODO pass around the list
      //free(item);
    }
  if(!iob)
    {
      int xlen = len+5;
      printf(" %s running, create iob\n" , __FUNCTION__);
      
      iob = (struct iobuf *)malloc(sizeof (struct iobuf));


      iob->outbuf = (char *)malloc(xlen);
      iob->outbuf[0] = 0;
      iob->outsize=xlen;
      iob->outptr=0;
      iob->outlen=0;
      //iob->prev = iob;
      //iob->next = iob;
      item = new_list(iob);
    }
  return item;
}


int extend_iobuf(struct iobuf * iob,char *buf, int len)
{
  memcpy(iob->outbuf+iob->outptr,buf,len);
  iob->outptr+=len;
  return 0;
}


int xadd_iob(struct iosock *in, char *buf, int len)
{
  int need_push = 0;
  struct iobuf *iob = NULL;
  struct list *item;
  //struct iobuf *ciob = NULL;
  if (in->oubuf_list != NULL)
    {
      item = in->oubuf_list;
    }
  else
    {
      item = new_iobuf_item(len);
      need_push = 1;
    }
  iob = item->data;
  if (iob->outlen+len >= iob->outsize)
    {
      item = new_iobuf_item(len);
      iob = item->data;
      need_push = 1;
    }
  extend_iobuf(iob, buf, len);
  if(need_push)
    {
      push_list(&in->oubuf_list, item);
    }
  return 0;
}


struct list *pull_in_iob(struct iosock *in, char **spp, int*len)
{
  struct iobuf *iob=NULL;
  struct list *item;
  item = in->oubuf_list;
  printf(" test item [%p]\n", item);
  if(item)
    {
      iob = item->data;
      *len = iob->outlen;
      *spp = iob->outbuf;
      printf(" After pull [%s] len %d iob %p\n", *spp, *len, iob);
      pop_list(&in->oubuf_list, item);
    }

  //if(g_debug)

  return item;
}

int print_iob_list(struct list **item, char *why)
{
  int rc = 0;
  struct list *sitem;
  struct list *titem;
  struct iobuf *iob= NULL;
  sitem = NULL;
  if(!*item)
    {
      printf("%s ERROR test item [%p] %s\n", __FUNCTION__, *item, why);
      return 0;
    }
  printf("%s test item [%p] %s\n", __FUNCTION__, *item, why);

  while(foreach_item(&sitem, item))
    {
      titem = *item;
      iob = titem->data;
      
      printf(" iob %p data len/size %d/%d->[%s]\n"
	     , iob
	     , iob->outlen
	     , iob->outsize
	     , iob->outbuf
	     );
    }
  return rc;
}

int handle_noutput(struct iosock *in)
{
  int rc = 0;
  struct iobuf *iob;
  struct list *item;
  char *sp;
  int len;
  int bcount = 0;
  // iobway
  while((bcount++ < 1024) && (in->outbptr != in->outblen))
    {
      len = 0;
      item = pull_in_iob(in, &sp, &len);
      if(!item)
	{
	  printf(" %s term nothing left outbptr/blen %u/%u \n"
		 , __FUNCTION__
		 , in->outbptr
		 , in->outblen
		 );
	  break;
	}
      iob = item->data;
      //iob = pull_iob(&in->iobuf, &sp, &len);
      if(0)printf(" %s running the new way iob %p len %d sp [%s]\n"
		  , __FUNCTION__, iob, len, sp);
      if(iob)
	{
	  if(0)print_iob(iob);
	}
      else
	{
	  in->outbptr = 0;
	  in->outblen = 0;
	}
      if(len > 0)
	{
	  rc = write(in->fd, sp, len);
	  if(rc <= 0)
	    {
	      // shutdown iobuffer
	      // TODO unload iobs
	      in->outbptr = 0;
	      in->outblen = 0;
	    }
	  if(rc >0)
	    {
	      in->outbptr += rc;
	      if (in->outbptr == in->outblen)
		{
		  in->outbptr = 0;
		  in->outblen = 0;
		}
	      iob->outptr+=rc;
	      if(iob->outptr == iob->outlen)
		{
		  iob->outptr= 0;
		  iob->outlen= 0;
		  push_list(&g_iob_list, item);
		}
	      
	    }
	}
      //TODO push_iob(&g_iob_list, iob);
    }
  return rc;
}



// test new list interface
// TODO check stash on g_iob_list
// check handle_noutput
int test_niob(void)
{
  struct iosock inx;
  struct iosock *in = &inx;
  char *sp;
  int len;
  struct iobuf *iob= NULL;
  //  struct iobuf *iob1= NULL;
  //struct iobuf *iob2= NULL;
  //struct iobuf *iob3= NULL;

  struct list *item;
  g_new_iob_size =  24;
  init_iosock(in);
  in->fd = 1;
  sp = "<<dont send this block>>";
  //printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);
  printf(" test sp [%s] in->outblen %lu sp len (%lu)\n"
	 , sp, (long unsigned int)in->outblen, (long unsigned int)strlen(sp));

  sp = "first inblock ";
  //printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);
  printf(" test sp [%s] in->outblen %lu sp len (%lu)\n"
	 , sp, (long unsigned int)in->outblen, (long unsigned int)strlen(sp));

  //printf(" test sp [%s] done\n", sp);

  sp = "second longer inblock ";
  in_snprintf(in, NULL, "%s", sp);
  printf(" test sp [%s] in->outblen %lu sp len (%lu)\n"
	 , sp, (long unsigned int)in->outblen, (long unsigned int)strlen(sp));

  sp = "end!!";
  //printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);
  printf(" test sp [%s] in->outblen %lu sp len (%lu)\n"
	 , sp, (long unsigned int)in->outblen, (long unsigned int)strlen(sp));

  item = in->oubuf_list;
  print_iob_list(&item, "full list");
  item = pull_in_iob(in, &sp, &len);
  push_list(&g_iob_list, item);
  
  // must correct size in->outbptr
  iob = item->data;
  in->outbptr += iob->outlen;
  if(g_debug)
    printf(" After pull 1 data [%s] len %d iob %p\n", sp, len, iob);
  iob->outlen = 0;
  iob->outptr = 0;
  
  item = in->oubuf_list;

  printf(" after pull list  [%p]\n", item);
  print_iob_list(&item,"after first pull");

  printf("============================handle_noutput=========\n");

  handle_noutput(in);
  printf("\n============================handle_noutput=========\n");
  print_iob_list(&g_iob_list,"after handle_noutput");

  return 0;

}

//foreach
int count_iob_bytes(struct list **listp)
{
  struct iobuf *iob = NULL;
  struct list *slist;
  struct list *item;
  int len =0;
  item = *listp;

  if(g_debug_iob_list)
    printf(" test item [%p]\n", item);
  slist = NULL;
  while(foreach_item(&slist, &item))
    {
      
      iob = item->data;
      len += iob->outlen - iob->outptr;

      if( g_debug_iob_list)
	printf(" iob %p data len/size %d/%d->[%s]\n"
	       , iob
	       , iob->outlen
	       , iob->outsize
	       , iob->outbuf
	     );
    }
  return len;
}

