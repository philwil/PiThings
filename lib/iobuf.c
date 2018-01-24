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
extern int g_new_iob_size;

struct iobuf *seek_iob(struct iobuf **inp, int len)
{
  struct iobuf *iob = *inp;
  struct iobuf *iobs = *inp;
  while (iob)
    {
      if(iob->outlen >= len)
	break;
      iob = iob->next;
      if(iob == iobs)
	iob = NULL;
    }
  if(iob)
    {
      if(iob == iobs)
	pull_iob(inp, NULL, 0);
      else
	pull_ciob(NULL, iob, NULL, 0);
    }
  return iob;
}

struct iobuf *new_iobuf(int len)
{
  struct iobuf *iob = NULL;
  printf(" %s running 1 len %d\n" , __FUNCTION__, len );
  iob = seek_iob(&g_iob_store, len); 
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
      iob->prev = iob;
      iob->next = iob;
    }
  return iob;
}


int extend_iobuf(struct iobuf * iob,char *buf, int len)
{
  memcpy(iob->outbuf+iob->outptr,buf,len);
  iob->outptr+=len;
  return 0;
}

int push_ciob(struct iobuf **iobp, struct iobuf *ciob, struct iobuf *iob)
{

  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  //if(iobp)ciob= *iobp;
  if(ciob)
    {
      niob = ciob->next;
      piob = ciob->prev;
      if ((piob == niob) && (piob == ciob))
	{
	  if(0)printf("zero insert iob %p ciob %p\n", iob , ciob); 
	  ciob->next = iob;
	  ciob->prev = iob;
	  iob->next = ciob;
	  iob->prev = ciob;
	}
      else
	{
	  if(0)printf("before ciob insert iob %p ciob %p\n", iob , ciob); 
	  iob->next = ciob;
	  iob->prev = piob;
	  ciob->prev = iob;
	  piob->next = iob;
	  
	}
    }
  else
    {
      if(iobp)*iobp=iob;
    }
  return 0;
}

int push_iob(struct iobuf **iobp ,  struct iobuf *iob)
{
  struct iobuf *ciob = NULL;
  if(iobp)ciob= *iobp;
  return push_ciob(iobp, ciob, iob);
}

int store_iob(struct iobuf **iobp ,  struct iobuf *iob)
{
  if(iob)
    {
      iob->outptr = 0;
      push_iob(iobp, iob);
    }
  return 0;

}

int xadd_iob(struct iosock *in, char *buf, int len)
{
  int need_push = 0;
  struct iobuf *iob = NULL;
  //struct iobuf *ciob = NULL;
  if (in->iobuf != NULL)
    {
      iob = in->iobuf;
    }
  else
    {
      iob = new_iobuf(len);
      need_push = 1;
    }

  if (iob->outlen+len >= iob->outsize)
    {
      iob = new_iobuf(len);
      need_push = 1;
    }
  extend_iobuf(iob, buf, len);
  if(need_push)
    {
      push_iob(&in->iobuf, iob);
    }
  return 0;
}

struct iobuf *pull_ciob(struct iobuf **inp, struct iobuf *ciob, char **bufp, int *lenp)
{
  struct iobuf *piob = NULL;
  struct iobuf *niob = NULL;
  //int rc = -1;
  if(lenp) *lenp = 0;
  if (ciob != NULL)
    {

     if(bufp) *bufp = &ciob->outbuf[ciob->outptr];
     if(lenp) *lenp = ciob->outlen-ciob->outptr;
     if ((ciob->prev == ciob->next) && (ciob->prev == ciob))
       {
	 if(0)printf(" pull last iob %p\n", ciob);
	 if(inp)*inp = NULL;
       }
     else
       {
	 if(0)printf(" pull this iob 1  ciob %p niob %p piob %p\n"
		, ciob, ciob->next, ciob->prev);
	 niob = ciob->next;
	 piob = ciob->prev;
	 if(inp)*inp = niob;
         niob->prev = piob;
	 piob->next = niob;
	 if(0)printf(" pull this iob 2  ciob %p next %p prev %p\n"
		, niob, niob->next,niob->prev);

       }
     ciob->next = ciob;
     ciob->prev = ciob;
    }
  return ciob;
}

struct iobuf *pull_iob(struct iobuf **inp, char **bufp, int *lenp)
{
  struct iobuf *ciob = NULL;
  if(inp)  ciob = *inp;
  return pull_ciob(inp, ciob, bufp, lenp);
}

int print_iob(struct iobuf *iob)
{
  printf("@%p next@%p  out p/l/s %d/%d/%d [%s]\n"
	 , iob
	 , iob->next
	 , iob->outptr   // where we read from
	 , iob->outlen   // where we write to
	 , iob->outsize  // space
	 , iob->outbuf
	 );
  return 0;
}

int print_iobs(struct iobuf *in)
{
  int rc = 0;
  struct iobuf *iob = NULL;
  struct iobuf *siob = NULL;

  siob = in;//in->iobuf;
  iob = in;//in->iobuf;

  while(iob)
    {
      rc++;
      print_iob(iob);
      iob = iob->next;
      if(iob == siob) iob = NULL;
    }
  return rc;
}

int remove_iobs(struct iobuf **in)
{
  int rc = 0;
  struct iobuf *iob = NULL;
  struct iobuf *liob = NULL;
  struct iobuf *siob = NULL;

  siob = *in;//in->iobuf;
  iob = *in;//in->iobuf;

  while(iob)
    {
      rc++;
      if(iob->outbuf)free(iob->outbuf);
      
      liob = iob;
      iob = iob->next;
      if(iob == siob) iob = NULL;
      if(liob)free(liob);
    }
  *in = NULL;
  return rc;
}


//
int test_iob(void)
{
  struct iosock inx;
  struct iosock *in = &inx;
  char *sp;
  int len;
  struct iobuf *iob= NULL;
  struct iobuf *iob1= NULL;
  struct iobuf *iob2= NULL;
  struct iobuf *iob3= NULL;

  init_iosock(in);

  if(g_debug)
    printf(" After init :-\n");
  print_iobs(in->iobuf);
  sp = "1 first inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 1 :-\n");
  print_iobs(in->iobuf);
  sp = "2 next inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 2 :-\n");
  print_iobs(in->iobuf);

  sp = "3 lastst inblock\n";
  in_snprintf(in, NULL, "%s", sp);
  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After last :-\n");
  print_iobs(in->iobuf);
  //ciob = in->iobuf;

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 1 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  printf("\n\n");

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 2 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 3 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 4 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore follows\n");
  print_iobs(g_iob_store);
  iob = new_iobuf(12);
  if(g_debug)
    printf("\n\n iobstore after small pull %p\n", iob);
  print_iobs(g_iob_store);

  if(iob) store_iob(&g_iob_store, iob);
  iob = new_iobuf(120);
  if(g_debug)
    printf("\n\n iobstore after large pull %p\n", iob);
  print_iobs(g_iob_store);
  if(iob) store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore after store %p\n", iob);
  print_iobs(g_iob_store);
  remove_iobs(&g_iob_store);
  iob1 = new_iobuf(12);
  in_snprintf(NULL, iob1, "the name [%s] value is %d ", "some_name", 22);
  in_snprintf(NULL, iob1, "more stuff  the name [%s] value is %d ", "some_name", 23);
  if(g_debug)
    printf("\n\n iob 1 %p after snprintf  [%s] prev %p next %p \n"
	   , iob1
	   , iob1->outbuf
	   , iob1->next
	   , iob1->prev
	   );
  iob2 = iob1->next;
  if(g_debug)
    printf("\n\n iob 2 %p after snprintf  [%s] prev %p next %p \n"
	   , iob2
	   , iob2->outbuf
	   , iob2->next
	   , iob2->prev
	   );
  iob3 = iob2->next;
  if(g_debug)
    printf("\n\n iob 3 %p after snprintf  [%s] prev %p next %p \n"
	   , iob3
	   , iob3->outbuf
	   , iob3->next
	   , iob3->prev
	   );

  return 0;
}

// test new list interface
int test_niob(void)
{
  struct iosock inx;
  struct iosock *in = &inx;
  char *sp;
  int len;
  struct iobuf *iob= NULL;
  struct iobuf *iob1= NULL;
  struct iobuf *iob2= NULL;
  struct iobuf *iob3= NULL;

  struct list *slist;
  struct list *item;
  g_new_iob_size =  16;
  init_iosock(in);
  sp = "first inblock";
  printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);
  //printf(" test sp [%s] done\n", sp);

  sp = "second longer inblock ";
  printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);

  sp = "end";
  printf(" test sp [%s]\n", sp);
  in_snprintf(in, NULL, "%s", sp);

  item = in->oubuf_list;
  printf(" test item [%p]\n", item);
  slist = NULL;
  while(foreach_item(&slist, &item))
    {
      
      iob = item->data;
      
      printf(" iob %p data len/size %d/%d->[%s]\n"
	     , iob
	     , iob->outlen
	     , iob->outsize
	     , iob->outbuf
	     );
    }
  return 0;
  
  init_iosock(in);

  if(g_debug)
    printf(" After init :-\n");
  print_iobs(in->iobuf);
  
  sp = "1 first inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 1 :-\n");
  print_iobs(in->iobuf);
  sp = "2 next inblock\n";
  in_snprintf(in, NULL, "%s", sp);

  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After 2 :-\n");
  print_iobs(in->iobuf);

  sp = "3 lastst inblock\n";
  in_snprintf(in, NULL, "%s", sp);
  //add_iob(in, sp, strlen(sp));
  if(g_debug)
    printf(" After last :-\n");
  print_iobs(in->iobuf);
  //ciob = in->iobuf;

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 1 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  printf("\n\n");

  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 2 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 3 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n");
  iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" After pull 4 [%s] len %d iob %p\n", sp, len, iob);
  print_iobs(in->iobuf);
  store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore follows\n");
  print_iobs(g_iob_store);
  iob = new_iobuf(12);
  if(g_debug)
    printf("\n\n iobstore after small pull %p\n", iob);
  print_iobs(g_iob_store);

  if(iob) store_iob(&g_iob_store, iob);
  iob = new_iobuf(120);
  if(g_debug)
    printf("\n\n iobstore after large pull %p\n", iob);
  print_iobs(g_iob_store);
  if(iob) store_iob(&g_iob_store, iob);
  if(g_debug)
    printf("\n\n iobstore after store %p\n", iob);
  print_iobs(g_iob_store);
  remove_iobs(&g_iob_store);
  iob1 = new_iobuf(12);
  in_snprintf(NULL, iob1, "the name [%s] value is %d ", "some_name", 22);
  in_snprintf(NULL, iob1, "more stuff  the name [%s] value is %d ", "some_name", 23);
  if(g_debug)
    printf("\n\n iob 1 %p after snprintf  [%s] prev %p next %p \n"
	   , iob1
	   , iob1->outbuf
	   , iob1->next
	   , iob1->prev
	   );
  iob2 = iob1->next;
  if(g_debug)
    printf("\n\n iob 2 %p after snprintf  [%s] prev %p next %p \n"
	   , iob2
	   , iob2->outbuf
	   , iob2->next
	   , iob2->prev
	   );
  iob3 = iob2->next;
  if(g_debug)
    printf("\n\n iob 3 %p after snprintf  [%s] prev %p next %p \n"
	   , iob3
	   , iob3->outbuf
	   , iob3->next
	   , iob3->prev
	   );

  return 0;
}

int count_buf_bytes(struct iobuf *oubuf)
{
  int rc=0;
  struct iobuf *oust = oubuf;
  while(oubuf)
    {
      rc += oubuf->outlen - oubuf->outptr;
      oubuf = oubuf->next;
      if(oubuf == oust)
	oubuf = NULL;
    }
  
  
  return rc;
}

int test_iob_out(void)
{

  struct iosock iosock;
  struct iosock *in;
  int rc;
  char *sp;
  sp = "This is a direct test\n";
  in = &iosock;
  init_iosock(in);
  in->fd = 1;
  rc = write(in->fd, sp, strlen(sp));
  printf(" sent %d chars\n", rc);	     

  sp = "** first string**";
  in_snprintf(in, NULL, " in_sprintf 1 sp [%s] num %d\n", sp,21);
  //add_iob(in, sp, strlen(sp));
  printf(" after in_snprintf 1\n");
  print_iobs(in->iobuf);
  printf(" in->outblen %u\n", in->outblen);
  printf(" in->outbptr %u\n", in->outbptr);
  
  sp = " ** adding a much longer second string  twice **";
  in_snprintf(in, NULL, " in_sprintf 2 sp [%s] [%s] num %d\n", sp,sp, 22);

  //add_iob(in, sp, strlen(sp));
  printf(" after in_snprintf 2\n");
  print_iobs(in->iobuf);
  printf(" in->outblen %u\n", in->outblen);
  printf(" in->outbptr %u\n", in->outbptr);
  printf(" ===================\n");
  handle_output(in);
  printf(" ===================\n\n");
  printf(" after handle_output\n\n");
  print_iobs(in->iobuf);

  
  return 0;
}
