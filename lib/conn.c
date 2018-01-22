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

extern struct list *g_conn_list;
extern int g_conn_debug;


//
int test_conn_list(void)
{
  struct list *item;
  printf(" test_conn_list\n");

  item = get_node_list(&g_conn_list,"127.0.0.1", 5432);
  push_list(&g_node_list, item);

  item = get_node_list(&g_conn_list,"127.0.0.1", 6000);
  push_list(&g_node_list, item);

  item = get_node_list(&g_conn_list,"127.0.0.1", 7000);
  push_list(&g_node_list, item);

  print_node_list(g_conn_list);
  remove_node_list(&g_conn_list);
  return 0;
}

