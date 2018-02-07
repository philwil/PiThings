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
extern struct cmds h_cmds[];
extern struct space *g_space;
extern struct space *g_spaces[];
extern int g_space_idx;
extern int g_space_debug;

extern struct list *g_space_list;
extern struct list *g_conn_list;

int init_g_spaces(void)
{
  int i;
  for (i=0 ; i< NUM_IDX; i++)
    g_spaces[i]=NULL;
  return 0;

}
// split up multi/space/name
// look for children of the same name 
// return found name or new space object
//    sp1 = add_space(&g_space, "ADD uav1/motor1/speed");
// local nodes only
// bug error in reading from file
struct space *_add_space_in(struct list **root, char *name,
			    struct iosock *in)
{
  struct space *space=NULL;
  struct hmsg *hm;
  struct hmsg hmsg;
  int idx;

  if(in)
    {
      hm = in->hm;
    }
  else
    {
      init_hmsg(&hmsg);
      setup_hmsg(&hmsg, name);
      hm = &hmsg;
    }
  show_hmsg(hm);
  idx = add_hmsg_spaces(root, hm);
  if(idx >= 0)
    space = g_spaces[idx];
  if(!in)clean_hmsg(hm);
  return space;
}

struct space *add_space_in(struct list **root, char *name,
			    struct iosock *in)
{
  if(g_conn_list)
    {
      printf("%s adding [%s] to each connection\n", __FUNCTION__, name);
    }

  return _add_space_in(root, name, in);
}


int test_find_parents(void)
{
  int num=0;
  int rc=0;
  int i;
  struct space *slist[64];
  struct space *sp1 = add_space(&g_space_list, "ADD uav1/motor1/speed");
  for (i = 0 ; i < 64; i++)
    slist[i] = NULL;
  num = find_parents(sp1,slist,num,64);
  printf("%s sp1 [%s] parent %p rc %d num %d\n"
	 , __FUNCTION__
	 , sp1->name
	 , sp1->parent
	 , rc
	 , num
	 );
  if (num > 0)
    {
      for ( i = num-1; i >=0; i--)
	{
	  printf("%d [%s] ", i, slist[i]?slist[i]->name?slist[i]->name:"NONAME":"NONODE");
	}
      printf("\n");
      
    }
  return rc;
}

int find_parents(struct space* node, struct space **list, int idx, int max)
{
  int rc = 1;
  list[idx++] = node;
  if (node->parent)
    {
      rc += find_parents(node->parent, list, idx, max);
    }
  return rc ;
}


struct space *setup_space(char *name, struct space*parent)
{
  struct space *space = calloc(sizeof(struct space), 1);

  space->idx = g_space_idx++;
  space->parent = parent;

  space->name = strdup(name);

  space->attrs = NULL;
  space->class = NULL;
  space->clone = NULL;
  space->child = NULL;
  space->desc = NULL;
  space->value = NULL;
  space->node = NULL;
  space->group = NULL;

  space->onset = NULL;
  space->onget = NULL;

  space->type = SPACE_ROOT;
  if(space->idx < NUM_IDX)
    {
      g_spaces[space->idx] = space;
    }
  return space;
}

void free_space(struct space *space)
{
  //foreach
  //  sp += strlen(sp);
  struct list *slist;
  struct list *item;
  //  struct list *nitem;
  struct space *parent;
  parent = space->parent;
  if(parent)
    {
      slist = parent->child;
      item = slist;
      while (item)
	{
	  //child = (struct space *)slist->data;
	  if(item->data == space)
	    {
	      item->data = NULL;
	    }
	  item = item->next;
	  if (slist == item) 
	    item = NULL;
	}
    }
  //TODO free attrs
  //TODO free children
  // 
  g_spaces[space->idx] = NULL;
  free(space->name);
  free(space);

}

struct space *new_space(char *name , struct space *parent, struct list **root_space, struct node *node)
{
  struct space *space;
  struct list * item;
  printf(" new space [%s] root %p\n"
	 , name
	 , root_space
	 );
  space = setup_space(name, parent);

  if(node)
    {
      space->node = node;
    }
  if(root_space)
    {
      item = new_list(space);
      add_list(root_space, item);
    }
  return space;
}  

struct space *new_space_class(char *name , struct space *parent)
{
  //int i;
  struct space *space=NULL;
#if 0
  struct space *class;
  //  struct space *last_space;
  printf(" new space class [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  class = parent->class;
  space = setup_space(name, parent);
  if(class)
    {
	  last_space = class->prev;
	  last_space->next = space;
	  space->prev =  class->prev;
	  space->next =  class;
	  class->prev = space;
    }
  else
    {
      parent->class = space;
    }
#endif

  return space;
}  

#if 0
struct space *new_space_attr(char *name , struct space *parent)
{
  //int i;
  struct space *space=NULL;
#if 0
  struct space *attr;
  //  struct space *last_space;
  printf(" new space attr [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  attr = parent->attr;
  space = setup_space(name, parent);


  if(attr)
    {
	  last_space = attr->prev;
	  last_space->next = space;
	  space->prev =  attr->prev;
	  space->next =  attr;
	  attr->prev = space;
    }
  else
    {
      parent->attr = space;
    }
#endif
  return space;
}  
#endif

#if 0

struct space *new_space_attr_str(char *name, struct space *parent, char *val)
{

  struct space *space = NULL;
  printf(" new space attr_str [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );

  space = new_space_attr(name, parent);
  if(space)
    {
      space->cval = val;
      space->type = SPACE_STR;

    }
  return space;
}

struct space *new_space_attr_float(char *name, struct space *parent, float val)
{
  struct space *space = NULL;
  printf(" new space attr_float [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  space = new_space_attr(name, parent);
  if(space)
    {
      space->fval = val;
      space->type = SPACE_FLOAT;

    }
  return space;
}

struct space *new_space_attr_int(char *name, struct space *parent, int val)
{

  struct space *space = NULL;
  printf(" new space attr_int [%s] parent [%s]\n"
	 , name
	 , parent?parent->name:"no parent"
	 );
  space = new_space_attr(name, parent);
  if(space)
    {
      space->ival = val;
      space->type = SPACE_INT;

    }
  return space;
}
#endif

int show_space_class(struct iosock *in,struct space *base, int indent)
{
  char atname[128];
  int len=0;
  snprintf(atname, sizeof(atname),"  %s Class", base->name); 
  if(base->class)
  {
    // len = show_spaces(in, base->class, atname, indent);
  }
  return len;
}
#if 0
int show_space_attr(struct iosock *in,struct space *base, int indent)
{
  char atname[128];
  int len=0;
  snprintf(atname, sizeof(atname),"  %s Attr", base->name); 
  if(base->attr)
  {
    //len = show_spaces(in,base->attr, atname, indent);
  }
  return len;
}
#endif

int show_space(struct iosock *in, struct space*base, int indent)
{
  int rc=-1;

  if(!base)return rc;
  if(!in)return rc;
 
  rc =  indent;
  while(rc--)
    {
	printf(" ");
    }
  printf(" %p space %03d name [%s] node [%p]\n"
	 , base
	 , base->idx
	 , base->name
	 , base->node
	 //, base->next->name
	 //, base->prev->name
	 );
  //  if(0)  show_space_attr(in,base, indent+3);

  if(0)show_space_class(in,base, indent+5);
  return rc;
}


int show_spaces(struct iosock *in, struct list **list, char *desc, int indent)
{
  struct space *start;
  struct list *slist = *list;
  struct list *ilist = *list;
  int rc = 0;
  in_snprintf(in, NULL, "spaces ... %s\n", desc ? desc:" ");

  while (ilist)
    {
      start =(struct space *)ilist->data;
      rc = show_space(in, start, indent);
      if(start->child) 
	{
	  rc =  show_spaces(in, &start->child,"child",indent+2);
	}
      if(ilist->next != slist)
	ilist=ilist->next;
      else
	ilist =  NULL;
    }
  return rc;
}

int show_space_new(struct iosock *in, struct list *list,  char *desc, int len, char *bdesc)
{
  struct space *space=NULL;
  struct list *clist=NULL;
  struct list *slist=NULL;
  int ret;
  int rc=-1;
  char *sp = desc;
  int slen = len;

  if(!list)return rc;
  if(!desc)return rc;
  if(len == 0)return rc;
  //  sp += strlen(desc);
  space = (struct space *)list->data;
  clist = space->child;
  if(clist == NULL)
    {
      snprintf(sp, slen,
	       "/%s => %d"
	       , space->name
	       , space->idx
	       );
      if(in)in_snprintf(in, NULL, ">>>>[%s]\n", bdesc);
      if(g_space_debug)
	printf(" >> [%s] %s %d %d\n"
	       , bdesc
	       , space->name
	       , space->idx
	       , space->ridx
	       );
      slen = strlen(sp);

      return slen;
    }
  else
    {
      snprintf(sp, slen,
	       "/%s"
	       , space->name
	       );
      if(g_space_debug)
	printf("    run >> [%s]\n", bdesc);
    }

  // foreach child do the same
  slen -= strlen(sp);
  sp += strlen(sp);
  //  sp += strlen(sp);
  slist = clist;
  while (slist)
    {
      //child = (struct space *)slist->data;

      slen -= show_space_new(in, slist, sp, slen, bdesc);
      slist = slist->next;
      if (slist == clist) 
	slist = NULL;
    }
  //  ret =  strlen(buf);
  ret =  slen;
  return ret;
}

int show_spaces_new(struct iosock *in, struct list **listp, char *desc, int len, char *bdesc)
{
  int rc  = 0;
  struct list *ilist = NULL;
  struct list *slist = NULL;
  if(g_space_debug)
    printf("%s listp %p *listp %p\n", __FUNCTION__, listp , *listp);
  //struct space *xstart=NULL;
  ilist = *listp;
  slist = ilist;

  while (ilist)
    {
      rc = show_space_new(in, ilist, desc, len, bdesc);
      if(g_space_debug)
	printf(" dbg >>> [%s] rc %d\n", desc, rc);
      //if(in)in_snprintf(in, NULL, ">>>>[%s]\n", desc);
      //xstart = start->next;
      ilist = ilist->next;

      if(ilist == slist) 
	ilist =  NULL;
      //start =  NULL;
    }
  //printf(" base [%s] %d @ %p \n", base->name, base->idx, base);
  //if(xstart)
  //printf(" next [%s] %d @ %p \n", xstart->name, xstart->idx, xstart);

 return rc;
}


// reworking with lists
struct space *find_space_new(struct list **listp, char *name)
{
  struct hmsg * hm;
  struct hmsg hmsg;
  //use hmsg
  struct space *sp1=NULL;
  char *spv = NULL;
  int idx = 0;
  init_hmsg(&hmsg);
  setup_hmsg(&hmsg, name);
  hm = &hmsg;
  show_hmsg(hm);
  idx = find_hmsg_spaces(listp, &hmsg);
  if(idx >= 0)
    {
      sp1 = g_spaces[idx];
    }

  if(g_space_debug)
    {
      // end of name list we are done
      if(sp1)
	printf(" %s we found it seeking [%s] found [%s]\n"
	       , __FUNCTION__, spv,  sp1->name);
      else
	printf(" %s no luck seeking [%s]\n", __FUNCTION__, spv);
    }
  //free_stuff(idv, valv);
  // free_stuff(idx, valx);
  clean_hmsg(hm);
  return sp1;
}

int add_child(struct space *parent, struct space *child)
{
  struct list *item = new_list(child);
  push_list(&parent->child, item);
  child->parent = parent;
  return 0;
}

#if 0
int add_attr(struct space *parent, struct space *attr)
{
  struct list *item = new_list(attr);
  push_list(&parent->attr, item);
  attr->parent = parent;
  return 0;
}
#endif

int del_child(struct space *parent, struct space *child)
{
  struct list *item = new_list(child);
  del_list(&parent->child, item);
  child->parent = parent;
  return 0;
}

#if 0
int del_attr(struct space *parent, struct space *attr)
{
  struct list *item = new_list(attr);
  del_list(&parent->attr, item);
  attr->parent = parent;
  return 0;
}
#endif
#if 0
// create a copy of the space if any at name
//  base = find_space_new(base, name);
struct space *copy_space_new(struct space *base, char *new_name)
{
  struct space *start;
  struct space *sp1;
  struct space *sp2;
  struct space *space= NULL;

  space = setup_space(new_name, NULL);

  start = base->child;
  sp1 = base->child;
  while(sp1)
    {
      sp2 = copy_space_new(sp1, sp1->name);
      add_child(space, sp2);
      sp1=sp1->next;
      if (sp1 == start)
	sp1 = NULL;
    }
  return space;
}
#endif

// find a space given a name
struct space *find_space(struct list**list, char *name)
{
  struct list *ilist;
  struct list *slist;
  struct space *space;

  if(!list || !*list || !name)
    {
      return NULL;
    }
  slist = *list;
  ilist = *list;
  //if(g_debug)
  // printf(" >>> find space [%s] base name [%s] ... \n", name
  //	 ,base?base->name ? base->name:"No name":"No base");
  while (ilist)
    {
      space = (struct space *)ilist->data;
      if(g_space_debug)
	printf(" %s looking at space [%s]\n", __FUNCTION__, space->name);
      if(strcmp(space->name, name)==0)
	{
	  break;
	}
      
      if(ilist->next != slist)
	ilist=ilist->next;
      else
	{
	  ilist =  NULL;
	  space = NULL;
	}
    }
  
  if(g_debug)
    {
      printf(" >>> find space name [%s] space %p... \n", name, space);
    }
  return space;
}

/*
before any s0->p = s0 s0->n=s0
after add s1 s0->p = s1 s0->n = s1 s1->p = s0 s1->n=s0
after add s2 s0->p = s2 s0->n = s1 s1->p = s0 s1->n=s2 s2->p = s1 s2->n=s0
 */
int insert_space(struct list **parent, struct space *space)
{
  struct list * item = new_list(space);
  push_list(parent, item);

  return 0;
}
struct space *show_space_in(struct list **root, char *name, struct iosock *in)
{
  struct hmsg * hm;
  struct hmsg hmsg;
  int idx=-1;
  struct space *sp1=NULL;
  char sbuf[4096];
  struct list **base;

  if(in)
    {
      hm = in->hm;
    }
  else
    {
      init_hmsg(&hmsg);
      setup_hmsg(&hmsg, name);
      hm = &hmsg;
    }
  //init_hmsg(&hmsg);
  //setup_hmsg(&hmsg, name);

  //hm = in->hm;
  show_hmsg(hm);
  //idx = find_hmsg_spaces(root, &hmsg);
  if(idx >= 0)
    {
      sp1 = g_spaces[idx];
    }

  base = &g_space_list;
  if(sp1)
  {
    printf(" %s got sp1 name [%s] child %p\n", __FUNCTION__
	   , sp1->name
	   , sp1->child
	   );
  //  spb = &sp1;
    if(sp1->child)
      base = &sp1->child;
  }
  show_spaces_new(in, base, sbuf, 4096, sbuf);
  if(!in)clean_hmsg(hm);

  return NULL;
}

int set_space_value(struct space *sp1, char *spv, char *name)
{
  int rc = -1;
  if(sp1)
    {
      if(sp1->value) free(sp1->value);
      sp1->value = NULL;
      if(spv && (strlen(spv) > 0))
	sp1->value = strdup(spv);
      if(sp1->onset)
	sp1->onset(sp1, sp1->idx, name, spv);
      rc = 0;
    }
  return rc;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
int set_spacexx(struct list **list, char *name, char *value)
{
  int rc = -1;
  struct space *sp1 = NULL;
  char sname[3][128];  // TODO
  char * spv;

  sname[0][0]=0;
  sname[1][0]=0;
  sname[2][0]=0;
  rc = sscanf(name,"%s %s %s", sname[0], sname[1], sname[2]);
  sp1 = find_space_new(list, sname[1]);
  spv = sname[2];
  if(value)
    spv =  value;
  rc = set_space_value(sp1, spv, name);

  return rc;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
struct space *set_space_in(struct list **root, char *name, struct iosock *in)
{
  struct space *sp1=NULL;
  struct hmsg *hm;
  struct hmsg hmsg;
  int idx;
  char *spv=NULL;

  if(in)
    {
      hm = in->hm;
    }
  else
    {
      init_hmsg(&hmsg);
      setup_hmsg(&hmsg, name);
      hm = &hmsg;
    }
  //init_hmsg(&hmsg);
  //setup_hmsg(&hmsg, name);
  show_hmsg(hm);
  idx = find_hmsg_spaces(root, hm);
  if(idx >= 0)
    {
      sp1 = g_spaces[idx];
      str_replace(&sp1->value, NULL);
      if(hm->data)
	{
	  spv=hm->data;
	  hm->data = NULL;
	}
      //else if(hmsg.vers)
      //{
      //  spv=hmsg.vers;
      //  hmsg.vers = NULL;
      //}
      if(sp1)
	{
	  printf("%s [%s] old val [%s] \n"
		 , __FUNCTION__
		 , sp1->name ? sp1->name : " no name"
		 , sp1->value ? sp1->value : " not set"
		 );
	  printf("%s hm->data[%s] hm->vers[%s]\n"
		 , __FUNCTION__
		 , hm->data ? hm->data: "no data"
		 , hm->vers ? hm->vers : "no vers"
		 );
	  printf("%s spv [%s]\n"
		 , __FUNCTION__
		 , spv ? spv : "no spv"
		 );
	  
	  set_space_value(sp1, spv, name);
	}
      else
	{
	  printf("%s [%s] name not found\n"
		 , __FUNCTION__
		 , hm->url ? hm->url: " no url"
		 );
	}
      //rc = 0;
      if(in)in_snprintf(in,NULL,"OK SET %s to [%s]\n",sp1->name, sp1->value);
    }
  else
    {
      printf("%s [%s]  NOT FOUND\n"
	     , __FUNCTION__ 
	     , hm->url
	     );
      if(in)in_snprintf(in,NULL,"?? SET idx %d  not found \n", idx);//sp1->name);
    }

  if(!in)clean_hmsg(hm);
  return sp1;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
int set_space(struct list **list, char *name)
{
  set_space_in(list, name, NULL);
  return 0;
}

// TODO use struct attr in hmsg 
char* get_hmsg_attr(struct hmsg *hm, char *name)
{
  int i;
  //struct attr *attr = NULL;
  char * spa;
  for (i = 0 ; i < NUM_ATTRS; i++)
    {
      if(hm->attrs[i])
	{
	  spa = hm->attrs[i];
	  if (strstr(spa, name) == spa)
	    {
	      break;
	    }
	}
      else
	{
	  spa = NULL;
	  break;
	}
    }
  if(spa) 
    spa = &spa[strlen(name)];
  return spa;
}

// This is the GET/POST response
// We start an HTML acquire till the last blank line pair
// and any content.
// then we'll rereun the original command with the header decoded 
// char *get_space(struct space *base, char *name)
struct space *get_html_in(struct list **root, char *name, struct iosock *in)
{
  struct hmsg *hm;
  //struct attr* attr;
  char *spa;
  int rc = 0;
  int idx;
  //struct space *sppar=NULL;
  struct space *spr=NULL;
  struct space *sp1=NULL;
  char *sp_child;
  char *sp_root;
  char buf[2048];
  char sp_buf[2048];
  char sp_kids[2048];
  int sp_rlen;
  struct list *ritem = NULL;
  struct list *item=g_space_list;

  sp_child="<select name=\"forma\" onchange=\"location = this.value;\">"
    "<option value=\"gpio1\">gpio1</option>"
    "<option value=\"gpio2\">gpio2</option>"
    "<option value=\"gpio3\">gpio3</option>"
    "</select>";

  hm = in->hm;
  rc=hm->more;

  sp_root=sp_buf;

  sp_rlen = sizeof(sp_buf);
  snprintf(sp_root, sp_rlen
	   ,"%s"
	   ,"<select name=\"root\" onchange=\"location =/this.value;\">"
	   );
  sp_root = &sp_buf[strlen(sp_buf)];
  sp_rlen = sizeof(sp_buf)- strlen(sp_buf);
  // get root list
  while(foreach_item(&ritem,&item))
    {
      spr = item->data;
      snprintf(sp_root, sp_rlen
	       ,"<option value=\"%s\">%s</option>"
	       , spr->name
	       , spr->name
	       );
      sp_root = &sp_buf[strlen(sp_buf)];
      sp_rlen = sizeof(sp_buf)- strlen(sp_buf);
    }
  snprintf(sp_root, sp_rlen
	   ,"%s"
	   ,"</select>"
	   );
  
  idx = find_hmsg_spaces(root, hm);
  if(idx >= 0)
    {
      sp1 = g_spaces[idx];
    }

  if(sp1)
    {

      spa = get_hmsg_attr(hm,"value=");
      if(spa)
	{
	  str_replace(&sp1->value, spa);
	}

      sp_root=sp_kids;
      sp_rlen = sizeof(sp_kids);
      snprintf(sp_root, sp_rlen
	       ,"%s"
	       ,"<select name=\"kids\" onchange=\"location =this.value;\">"
	       );
      sp_root = &sp_kids[strlen(sp_kids)];
      sp_rlen = sizeof(sp_kids)- strlen(sp_kids);
      // get root list
      item = sp1->child;
      ritem = NULL;
      while(foreach_item(&ritem,&item))
	{
	  spr = item->data;
	  snprintf(sp_root, sp_rlen
	       ,"<option value=\"%s\">%s</option>"
		   , spr->name
		   , spr->name
		   );
	  sp_root = &sp_kids[strlen(sp_kids)];
	  sp_rlen = sizeof(sp_kids)- strlen(sp_kids);
	}
      snprintf(sp_root, sp_rlen
	       ,"%s"
	       ,"</select>"
	       );

      snprintf(buf, 2048, "%s 200 OK\r\n"
	       "Content-Type: text/html\r\n\r\n"
	       "<html><head>"
	       "</head>\n"
	       ,hm->vers);
      write(in->fd, buf, strlen(buf));
      snprintf(buf, 2048,
	       "<!DOCTYPE html>"
	       "<html>"
	       "<body>"
	       "<form action=\"%s\">"
	       "Root %s "
	       "Child %s "
	       "Variable %s"
	       "<input type=\"text\" name=\"value\" value=\"%s\">"
	       "<input type=\"submit\" value=\"Change\">"
	       "%s"
	       "</form><br><br>" 
	       "</body>"
	       "</html>"
	       , hm->url, sp_buf, sp_kids, sp1->name, sp1->value, sp_child);
	      //send_html_form(in, spv[1], sp1->name, sp1->value);
	      write(in->fd, buf, strlen(buf));
	      close(in->fd);
      if(1)printf(" %s http %d rc %d looking for [%s] [%s] found [%s]\n"
		  , __FUNCTION__
		  , rc
		  , hm->http
		  , hm->action
		  , hm->url
		  , sp1->name
	      );
    }
  else
    {
      if(1)printf(" %s http %d rc %d looking for [%s] [%s] NOT FOUND\n"
		  , __FUNCTION__
		  , rc
		  , hm->http
		  , hm->action
		  , hm->url
	      );
    }
  str_replace(&in->hcmd, hm->action);
  return NULL;
}


// This is the GET command
//char *get_space(struct space *base, char *name)
struct space *get_space_in(struct list **root, char *name, struct iosock *in)
{
  struct space *sp1=NULL;
  struct hmsg *hm;
  struct hmsg hmsg;
  int idx;
  if(in)
    {
      hm = in->hm;
    }
  else
    {
      init_hmsg(&hmsg);
      setup_hmsg(&hmsg, name);
      hm = &hmsg;
    }
  show_hmsg(hm);
  idx = find_hmsg_spaces(root, hm);
  if(idx >= 0)
    {
      sp1 = g_spaces[idx];
      if(in && sp1->onget)
	sp1->onget(sp1, sp1->idx, hm->url);

      printf("%s [%s] val [%s] \n"
	     , __FUNCTION__
	     , sp1->name
	     , sp1->value
	     );
      if(in)in_snprintf(in,NULL,"OK SEE %s to [%s]\n",sp1->name, sp1->value);
    }
  else
    {
      printf("%s [%s]  NOT FOUND\n"
	     , __FUNCTION__ 
	     , hm->url
	     );
      if(in)in_snprintf(in,NULL,"?? SEE [%s] not found \n", hm->url);
    }
  if(!in)clean_hmsg(hm);
  return sp1;
}

//rc  = set_space(g_space, "SET uav3/motor2/speed 3500");
char *get_space(struct list **root, char *name)
{
  char *ret =  NULL;
  struct space *sp0;
  //TODO
  //struct space *gbase = base;
  sp0 = get_space_in(root, name, NULL);
  if(sp0)
    ret = sp0->value;
  return ret;
}

