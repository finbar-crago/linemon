#include <stdio.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>

int run;
pjsua_conf_port_id pid;

#define CHECK(fn) puts(fn);

#define DIE(fn) \
  { printf("[%s:%d(%s)] !%s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__,#fn); \
    exit(-1); }


#define PJ(fn) pj_status = fn; if(pj_status != PJ_SUCCESS) DIE(fn)


pj_status_t get_frame(pjmedia_port *port, pjmedia_frame *frame){
  puts("get_frame");
  return PJ_SUCCESS;  
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
  if(ci.state == PJSIP_INV_STATE_DISCONNECTED){}
}

static void on_call_media_state(pjsua_call_id call_id){
  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);
  if(ci.media_status == PJSUA_CALL_MEDIA_ACTIVE){
    // FIXME: This is lazy....
    pjsua_conf_connect(1, ci.conf_slot);
  }
}


int main(int argc, char **argv){
  pj_status_t pj_status;
  run = 0;
  sip_init();
  pjsua_acc_id acc_id = sip_register("172.28.128.3", "100", "pass");
  while(!run){sleep(1);}

  port_init();

  pj_str_t uri = pj_str("sip:101@172.28.128.3");
  pjsua_call_setting opt;
  pjsua_call_setting_default(&opt);

  pjsua_call_id call;
  pjsua_call_make_call(acc_id, &uri, &opt, NULL, NULL, &call);
 
  while(1){sleep(300);}
  return 0;
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

  PJ( pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, NULL) );

  PJ( pjsua_set_null_snd_dev() );
  PJ( pjsua_start() );
  
  return 1;
}

int port_init(){
  pj_status_t pj_status;

  pj_pool_t *pool = pjsua_pool_create("mypool", 128, 128);

  pj_size_t len = 8000*300;
  char *buf = malloc(sizeof(char)* len);
  int t=0; while(t++<len) buf[t] = (t & (t * 16)) | t >> 7;

  pjmedia_port *src_port;
  PJ( pjmedia_mem_player_create(pool, buf, len, 8000, 1, 8000, 16, 0, &src_port) );

  pjsua_conf_port_id src;
  PJ( pjsua_conf_add_port(pool, src_port, &src) );
    
  PJ( pjsua_conf_connect(src, 0) );
  CHECK("pjsua_conf_connect");

  return 1;
}


pjsua_acc_id sip_register(char *host, char *user, char *pass){
  pj_status_t pj_status;
  if(strlen(user)+strlen(host) > 58)
    DIE(user+host > 58);

  char *id = malloc(64);
  char *uri= malloc(64);
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
