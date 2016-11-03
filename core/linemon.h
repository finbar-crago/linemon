#ifndef _LINEMON_H
#define _LINEMON_H

#include <stdio.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>

#ifdef __cplusplus
extern "C"{
}
#endif

#define BUFSIZE 8000*30

#define DIE(fn) \
  { printf("[%s:%d(%s)] !%s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__,#fn); \
    exit(-1); }

#define PJ(fn) pj_status = fn; if(pj_status != PJ_SUCCESS) DIE(fn)


/* --- sipstuff.c ----*/
int sip_init();
int port_init();

pjsua_acc_id sip_register(char *host, char *user, char *pass);
pjsua_call_id dial(pjsua_acc_id acc_id, char *host, char *extn);

#ifdef __cplusplus
}
#endif
#endif //_LINEMON_H


