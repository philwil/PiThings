#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char evars[20][80]={"SERVER_SOFTWARE", "SERVER_NAME", "SERVER_PROTOCOL", 
                    "SERVER_PORT",
                    "GATEWAY_INTERFACE", "REQUEST_METHOD", 
                    "PATH_INFO", "PATH_TRANSLATED", "SCRIPT_NAME", 
                    "QUERY_STRING", 
                    "REMOTE_HOST", "REMOTE_ADDR", "REMOTE_USER", 
                    "REMOTE_IDENT",
                    "AUTH_TYPE", "CONTENT_TYPE", "CONTENT_LENGTH", 
                    "HTTP_ACCEPT", "HTTP_USER_AGENT", "HTTP_REFERER"};


int main(void) {
    int numvars=20;
    int i;
    char *sp;
    int content=0;
    char c;
    int rc;
    printf("Content-type: text/plain\n\n");
    sp = getenv("CONTENT_LENGTH");
    if (sp)
      content = atoi(sp);
    
    for (i=0;i<numvars;i++)
      {
	sp = getenv(evars[i]);
	printf("%s = %s\n", evars[i]
	       , sp?sp:"no val"
	       );
      }
    printf("content ... %d\n", content);
    while(content--)
      {
	rc =read(0,&c, 1);
	printf("%c",c);
	if(rc<=0) return 0;
      }
    printf("content done ... %d\n\n", content);
    return 0;
}
 
