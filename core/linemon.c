#include "linemon.h"

//TODO: Move all this to a nice struct...
int run;
char *tx_buf, *rx_buf;
FILE *tx_out, *rx_out;
int   tx_pos,  rx_pos;


void usage(char *cmd){
  printf("Usage: %s [user:[pass]@]host extn\"\n", cmd);
  exit(-1);
}

int main(int argc, char **argv){
  if(argc != 3) usage(argv[0]);
  pj_status_t pj_status;

  char *host, *user, *pass, *extn;
  char *ptr = strchr(argv[1],'@');
  if(ptr){*ptr=0;ptr++;host=ptr;
    user=argv[1];ptr=strchr(user,':');
    if(ptr){*ptr=0;ptr++;pass=ptr;}
    else pass=NULL;
  } else {
    host = argv[1];
    user = NULL;
    pass = NULL;
  } extn = argv[2];


  run = 0;
  sip_init();

  pjsua_acc_id acc = 0;
  if(user)
    acc = sip_register(host, user, pass);

  port_init();
  if(user)
    while(!run){sleep(1);}

  dial(acc, host, extn);

  while(run){sleep(1);}
  pjsua_destroy();
  close(tx_out);
  close(rx_out);
  return 0;
}
