/*
   The MIT License (MIT)
   Copyright (c) 2016 Marek Miller

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <uuid/uuid.h>

#include "bitcoinrpc.h"
#include "bitcoinrpc_cl.h"
#include "bitcoinrpc_global.h"


struct bitcoinrpc_cl {
  uuid_t uuid;
  char uuid_str[37];  /* man 3 uuid_unparse */

  char user[BITCOINRPC_PARAM_MAXLEN];
  char pass[BITCOINRPC_PARAM_MAXLEN];
  char addr[BITCOINRPC_PARAM_MAXLEN];
  unsigned int port;

  char url[BITCOINRPC_URL_MAXLEN];

  char tmpstr[BITCOINRPC_PARAM_MAXLEN]; /* very internal */
  unsigned int tmpstr_len;

  CURL *curl;
  struct curl_slist *curl_headers;

  /*
     This is a legacy pointer. You can point to an auxilliary structure,
     if you prefer not to touch this one (e.g. not to break ABI).
   */
  void *legacy_ptr_4f1af859_c918_484a_b3f6_9fe51235a3a0;
};

/*
   Internal stuff
 */

# define bitcoinrpc_cl_update_url_(cl) \
  snprintf(cl->url, BITCOINRPC_URL_MAXLEN, "http://%s:%d", \
           cl->addr, cl->port);


CURL*
bitcoinrpc_cl_get_curl_(bitcoinrpc_cl_t *cl)
{
  if (NULL == cl)
    return NULL;
  return cl->curl;
}

/* ------------------------------------------------------------------------ */

bitcoinrpc_cl_t*
bitcoinrpc_cl_init(void)
{
  return bitcoinrpc_cl_init_params(BITCOINRPC_USER_DEFAULT,
                                   BITCOINRPC_PASS_DEFAULT,
                                   BITCOINRPC_ADDR_DEFAULT,
                                   BITCOINRPC_PORT_DEFAULT);
}


bitcoinrpc_cl_t*
bitcoinrpc_cl_init_params(const char* user, const char* pass,
                          const char* addr, const unsigned int port)
{
  bitcoinrpc_cl_t *cl = bitcoinrpc_global_allocfunc(sizeof *cl);

  if (NULL == cl)
    return NULL;

  memset(cl->tmpstr, 0, BITCOINRPC_PARAM_MAXLEN);

  memset(cl->user, 0, BITCOINRPC_PARAM_MAXLEN);
  memset(cl->pass, 0, BITCOINRPC_PARAM_MAXLEN);
  memset(cl->addr, 0, BITCOINRPC_PARAM_MAXLEN);
  cl->port = 0;

  cl->curl = NULL;

  uuid_generate_random(cl->uuid);
  uuid_unparse_lower(cl->uuid, cl->uuid_str);

  if (NULL == user)
    return NULL;
  /* make room for terminating '\0' */
  strncpy(cl->user, user, BITCOINRPC_PARAM_MAXLEN - 1);

  if (NULL == pass)
    return NULL;
  strncpy(cl->pass, pass, BITCOINRPC_PARAM_MAXLEN - 1);

  if (NULL == addr)
    return NULL;
  strncpy(cl->addr, addr, BITCOINRPC_PARAM_MAXLEN - 1);

  if (port <= 0 || port > 65535 )
    return NULL;

  cl->port = port;

  bitcoinrpc_cl_update_url_(cl);

  cl->curl = curl_easy_init();

  cl->curl_headers = NULL;
  cl->curl_headers = curl_slist_append(cl->curl_headers, "content-type: text/plain;");
  if (NULL == cl->curl_headers)
    return NULL;

  curl_easy_setopt(cl->curl, CURLOPT_HTTPHEADER, cl->curl_headers);

  if (NULL == cl->curl)
    return NULL;

  return cl;
}


BITCOINRPCEcode
bitcoinrpc_cl_free(bitcoinrpc_cl_t *cl)
{
  if (NULL == cl)
    return BITCOINRPCE_ARG;

  curl_slist_free_all(cl->curl_headers);
  curl_easy_cleanup(cl->curl);
  cl->curl = NULL;
  bitcoinrpc_global_freefunc(cl);
  cl = NULL;

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_cl_get_user(bitcoinrpc_cl_t *cl, char *buf)
{
  if (NULL == cl || NULL == buf)
    return BITCOINRPCE_ARG;
  strncpy(buf, cl->user, BITCOINRPC_PARAM_MAXLEN);

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_cl_get_pass(bitcoinrpc_cl_t *cl, char *buf)
{
  if (NULL == cl || NULL == buf)
    return BITCOINRPCE_ARG;
  strncpy(buf, cl->pass, BITCOINRPC_PARAM_MAXLEN);

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_cl_get_addr(bitcoinrpc_cl_t *cl, char *buf)
{
  if (NULL == cl || NULL == buf)
    return BITCOINRPCE_ARG;
  strncpy(buf, cl->addr, BITCOINRPC_PARAM_MAXLEN);

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_cl_get_port(bitcoinrpc_cl_t *cl, unsigned int *bufi)
{
  if (NULL == cl || NULL == bufi)
    return BITCOINRPCE_ARG;
  *bufi = cl->port;

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_cl_get_url(bitcoinrpc_cl_t *cl, char *buf)
{
  bitcoinrpc_cl_update_url_(cl);  /* one never knows */

  if (NULL == cl || NULL == buf)
    return BITCOINRPCE_ARG;
  strncpy(buf, cl->url, BITCOINRPC_URL_MAXLEN);

  return BITCOINRPCE_OK;
}
