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

extern struct cmds g_cmds[];
extern  struct cmds h_cmds[];

extern struct list *g_node_list;
extern struct list *g_conn_list;
extern struct list *g_space_list;

extern char *g_myname;
extern char *g_myaddr;
extern int g_port_no;
extern struct space *g_spaces[];

int show_hvals(char *key, char *hvals[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(hvals[i])
	{
	  printf("==== %s @%d [%s]\n", key, i, hvals[i]);
	}
    }
  return 0;
}

int show_hmsg(struct hmsg *hm)
{
  printf("=======Show hmsg @%p=======\n",hm);
  printf("===   sp [%s]\n",hm->sp);
  printf("===action[%s]\n",hm->action);
  printf("===   url[%s]\n",hm->url);
  printf("===  vers[%s]\n",hm->vers);
  printf("==qstring[%s]\n",hm->qstring);
  printf("==   dlen[%d]\n",hm->dlen);
  printf("==   slen[%d]\n",hm->slen);
  printf("==   hlen[%d]\n",hm->hlen);
  printf("==   more[%d]\n",hm->more);
  printf("==   http[%d]\n",hm->http);
  printf("==   data[%s]\n",hm->data);
  show_hvals("    hvals1", hm->hvals1, NUM_HVALS1);
  show_hvals("    hvals2", hm->hvals2, NUM_HVALS2);
  show_hvals("    attrs",  hm->attrs,  NUM_ATTRS);
  show_hvals("    snames", hm->snames, NUM_SPACES);
  return 0;

}

int init_hmsg(struct hmsg *hm)
{
  int i;
  hm->sp = NULL;
  hm->url = NULL;
  hm->action = NULL;
  hm->qstring = NULL;
  hm->data = NULL;
  hm->dlen = 0;
  hm->more = 0;
  for (i=0; i<NUM_HVALS1; i++)
    {
      hm->hvals1[i] = NULL;
    }
  for (i=0; i<NUM_HVALS2; i++)
    {
      hm->hvals2[i] = NULL;
    }
  for (i=0; i<NUM_ATTRS; i++)
    {
      hm->attrs[i] = NULL;
    }
  for (i=0; i<NUM_SPACES; i++)
    {
      hm->snames[i] = NULL;
    }
  for (i=0; i<NUM_SPACES; i++)
    {
      hm->spaces[i] = NULL;
    }
  return 0;

}

void clean_sp(char **spp)
{
  char *sp;
  if(spp)
    {
      sp = *spp;
      if(sp)free(sp);
      *spp = NULL;
    }
}

void clean_vals(char *vals[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(vals[i]) free(vals[i]);
      vals[i] = NULL;
    }
}

void clean_spaces(struct space *spaces[], int num)
{
  int i;
  for (i=0; i<num; i++)
    {
      if(spaces[i]) free_space(spaces[i]);
      spaces[i] = NULL;
    }
}

int clean_hmsg(struct hmsg *hm)
{
  clean_sp(&hm->sp);
  clean_sp(&hm->url);
  clean_sp(&hm->action);
  clean_sp(&hm->qstring);
  clean_sp(&hm->vers);
  clean_sp(&hm->data);
  hm->dlen = 0;
  clean_vals(hm->hvals1, NUM_HVALS1);
  clean_vals(hm->hvals2, NUM_HVALS2);
  clean_vals(hm->attrs,  NUM_ATTRS);
  clean_vals(hm->snames, NUM_SPACES);

  return 0;
}

char *get_valx(char *valx[], int num)
{
  char *sp;
  sp = valx[num];
  valx[num] =  NULL;
  return sp;
}

int null_valx(char *valx[], int num)
{
  while(num--)
    {
      if(valx[num]) 
	{
	  //free (valx[num]);
	  valx[num] = NULL;
	}
    }
  return num;
}

int clean_valx(char *valx[], int num)
{
  while(num--)
    {
      if(valx[num]) 
	{
	  free (valx[num]);
	  valx[num] = NULL;
	}
    }
  return num;
}

int decode_content_length(char *hsp)
{
  char tmp[128];
  char *tsp;
  int dlen;

  hsp +=strlen("Content-Length:");
  tsp = tmp;
  tsp[0]=0;
  while (*hsp && (*hsp == ' ') && (*hsp != '\n') )hsp++; 
  //printf(" After no Space sp [%s]\n",hsp);
  while (*hsp && (*hsp != '\n') )*tsp++ = *hsp++;
  *tsp = 0;
  //printf(" After decode tmp [%s]\n",tmp);
  dlen = atoi(tmp);
  //printf(" After decode dlen [%d]\n",dlen);
  return dlen; 
}

char *find_data(char *hsp)
{
  char *sp = NULL;
  sp = strstr(hsp,"\r\n\r\n");
  if(sp)sp  += strlen("\r\n\r\n");
  if (!sp)sp = strstr(hsp,"\n\r\n\r");
  if(sp)sp  += strlen("\n\r\n\r");
  if (!sp)sp = strstr(hsp,"\n\n");
  if(sp)sp  += strlen("\n\n");
  if(sp)
    {
      printf(" After %s  [%x] [%x]\n",__FUNCTION__, sp[0], sp[1]);
    }
  else
    {
      printf(" After %s  data not found\n",__FUNCTION__);
    }
  return sp;
}

// things to detect
// 1. get /n but no http in vers slen = string length 
// 2. get /n but  http in vers slen = 0 http = 1 
// 3. get /n/n but no http no dlen in vers slen = command length 
// 4. get /n/n http = 1 no dlen in vers slen = command length 
// 5. get /n/n http = 1 dlen>0 slen = command length + dlen 
// more = 1 if strlen is less that slen
char *setup_hmsg_len(struct hmsg *hm, char *insp, int len)
{
  int idx=-1;
  char *sp=NULL;
  char *spx=NULL;
  /*
  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: xxx\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n\r"
    "\n\r"
    "thisistherealnumber\n";
  */
  hm->hlen = len;
  hm->slen = 0;

  if (!strchr(insp,'\n'))
    {
      hm->more = 1;
      return NULL;
    } 
  hm->more = 0;
  str_replace(&hm->sp,insp);
  idx = parse_stuff(' ', 4 , (char **)hm->hvals1, hm->sp, 0);
  if(1)printf(" %s parse_stuff 1  idx %d got [%s] [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->hvals1[0]
	      , hm->hvals1[1]
	      , hm->hvals1[2]
	      , hm->hvals1[3]
	      );
  if(idx > 0)
    {
      hm->action = get_valx(hm->hvals1, 0);  // POST GET etc
      hm->slen = (strstr(hm->sp,hm->action) - hm->sp) +strlen(hm->action);
    }
  if(idx > 1)
    {
      sp         = get_valx(hm->hvals1, 1);  // uri + query
      hm->slen = (strstr(&hm->sp[hm->slen],sp) - hm->sp) +strlen(sp);
    }
  if(idx > 2)
    {
      hm->vers   = get_valx(hm->hvals1, 2);  // VERS
      hm->slen = (strstr(&hm->sp[hm->slen],hm->vers) - hm->sp)+strlen(hm->vers);
    }

  if(hm->vers && (strstr(hm->vers,"HTTP")))
    {
      hm->http = 1;
    }
  else
    {
      hm->http = 0;
      if(hm->vers)
	hm->data   = strdup(hm->vers);  // VERS
      if(hm->data)
	{
	hm->dlen = strlen(hm->data);
	hm->slen += hm->dlen;
	}
      else
	hm->dlen = 0;

    }
  if(sp)
    {
      idx = parse_stuff('?', 4 , (char **)hm->hvals2, sp,'?');
      if(1)printf(" %s parse_stuff 2 idx %d got [%s] [%s]\n"
		  , __FUNCTION__
		  , idx
		  , hm->hvals2[0]
		  , hm->hvals2[1]
		  );

      hm->url = get_valx(hm->hvals2, 0);  //url
      hm->qstring = get_valx(hm->hvals2, 1);  //query
      sp = hm->qstring;
      if(sp)
	{
	  idx = parse_stuff('&', 8 , (char **)hm->attrs, sp,' ');
	  if(0)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
		      , __FUNCTION__
		      , idx
		      , hm->attrs[0]
		      , hm->attrs[1]
		      , hm->attrs[2]
		      , hm->attrs[3]
		      );
	}
    }
  sp = hm->url;
  if(sp)
    {
      idx = parse_stuff('/', 8 , (char **)hm->snames, sp,' ');
      if(0)printf(" %s parse_stuff idx %d got [%s] [%s] [%s] [%s]\n"
	      , __FUNCTION__
	      , idx
	      , hm->snames[0]
	      , hm->snames[1]
	      , hm->snames[2]
	      , hm->snames[3]
	      );
    }
  // "Content-Length: 19\n"
  //hm->dlen = 0;
  //hm->data = NULL;
  sp = strstr(insp, "Content-Length:");
  if(sp)
    {
      //printf("decode content len from [%s]\n",sp);
      hm->dlen = decode_content_length(sp);
    }
  if(!hm->http)  if(!spx) spx = strstr(hm->sp,"\n");
  if(!spx)spx = strstr(hm->sp,"\r\n\r\n");
  if(!spx) spx = strstr(hm->sp,"\n\r\n\r");
  if(!spx) spx = strstr(hm->sp,"\n\n");
  if(!spx) spx = strstr(hm->sp,"\n");
  printf(" %s looking for slen spx =[%s]\n", __FUNCTION__, spx);
  if(spx)
    {
      hm->slen = spx - hm->sp;
      if(hm->http)hm->slen += hm->dlen;

    }
  sp = NULL;
  if(hm->dlen && hm->http)
    sp = find_data(hm->sp);
  if(sp)
    {
      hm->data=strdup(sp);
      hm->data[hm->dlen]=0;
    }
  hm->more = 0;

  if(len < hm->slen)
    hm->more = 1;

  return sp;
}


char *setup_hmsg(struct hmsg *hm, char *insp)
{
  return setup_hmsg_len(hm, insp, strlen(insp));
}

struct hmsg*new_hmsg(void)
{
  struct hmsg *hm;
  hm = malloc(sizeof(struct hmsg));
  init_hmsg(hm);
  return hm;
}

int test_hmsg(void)
{
  int idx = 0;
  char *sp;
  struct hmsg hmsg;

  sp ="POST /pine1/gpios/gpio1?value=somenewvalue&fum=2345 HTTP/1.1\n"
    "User-Agent: curl/7.26.0\n"
    "Host: 127.0.0.1:5432\n"
    "Accept: */*\n"
    "Content-Length: 19\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "\n"
    "thisistherealnumber\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="GET /pine1/gpios/gpio1\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="GET /pine1/gpios/gpio1?value=1\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  clean_hmsg(&hmsg);

  sp ="POST /pine1/gpios/gpio21?value=1&action=set\n Content-Length: 6\n\n123456789\n\n";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  //clean_hmsg(&hmsg);

  idx = find_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);
  idx = add_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);
  idx = find_hmsg_spaces(&g_space_list,&hmsg);
  printf("idx val = %d\n", idx);
  clean_hmsg(&hmsg);

  //sp ="POST /pine1/gpios/gpio21?value=1&action=set\n Content-Length: 6\n\n123456789\n\n";

  sp ="POST /pine1/gpios/gpio21?value=1&action=set SOMEVAL1";
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  //clean_hmsg(&hmsg);
  sp ="POST /pine1/gpios/gpio21?value=1&action=set  SOMEVAL2\n";
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  sp ="POST /pine1/gpios/gpio21?value=1&action=set   SOMEVAL3\n\n";
  setup_hmsg(&hmsg, sp);
  show_hmsg(&hmsg);
  //clean_hmsg(&hmsg);
  //sp ="POST /pine1/gpios/gpio21?value=1&action=set  SOMEVAL\n";

  return 0;
}

int do_hmsg_spaces(struct list **root, struct hmsg *hm, int add)
{
  int i = -1;
  struct space *sp1;
  struct space *sp2=NULL;
  struct list **sp_list;
  //struct list * item;;
  char *sp;
  int idx=-1;

  sp_list = root;

  for (i=0; i<NUM_SPACES; i++)
    {
      sp = hm->snames[i];
      //if (*sp == '/') sp++;
      printf(" looking for %d [%s]\n", i, sp?sp:"none");
      if(hm->snames[i] && (strlen(hm->snames[i]) > 0))
	{
	  sp = hm->snames[i];
	  //if (*sp == '/') sp++;
	  printf(" looking for %d [%s] ... ",i,  sp);

	  sp1 = find_space_name(sp_list, sp);
	  if(sp1)
	    {
	      printf(" found name [%s] idx %d\n", sp1->name, sp1->idx);
	      sp_list = &sp1->child;
	      hm->spaces[1] = sp1;
	      idx = sp1->idx;
	    }
	  else
	    {
	      if(add)
		{
		  printf(" .. adding [%s] !\n", sp);
		  sp1=new_space(sp,sp2,sp_list,NULL);
		  sp_list = &sp1->child;
		  idx = sp1->idx;
		}
	      else
		{
		  printf(" .. urgh !\n");
		  idx = -1;
		}
	    }
	  if(!add && (!sp1 || !sp_list))
	    {
	      printf(" List finised at index %d\n", i); 
	      break;
	    }
	}
      else
	{
	      printf(" List finised at index %d\n", --i); 
	      break;
	}
      sp2 = sp1;
    }
  if(add & (idx >= 0))
    {
      printf("setting attributes for [%s]\n", sp1->name);
      for (i = 0; i < NUM_ATTRS; i++)
	{

	  struct list *item;
	  struct attr *attr;
	  if(!hm->attrs[i]) break;
	  attr = find_space_attr(&sp1->attrs, hm->attrs[i]);
	  if(!attr)
	    {
	      attr =  new_space_attr(hm->attrs[i]);
	      item = new_list(attr);
	      add_list (&sp1->attrs, item);
	    }
	  else
	    {
	      replace_space_attr(attr,hm->attrs[i]);
	    }
	}
    }

  return idx;
}

struct attr*new_space_attr(char *name)
{
  struct attr *attr = NULL;
  int nlen = name - strchr(name,'=');
  attr=calloc(sizeof(struct attr), 1);
  attr->name =strndup(name, nlen);
  attr->nlen = nlen;
  attr->value = strdup(&name[nlen+1]);
  attr->vlen = strlen(attr->value);
  return attr;
}

struct attr*replace_space_attr(struct attr *attr, char *name)
{
  int nlen = name - strchr(name,'=');
  if(attr->vlen > strlen(&name[nlen+1]))
    {
      strcpy(attr->value, &name[nlen+1]);
    }
  else
    {
      free(attr->value);
      attr->value = strdup(&name[nlen+1]);
      attr->vlen = strlen(attr->value);
    }
  return attr;
}

struct attr*find_space_attr(struct list **root, char *name)
{
  struct attr *attr = NULL;
  struct list *item = *root;
  struct list *ritem = NULL;
  int nlen = name - strchr(name,'=');

  while (foreach_item(&ritem, &item))
    {
      if(item->data) 
	{
	  attr = item->data;

	  printf(" %s >>>> [%s] seeking [%s]\n",__FUNCTION__,attr->name, name);
	  if (strncmp(attr->name, name, nlen) == 0)
	    {
	      break;
	    }
	  else
	    {
	      attr = NULL;
	    }
	}
    }
  return attr;
}

int find_hmsg_spaces(struct list **root, struct hmsg *hm)
{
  return do_hmsg_spaces(root, hm, 0);
}

int add_hmsg_spaces(struct list **root, struct hmsg *hm)
{
  return do_hmsg_spaces(root, hm, 1);
}

int process_hmsg(struct hmsg *hmsg)
{
  //char *cmd;
  //char *action;
  return 0;
}


