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
#include "bitcoinrpc_global.h"


struct bitcoinrpc_cl
{
  uuid_t uuid;
  char uuid_str[37];  /* man 3 uuid_unparse */

  char user[BITCOINRPC_PARAM_MAXLEN];
  char pass[BITCOINRPC_PARAM_MAXLEN];
  char addr[BITCOINRPC_PARAM_MAXLEN];
  unsigned int port;

  char url[BITCOINRPC_URL_LEN];

  char tmpstr[BITCOINRPC_PARAM_MAXLEN]; /* very internal */
  unsigned int tmpstr_len;

  CURL *curl;

  /*
  This is a legacy pointer. You can point to an auxilliary structure,
  if you prefer not to touch this one (e.g. not to break ABI).
  */
  void *legacy_ptr_4f1af859_c918_484a_b3f6_9fe51235a3a0;
};


bitcoinrpc_cl_t*
bitcoinrpc_cl_init (bitcoinrpc_err_t *e)
{
  return bitcoinrpc_cl_init_params (BITCOINRPC_USER_DEFAULT,
                                BITCOINRPC_PASS_DEFAULT,
                                BITCOINRPC_ADDR_DEFAULT,
                                BITCOINRPC_PORT_DEFAULT, e);
}


bitcoinrpc_cl_t*
bitcoinrpc_cl_init_params ( const char* user, const char* pass,
                            const char* addr, const unsigned int port,
                            bitcoinrpc_err_t *e)
{

  bitcoinrpc_cl_t *cl = malloc (sizeof * cl);
  if (NULL == cl)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_ALLOC, "cannot allocate memory");
    return NULL;
  }

  memset (cl->tmpstr, 0, BITCOINRPC_PARAM_MAXLEN);

  memset (cl->user, 0, BITCOINRPC_PARAM_MAXLEN);
  memset (cl->pass, 0, BITCOINRPC_PARAM_MAXLEN);
  memset (cl->addr, 0, BITCOINRPC_PARAM_MAXLEN);
  cl->port = 0;

  cl->curl = NULL;

  uuid_generate_random (cl->uuid);
  uuid_unparse_lower (cl->uuid, cl->uuid_str);

  if (NULL == user)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_PARAM, "user name set to NULL");
    return NULL;
  }
  /* make room for terminating '\0' */
  strncpy(cl->user, user, BITCOINRPC_PARAM_MAXLEN - 1);

  if (NULL == pass)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_PARAM, "password set to NULL");
    return NULL;
  }
  strncpy(cl->pass, pass, BITCOINRPC_PARAM_MAXLEN - 1);

  if (NULL == addr)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_PARAM, "addres set to NULL");
    return NULL;
  }
  strncpy(cl->addr, addr, BITCOINRPC_PARAM_MAXLEN - 1);

  cl->port = port;

  cl->curl = curl_easy_init();
  if (NULL == cl->curl)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_CURLE, "curl_easy_init() returned NULL");
    return NULL;
  }

  return cl;
}


BITCOINRPCEcode
bitcoinrpc_cl_free (bitcoinrpc_cl_t *cl)
{

  curl_easy_cleanup (cl->curl);
  cl->curl = NULL;

  return BITCOINRPCE_OK;
}
