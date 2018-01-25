
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

extern struct iosock g_iosock[];
extern struct iobuf *g_iob_store;
//extern struct space *g_space;
extern int g_space_idx;
extern int g_quit_one;
extern int g_num_socks;


extern struct space *g_spaces[];
extern struct list *g_space_list;
extern struct list *g_iob_list;

int init_iosocks(void)
{
  int i;
  struct iosock *in;
  for (i = 0; i< NUM_SOCKS; i++)
  {

      in = &g_iosock[i];
      init_iosock(in);
  }
  return i;
}

int init_iosock(struct iosock *in)
{

  in->fd = -1;
  in->outbptr = 0;
  in->outblen = 0;
  in->oubuf = NULL;
  in->inbuf = NULL;
  //in->cmdptr = NULL;
  in->cmdlen = 0;
  in->hlen = 0;
  in->hidx = -1;
  in->hproto = 0;

  in->cmdbytes = 0;
  //in->cmdtrm = 0;
  in->cmdid = NULL;
  in->tlen = 0;
  in->nosend = 0;
  in->instate = STATE_IN_NORM;

  in->inbuf_list=NULL;
  in->oubuf_list=NULL;

  in->initem=NULL;
  in->ouitem=NULL;

  return 0;
}



//struct iobuf/
//{
//struct iobuf *prev;
//struct iobuf *next;
//char *outbuf;
//int outlen;
//int outptr;
//};
int connect_socket(int portno, char *addr)
{
     int sockfd;
     //char buffer[256];
     struct sockaddr_in serv_addr;
     //int n;
     //int data;
     char *sp;

     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     if(addr)
       {
	 sp =  addr;
       }
     else
       {
	 sp = "127.0.0.1";
       }
     if(inet_pton(AF_INET, sp, &serv_addr.sin_addr)<=0)
       {
	 printf("\n inet_pton error occured\n");
	 return 1;
       }

     serv_addr.sin_port = htons( portno );
     if (connect(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
	 printf("ERROR on connect");
	 close(sockfd);
	 return -1;
     }
     printf( "connect sock #%d\n", sockfd );
     return sockfd;
} 

int listen_socket(int portno)
{
     int sockfd;
     //char buffer[256];
     struct sockaddr_in serv_addr;
     //int n;
     //int data;
     int optval;
     
     printf( "using port #%d\n", portno );
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     {
	 printf("ERROR opening socket");
	 return -1;
     }
     optval = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
	       (const void *)&optval , sizeof(int));
      
#if 0
#ifdef SO_REUSEPORT
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif
#endif

     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons( portno );
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
	 printf("ERROR on binding");
	 close(sockfd);
	 return -1;
     }
     listen(sockfd,5);
     printf( "listen sock #%d\n", sockfd );

     return sockfd;
} 

int add_socket(int sockfd)
{
  int i;
  for (i = 0; i<NUM_SOCKS; i++)
    {
      if (g_iosock[i].fd < 0)
	{
	  init_iosock(&g_iosock[i]);
	  g_iosock[i].fd = sockfd;
	  g_num_socks++;
	  i = NUM_SOCKS;
	}
    }
  return sockfd;
}

int accept_socket_fd(int sockfd)
{
     int newsock, clilen;
     struct sockaddr_in cli_addr;
     //int i;
     if (sockfd != STDIN_FILENO)
     {
	 clilen = sizeof(cli_addr);
	 if ( ( newsock = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
	   {
	     printf("ERROR on accept\n");
	     return -1;
	   }
	 printf( "opened new communication with client \n" );
     }
     else
     {
	 newsock = sockfd;
     }
     return add_socket(newsock);
}


int accept_socket(int sockfd)
{
     int newsock, clilen;
     struct sockaddr_in cli_addr;
     //int i;
     if (sockfd != STDIN_FILENO)
     {
	 clilen = sizeof(cli_addr);
	 if ( ( newsock = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
	   {
	     printf("ERROR on accept\n");
	     return -1;
	   }
	 printf( "opened new communication with client \n" );
     }
     else
     {
	 newsock = sockfd;
     }
     return add_socket(newsock);
}

struct iosock *find_fd(int fsock)
{
    int i;
    struct iosock *in = NULL;
    for (i = 0; i< NUM_SOCKS; i++)
    {
        if (g_iosock[i].fd == fsock)
        {
	    in = &g_iosock[i];
	    break;
        }
    }
    return in;
}

int close_fds(int fsock)
{
    int rc = -1;
    struct iosock *in = find_fd(fsock);
    if (in)
      {
	// TODO drain iobufs	
	rc = 0;
	in->fd = -1;
	//in->inptr = 0;
	//in->inlen = 0;
	
	g_num_socks--;
      }
    return rc;
}

int get_rsize(struct iosock *in)
{
  //int rc=0;
  int rlen = 0;
  struct iobuf *inbf;  // input buffer

  inbf = in->inbuf;
  rlen = inbf->outsize-inbf->outlen;
  return rlen;
}

int handle_input_cmd(struct iosock *in)
{
    int rc=0;
    //int len=0;
    int n;
    char cmd[64];
    char *sp;
    struct iobuf *inbf;  // input buffer
    //struct iobuf *oubf;  // input buffer
    //int rsize;
    int tosend;
    int bytesin;

    inbf = in->inbuf;

    bytesin = inbf->outlen - inbf->outptr;
    if(in->cmdbytes > 0)
      {
	in->cmdlen -= bytesin;
      }
    if(in->cmdlen < 0)
      {
	in->cmdlen = 0;
      }
    if(g_debug)
      printf("%s bytesin %d cmdlen %d\n"
	     , __FUNCTION__
	     , bytesin
	     , in->cmdlen);

    sp = &inbf->outbuf[inbf->outptr];

    if(in->cmdlen <= 0)
      {
	//in->cmdptr = sp;

	n = sscanf(sp, "%s ", cmd);   //TODO use better sscanf
	if(0)in_snprintf(in, NULL
			 ," message received [%s] ->"
			 " n %d cmd [%s]\n"
			 , sp //&in->inbuf[in->inptr]
			 , n, cmd );

	in->tlen = in->cmdbytes;
	in->cmdbytes = 0;

      	in->instate = STATE_IN_NORM;
	run_str_in(in, sp, cmd);
	// TODO consume just the current cmd
	inbf->outptr += in->tlen;
	tosend = count_iob_bytes(&in->oubuf_list);
	if(g_debug)
	  printf(" %s rc %d n %d  cmd [%s] tosend %d tlen %d cmdid [%s]\n"
		 , __FUNCTION__
		 , rc, n, cmd, tosend
		 , in->tlen
		 , in->cmdid ? in->cmdid :"no id"
		 );
	//TODO use in_sprintf
	snprintf(cmd, sizeof(cmd), "REP %s %d\n\n"
		, in->cmdid
		, tosend
		);
	write (in->fd, cmd, strlen(cmd));
	if (inbf->outptr < inbf->outlen)
	  {
	    rc = 1;
	  }
      }
    else
      {
	if(in->cmdbytes > 0)
	  {
	    printf(">>>>>input still needs %d bytes\n"
		   , in->cmdlen );
	    rc = 1;
	  }
      }
    return rc;
}


int handle_input_rep(struct iosock *in)
{
    int rc=0;
    int len=0;
    int n;
    char cmd[64];
    char *sp;
    //char buf[1024];
    //char *bres;
    //int lres;
    //char *bufp;
    struct iobuf *inbf;  // input buffer
    //struct iobuf *oubf;  // input buffer
    int rsize;
    int tosend=0;
    int bytesin;

    inbf = in->inbuf;
    sp = &inbf->outbuf[inbf->outptr];

    rsize = in->cmdbytes - in->cmdlen;
    bytesin = inbf->outlen - inbf->outptr;
    if(g_debug)
      {
	printf("%s rsize  %d bytesin %d\n", __FUNCTION__, rsize, bytesin);
      }
    if (bytesin >= rsize)
      {
	in->cmdlen = 0;
	rsize = 0;
	rc = 1;
      }
    if (bytesin < rsize)
      {
	rsize -= bytesin;
	if(g_debug)
	  {
	    printf("%s adjusted rsize  %d\n", __FUNCTION__, rsize);
	  }
      }
    // we need more
    if(rsize > 0)
      {
	return 0;
      }
    if(g_debug)
      {
	printf("%s read sp [%s] outptr/len %d/%d/ cmd/bytes %d/%d\n"
	       , __FUNCTION__
	       , sp
	       , inbf->outptr
	       , inbf->outlen
	       , in->cmdlen
	       , in->cmdbytes
	       );
      }

    sp = &inbf->outbuf[inbf->outptr];
    //TODO only run this if we have received all of the command
    // use in->cmdbytes to count remaining bytes if any
    if(in->cmdlen <= 0)
      {
	n = sscanf(sp, "%s ", cmd);   //TODO use better sscanf
	if(0)in_snprintf(in, NULL
		    ," message received [%s] ->"
		    " n %d cmd [%s]\n"
		    , sp //&in->inbuf[in->inptr]
		    , n, cmd );
	in->instate = STATE_IN_NORM;
	run_new_hand(in->cmdid
		     , in->fd
		     , &inbf->outbuf[inbf->outptr]
		     , in->cmdbytes);
	//run_str_in(in, sp, cmd);
	rsize = in->cmdbytes;
	in->cmdbytes = 0;
	    
	if(g_debug)
	  {
	    printf(" %s rc %d n %d cmd [%s] tosend %d cmdid [%s]\n"
		   , __FUNCTION__
		   , rc, n, cmd, tosend
		   , in->cmdid ? in->cmdid :"no id"
		   );
	  }
	// TODO consume just the current cmd
	inbf->outptr += rsize;
	if(g_debug)
	  {
	    printf(" %s reset buffers rsize %d ptr/len %d/%d\n"
		   , __FUNCTION__
		   , rsize
		   , inbf->outptr
		   , inbf->outlen
		   );
	  }
	if(in->cmdbytes > 0)
	  {
	    printf("%s >>>>>reply still needs %d bytes\n"
		   , __FUNCTION__
		   , in->cmdlen );
	  }
	
	if(g_quit_one)
	  {
	    if(g_debug)
	      {
		printf("%s >>>>>forcing quit in->fd %d\n"
		       , __FUNCTION__
		       , in->fd
		       );
	      }
	    len = 0;
	    
	    if(in->fd>0) close(in->fd);
	    // Allow the next read to reset in->fd = -1;
	  }
      }
    return len;
}

void url_decode(char* src, char* dest, int max) {
    char *p = src;
    char code[3] = { 0 };
    while(*p && --max) {
        if(*p == '%') {
            memcpy(code, ++p, 2);
            *dest++ = (char)strtoul(code, NULL, 16);
            p += 2;
        } else {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}
				     
int send_html_head(struct iosock *in, char *msg)
{
  int len = 0;
  len = in_snprintf(in, NULL, "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html\r\n\r\n"
		    "<html><head><style>"
		    "body{font-family: monospace; font-size: 13px;}"
		    "td {padding: 1.5px 6px;}"
		    "</style></head><body>\n ");
  return len;
}

int send_html_tail(struct iosock *in, char *msg)
{
  int len;
  len = in_snprintf(in, NULL, "</body></html>");
 
  return len;
}

int run_str_http(struct iosock *in, char *sp, char *cmd, char *uri, char *vers)
{
  //struct space *space=NULL;
  //struct space *attr=NULL;
  int rc;
  struct iobuf *inbf;  // input buffer
  struct space *sp1;
  
  inbf = in->inbuf;
  rc = inbf->outlen - inbf->outptr;
  if(1)printf(" >>> %s >>>> rc %d cmd [%s] sp [%s]\n"
	      , __FUNCTION__
	      , rc
	      , cmd
	      , sp
	      );
  
  if(0)printf(" %s sp0 [%x] \n"
	      , __FUNCTION__
	      , sp[0]
	      );
  if(0)printf(" %s sp0 [%x] in %p \n"
	      , __FUNCTION__
	      , sp[0]
	      , in
	      );
      
  if(0)printf(" %s sp0 [%x] in->hidx %d \n"
	      , __FUNCTION__
	      , sp[0]
	      , in->hidx
	      );
      if(in->hidx >= 0)
	{
	  if(0)printf(" %s sp0 [%x] in->hidx %d %p\n"
		      , __FUNCTION__
		      , sp[0]
		      , in->hidx
		      , g_spaces[in->hidx]
		      );
	}
  
  if((sp[0] == 0xd) || (sp[0] == 0xa))
    {
      sp1 = NULL;
      if(in->hidx >= 0)
	{
	  sp1 = g_spaces[in->hidx];
	}
      if(sp1)
	{
	  printf(" %s start of data from 0x%x 0x%x hlen %d hidx %d name [%s]\n"
		 , __FUNCTION__
		 , sp[0]
		 , sp[1]
		 , in->hlen
		 , in->hidx
		 , (in->hidx >= 0)?g_spaces[in->hidx]->name:"not found"
		 );
	  
	  //if (sp1->value) free(sp1->value);
	  //sp1->value = malloc(in->hlen+1);
	  //memcpy(sp1->value,&sp[2],in->hlen);
	  //sp1->value[in->hlen] = 0;
	}
      in->cmdlen = in->hlen;
      in->cmdbytes = in->hlen;
      inbf->outptr += (in->hlen +2);
      // TODO find space referenced and set data
      //len = inbf->outlen - inbf->outptr;

    }
  else
    {
      rc = run_new_gcmd (cmd, &g_space_list, sp, in);
      rc = run_new_hcmd (cmd, &g_space_list, sp, in);
    }
  return 0;
}
// scan for double terminators from outptr to outlen 
int handle_input_norm(struct iosock *in)
{
    int rc=0;
    int len;
    int tlen;
    int n;
    char cmd[1024];
    char uri[1024];
    char vers[1024];
    char *sp;
    char savch;
    //char buf[1024];
    //char *bres;
    //int lres;
    //char *bufp;
    struct iobuf *inbf;  // input buffer
    //struct iobuf *oubf;  // input buffer
    //int rsize;
    int tosend;

    inbf = in->inbuf;
    sp = &inbf->outbuf[inbf->outptr];
    len = inbf->outlen - inbf->outptr;
    tlen = find_cmd_term(in, len, 0 /*,in->tlen*/);
    if(tlen == 1)
      in->tlen = 1;
    else
      in->tlen = 0;
    //if tlen == 1 we found one terminator
    // the next char may also be a terminator
    savch = 0;
    if(0 &&(tlen > 1))
      {
	savch = sp[tlen-1];
	if((savch == 0x0a) || (savch == 0x0d))
	  {
	    sp[tlen-1]=0;
	  }
      }
    if(g_debug)
      {
	printf("%s read len %d tlen %d sp[] %x outptr/len %d/%d\n==>sp [%s] \n"
	       , __FUNCTION__
	       , len
	       , tlen
	       , sp[tlen-1]
	       , inbf->outptr
	       , inbf->outlen
	       , sp
	       );

      }
    if(tlen > 1)
      {
	sp = &inbf->outbuf[inbf->outptr];
	cmd[0]=0;
	uri[0]=0;
	vers[0]=0;
	n = sscanf(sp, "%s %s %s", cmd, uri, vers);   //TODO use better sscanf
	//if(1)in_snprintf(in, NULL
	printf(		 " %s message received [%s] ->"
			 " n %d cmd [%s] uri [%s] vers [%s]\n"
			 , __FUNCTION__
			 , sp //&in->inbuf[in->inptr]
			 , n, cmd, uri, vers );
	if(g_debug)
	  {
	    printf(" %s oubuf_list #1 %p item %p buf %p\n"
		   , __FUNCTION__
		   , in->oubuf_list
		   , in->ouitem
		   , in->oubuf
		   );
	  }

	in->hproto = 0;  // Default
	if(strstr(vers,"HTTP/"))
	  {
	    in->instate = STATE_IN_HTTP;
	    in->hproto = 1;
	  }
	if (in->instate == STATE_IN_HTTP)
	  {
	    run_str_http(in, sp, cmd, uri, vers);
	  }
	else
	  {
	    if((savch == 0x0a) || (savch == 0x0d))
	      {
		sp[tlen-1]=savch;
		savch = 0;
	      }
	    run_str_in(in, sp, cmd);

	  }
	if((savch == 0x0a) || (savch == 0x0d))
	  {
	    sp[tlen-1]=savch;
	  }
	if(g_debug)
	  {
	    printf(" %s oubuf_list #2 list %p item  %p buf %p\n"
		   , __FUNCTION__
		   , in->oubuf_list
		   , in->ouitem
		   , in->oubuf
		   );
	  }
	tosend = count_iob_bytes(&in->oubuf_list);
	if(g_debug)
	  {
	    printf(" ... rc %d n %d cmd [%s] tlen %d tosend %d \n"
		   , rc, n, cmd, tlen, tosend
		   );
	  }
	// TODO consume just the current cmd
	// flag the fact that we got more
	
	inbf->outptr += tlen;
	if(inbf->outptr < inbf->outlen)
	  rc  = 1;
	if(g_debug)
	  {
	    printf(" reset buffers tlen = %d ptr/len %d/%d more %d\n"
		   , tlen
		   , inbf->outptr
		   , inbf->outlen
		   , rc
		   );
	  }
	if (inbf->outptr == inbf->outlen)
	  {
	    inbf->outptr = 0;
	    inbf->outlen = 0;
	    printf(" %s reset buffers tlen = %d ptr/len %d/%d\n"
		   , __FUNCTION__
		   , tlen
		   , inbf->outptr
		   , inbf->outlen
		   );
	  }
      }
    return rc;
}

// A bit complicated here
// we have an input
// we could have one or more commands in the input
// If we get the CMD command then its a bit simpler in tht we now have a
// fixed number of bytes to process before the next input
// We are confused between getting an input and processing a buffer
// once we get an input untill we can process no more
//

int g_def_input_len = 4096;

int handle_input(struct iosock *in)
{
  int more = 1;
  int len = 0;
  int rsize;
  char *sp;
  struct iobuf *inbf=NULL;  // input buffer
  struct list * item;  
  if (in->inbuf_list == NULL)
    {
      item = new_iobuf_item(g_def_input_len);
      in->inbuf_list = item;
      in->initem = item;
      in->inbuf = item->data;
      inbf = item->data;
    }
  // this is OK
  item = in->inbuf_list; 
  inbf = item->data;

  
  rsize = inbf->outsize-inbf->outlen;  // get remainig size
  printf(" ====>%s rsize %d inbf %p\n", __FUNCTION__, rsize, inbf);//->outlen);
  if(rsize == 0)
    {
      // oops we need to get another buffer
      item = new_iobuf_item(g_def_input_len);
      
      in->initem = item;
      in->inbuf = item->data;
      inbf = item->data;
      inbf->outlen = 0;
      inbf->outptr = 0;
      
      rsize = inbf->outsize-inbf->outlen;  // get remainig size
      printf(" ====>%s NEW rsize %d inbf %p\n", __FUNCTION__, rsize, inbf);//->outlen);
      push_list(&in->inbuf_list, item);
      
    }
  sp = &inbf->outbuf[inbf->outlen];

  len = read(in->fd, sp, rsize);
  
  if(len > 0)
    {
      sp[len] = 0;
      inbf->outlen += len;
      printf(" %s rsize %d len %d inbf->outptr/len %d/%d\n"
	     , __FUNCTION__, rsize, len, inbf->outptr, inbf->outlen);

      while(more)
	{
	  if(g_debug)
	    {
	      printf(" %s running more %d\n"
		     , __FUNCTION__
		     , more
		     );
	    }
	  if (in->instate == STATE_IN_CMD)
	    {
	      more = handle_input_cmd(in);
	    }
	  else if (in->instate == STATE_IN_REP)
	    {
	      more = handle_input_rep(in);
	      if(more == 0)
		len = 0;
	    }
	  //else if (in->instate == STATE_IN_HTTP)
	  //{
	  //  more = handle_input_http(in);
	  //  if(more == 0)
	  //	len = 0;
	  //}
	  else
	    {
	      more = handle_input_norm(in);
	    }
	  if(g_debug)
	    {
	      printf(" %s done more %d\n"
		     , __FUNCTION__
		     , more
		     );
	    }
	}
      
    }
  return len;
}

// DONE
// Wen we overflow the initial list we
// do not send the first list
// we refer to in->ouitem but when we send we should start with in->oubuf_list
// ALSO when in_snprintf  needs more room is should push the old buffer
// in fact is should push any new buffer
// onto in->oubuf_list
// we may have an in progress iobuf
// so we also need to keep the item we pulled
int handle_output(struct iosock *in)
{
  int rc = 0;
  struct iobuf *iob = NULL;
  struct list *item=NULL;
  char *sp;
  int len=0;
  //int bcount = 0;
  // iobway
 try_again:
  if(in->outbptr == in->outblen)
    {
      printf("%s >>> no data to send\n", __FUNCTION__);
      in->outbptr = 0;
      in->outblen = 0;
      return 0; // no data to send
    }
  in->ouitem = in->oubuf_list;
  if(!in->ouitem)
    {
      in->outbptr = 0;
      in->outblen = 0;
      printf("%s >>> no list queued\n", __FUNCTION__);
      return 0; // no data to send
    }
  item = in->ouitem;
  in->oubuf = item->data;
  iob = item->data;
  // for now just send one iob at a time
  if (iob)
    {
      len = iob->outlen - iob->outptr;
    }
  if(len == 0)
    {
      iob->outlen = 0;
      iob->outptr = 0;
      pop_list(&in->oubuf_list, item);
      push_list(&g_iob_list, item);
      goto try_again;
    }

  item = in->ouitem;  // keep it till we run out of data
  iob = item->data;
  sp = &iob->outbuf[iob->outptr];
  //len = iob->outlen -iob->outptr;
  //iob = pull_iob(&in->iobuf, &sp, &len);
  if(g_debug)
    printf(" %s running the new way item %p iob %p len %d sp [%s]\n"
	   , __FUNCTION__, item, iob, len, sp);

  rc = write(in->fd, sp, len);
  if(rc <= 0)
    {
      // shutdown iobuffer
      // TODO unload iobs
      in->outbptr = 0;
      in->outblen = 0;
      iob->outlen = 0;
      iob->outptr = 0;
      rc = -1;
    }
  if(rc >0)
    {
      in->outbptr += rc;
      iob->outptr+=rc;      
      if (in->outbptr == in->outblen)
	{
	  in->outbptr = 0;
	  in->outblen = 0;
	  iob->outlen = 0;
	  iob->outptr = 0;
	  
	  if((in->hproto) && (in->oubuf_list == NULL))
	    {
	      printf(" sent the last buffer to fd %d\n",
		     in->fd);
	      in->hproto = 0;
	      shutdown(in->fd, SHUT_WR);
	      //  in->fd = -1;
	    }
	}
    }
  return rc;
}


// read stdin, listen_sock and all accept_socks
int poll_sock(int lsock)
{
    struct pollfd fds[NUM_SOCKS+2];
    int idx = 0;
    int i;
    int timeout = 1000;
    int ret;
    int n;
    //char str[1024];
    struct iosock *in = NULL;
    int rc = 1;    

    if(lsock>0)
      {
	fds[idx].fd = lsock;
	fds[idx].events = POLLIN;
	fds[idx].revents = 0;
	idx++;
      }
    
    for (i = 0; i< NUM_SOCKS; i++)
      {
	if (g_iosock[i].fd >= 0)
	  {
	    if(g_debug)
	      {
		printf(" setup fd i %d idx %d fd %d\n"
		       , i
		       , idx
		       , g_iosock[i].fd
		       );
	      }
	    
	    fds[idx].fd = g_iosock[i].fd;
	    fds[idx].events = POLLIN;
	    fds[idx].revents = 0;
	    if((g_iosock[i].nosend == 0)
	       && (g_iosock[i].outbptr != g_iosock[i].outblen))
	      {
		fds[idx].events |= POLLOUT;
	      }
	    idx++;
	  }
      }
    if(idx == 0) return -1;
    if(g_debug)printf("poll start idx %d lsock %d \n", idx, lsock);
    ret =  poll(fds, idx, timeout);
    if(g_debug)printf("poll done ret = %d idx %d\n", ret, idx);

    if(ret > 0) 
    {
      if(g_debug)printf("poll ret = %d idx %d\n", ret, idx);
	
	for( i = 0; i < idx; i++) 
	{
	  if(g_debug)
	    {
	      printf(" idx %d fd %d revents 0x%08x\n",i, fds[i].fd, fds[i].revents);
	    }
	    if (fds[i].revents & POLLOUT) 
	    {
		in = find_fd(fds[i].fd);
		if(in)
		{
		    n = handle_output(in);
		    if (n < 0) 
		    {
			printf("error writing (n=%d), closing fd %d \n"
			       ,n,fds[i].fd);
			close(in->fd);
			close_fds(in->fd);
			fds[i].revents = 0;
		    }
		}
	    }
	    if (fds[i].revents & POLLIN) 
	    {
	      if((lsock > 0) && (fds[i].fd == lsock))
		{ 
		    ret = accept_socket(lsock);
		    printf("accept ret = %d \n", ret);
		    
		}
		else
		{
		    in = find_fd(fds[i].fd);
		    if(in)
		    {
		        n = handle_input(in);

			if (n <= 0) 
			{
			  if(g_debug)
			    {
			      printf("error reading (n=%d), "
				     "closing fd %d in->fd %d\n"
				     , n
				     ,fds[i].fd
				     , in->fd
				     );
			    }
			  close(fds[i].fd);
			  close_fds(fds[i].fd);
			  fds[i].fd = -1;
			  in->fd = -1; 
			}
			else 
			{
			  if(g_debug)
			    {
			      printf("message len %d\n", n);
			    }			    
			}
		    }
		}
	    }
	}
    }
    return rc;
}
