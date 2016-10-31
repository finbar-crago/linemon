#include <stdio.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>

#define BUFSIZE 8000*30

//TODO: Move all this to a nice struct...
int run;
char *tx_buf, *rx_buf;
FILE *tx_out, *rx_out;
int   tx_pos,  rx_pos;

#define CHECK(fn) puts(fn);

#define DIE(fn) \
  { printf("[%s:%d(%s)] !%s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__,#fn); \
    exit(-1); }

#define PJ(fn) pj_status = fn; if(pj_status != PJ_SUCCESS) DIE(fn)

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
  while(!run){sleep(1);}

  dial(acc, host, extn);

  while(run){sleep(1);}
  pjsua_destroy();
  close(tx_out);
  close(rx_out);
  return 0;
}


void on_reg_state(pjsua_acc_id acc_id){
  pjsua_acc_info acc_info;
  pjsua_acc_get_info(acc_id, &acc_info);
  printf("acc_info.status == %d\n", acc_info.status);
    if(acc_info.status == PJSIP_SC_OK)
      run = 1;
}

static void on_call_state(pjsua_call_id call_id, pjsip_event *e){
  PJ_UNUSED_ARG(e);
  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);
  if(ci.state == PJSIP_INV_STATE_CONNECTING){}
  if(ci.state == PJSIP_INV_STATE_DISCONNECTED){
    run = 0;
  }
}

static void on_call_media_state(pjsua_call_id call_id){
  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);
  if(ci.media_status == PJSUA_CALL_MEDIA_ACTIVE){
    // FIXME: This is lazy....
    pjsua_conf_connect(1, ci.conf_slot);
    pjsua_conf_connect(ci.conf_slot, 1);
  }
}

int sip_init(){
  pj_status_t pj_status;
  PJ( pjsua_create() );

  pjsua_config ua_cfg;
  pjsua_config_default(&ua_cfg);
  ua_cfg.cb.on_reg_state = &on_reg_state;
  ua_cfg.cb.on_call_state = &on_call_state;
  ua_cfg.cb.on_call_media_state = &on_call_media_state;

  pjsua_logging_config log_cfg;    
  pjsua_logging_config_default(&log_cfg);
  
  pjsua_media_config media_cfg;
  pjsua_media_config_default(&media_cfg);

  PJ( pjsua_init(&ua_cfg, &log_cfg, &media_cfg) );

  
  pjsua_transport_config transport_cfg;
  pjsua_transport_config_default(&transport_cfg);
  transport_cfg.port = 0;

  pjsua_transport_id transport_id;
  PJ( pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, &transport_id) );

  pjsua_acc_id local_acc_id;
  PJ( pjsua_acc_add_local(transport_id, 1, &local_acc_id) );

  PJ( pjsua_set_null_snd_dev() );
  PJ( pjsua_start() );
  
  return 1;
}


pj_status_t get_frame(pjmedia_port *port, pjmedia_frame *frame){
  if(tx_pos > BUFSIZE){
    run = 0;
    return PJ_SUCCESS;
  }

  int i = 0;
  char *buf = (char*)frame->buf;
  while(i < frame->size){
    buf[i++] = tx_buf[tx_pos++];
  }

  return PJ_SUCCESS;
}

pj_status_t put_frame(pjmedia_port *port, pjmedia_frame *frame){
  if(frame->type != PJMEDIA_FRAME_TYPE_AUDIO)
    return PJ_SUCCESS;

  int i=0;
  char *buf = (char*)frame->buf;
  while(i < frame->size){
    fputc(buf[i++], rx_out);
    fputc(tx_buf[rx_pos++], tx_out);

    if(rx_pos > BUFSIZE)
      exit(0);
  }

  return PJ_SUCCESS;
}

int port_init(){
  pj_status_t pj_status;


  pj_pool_t *pool = pjsua_pool_create("mypool", 128, 128);

  tx_buf = malloc(sizeof(char) * BUFSIZE);
  rx_buf = malloc(sizeof(char) * BUFSIZE);

  int t=0; while(t < BUFSIZE){
    tx_buf[t+0] = 0;
    tx_buf[t+1] = (t & (t * 16)) | t >> 7;
    t+=2;
  }

  pjmedia_port *port;
  PJ( pjmedia_null_port_create(pool, 8000, 1, 4000, 16, &port) );
  port->put_frame = &put_frame;
  port->get_frame = &get_frame;

  pjsua_conf_port_id id;
  PJ( pjsua_conf_add_port(pool, port, &id) );

  tx_out = fopen("tx_out.raw", "w+");
  rx_out = fopen("rx_out.raw", "w+");

  tx_pos = 0;
  rx_pos = 0;
  return 1;
}

pjsua_call_id dial(pjsua_acc_id acc_id, char *host, char *extn){
  if(strlen(host)+strlen(extn) > 250) DIE(host+extn > 250);
  char *c_uri = malloc(256);
  sprintf(c_uri, "sip:%s@%s", extn, host);
  pj_str_t uri = pj_str(c_uri);

  pjsua_call_setting opt;
  pjsua_call_setting_default(&opt);

  pjsua_call_id call;
  pjsua_call_make_call(acc_id, &uri, &opt, NULL, NULL, &call);

  return call;
}

pjsua_acc_id sip_register(char *host, char *user, char *pass){
  pj_status_t pj_status;
  if(strlen(user)+strlen(host) > 250)
    DIE(user+host > 250);

  char *id = malloc(256);
  char *uri= malloc(256);
  sprintf(id, "sip:%s@%s", user, host);
  sprintf(uri,"sip:%s", host);

  pjsua_acc_config cfg;
  pjsua_acc_config_default(&cfg);
  cfg.id = pj_str(id);
  cfg.reg_uri = pj_str(uri);
  cfg.cred_count = 1;
  cfg.cred_info[0].realm = pj_str("*");
  cfg.cred_info[0].scheme = pj_str("digest");
  cfg.cred_info[0].username = pj_str(user);
  cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
  cfg.cred_info[0].data = pj_str(pass);

  pjsua_acc_id acc_id;
  PJ( pjsua_acc_add(&cfg, PJ_TRUE, &acc_id) );

  free(id);
  free(uri);
  return acc_id;
}
