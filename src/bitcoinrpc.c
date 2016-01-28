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

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson.h>
#include <uuid/uuid.h>

#include "bitcoinrpc.h"
#include "bitcoinrpc_cl.h"
#include "bitcoinrpc_global.h"
#include "bitcoinrpc_err.h"
#include "bitcoinrpc_method.h"
#include "bitcoinrpc_resp.h"



struct bitcoinrpc_call_json_resp_
{
   json_t *json;
   json_error_t err;
};


size_t
bitcoinrpc_call_write_callback_ (char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t n = size * nmemb;
  struct bitcoinrpc_call_json_resp_ *j = (struct bitcoinrpc_call_json_resp_ *) userdata;
  j->json = json_loads (ptr, JSON_REJECT_DUPLICATES, &(j->err));
  return n;
}


BITCOINRPCEcode
bitcoinrpc_call (bitcoinrpc_cl_t * cl, bitcoinrpc_method_t * method,
                 bitcoinrpc_resp_t *resp, bitcoinrpc_err_t *e)
{

  json_t *j;
  char *data;
  char url[BITCOINRPC_URL_MAXLEN];
  char user[BITCOINRPC_PARAM_MAXLEN];
  char pass[BITCOINRPC_PARAM_MAXLEN];
  char credentials[2 * BITCOINRPC_PARAM_MAXLEN + 1];
  struct bitcoinrpc_call_json_resp_ jresp;
  BITCOINRPCEcode ecode;
  CURL * curl;
  CURLcode curl_err;
  char curl_errbuf[CURL_ERROR_SIZE];

  if (NULL == cl || NULL == method || NULL == resp )
    return BITCOINRPCE_PARAM;

  j = json_object();
  if (NULL == j)
    bitcoinrpc_RETURN(e, BITCOINRPCE_JSON, "JSON error while creating a new json_object");

  json_object_set_new (j, "jsonrpc", json_string ("1.0"));  /* 2.0 if you ever implement method batching */
  json_object_update  (j, bitcoinrpc_method_get_postjson_ (method));

  data = json_dumps(j, JSON_COMPACT);
  if (NULL == data)
    bitcoinrpc_RETURN (e, BITCOINRPCE_JSON, "JSON error while writing POST data");

  curl = bitcoinrpc_cl_get_curl_ (cl);

  if (NULL == curl)
    bitcoinrpc_RETURN (e, BITCOINRPCE_BUG, "this should not happen; please report a bug");

  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen (data));
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, bitcoinrpc_call_write_callback_);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jresp);

  ecode = bitcoinrpc_cl_get_url (cl, url);

  if (ecode != BITCOINRPCE_OK)
    bitcoinrpc_RETURN (e, BITCOINRPCE_BUG, "url malformed; please report a bug");
  curl_easy_setopt(curl, CURLOPT_URL, url);

  bitcoinrpc_cl_get_user(cl, user);
  bitcoinrpc_cl_get_pass(cl, pass);
  snprintf(credentials, 2 * BITCOINRPC_PARAM_MAXLEN + 1,
    "%s:%s", user, pass);
  curl_easy_setopt(curl, CURLOPT_USERPWD,	credentials);

  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
  curl_err =	curl_easy_perform(curl);

  json_decref(j); /* no longer needed */
  free(data);

  if (curl_err != CURLE_OK)
  {
    char errbuf[BITCOINRPC_ERRMSG_MAXLEN];
    snprintf (errbuf, BITCOINRPC_ERRMSG_MAXLEN, "curl error: %s", curl_errbuf);
    bitcoinrpc_RETURN (e, BITCOINRPCE_CURLE, errbuf);
  }

  /*
  fprintf(stderr, "%s\n", json_dumps(bitcoinrpc_method_get_postjson_ (method), JSON_INDENT(2)));
  fprintf(stderr, "%s\n", json_dumps(jresp.json, JSON_INDENT(2)));
  */
  bitcoinrpc_resp_set_json_ (resp, jresp.json);
  json_decref(jresp.json); /* no longer needed, since we have deep copy in resp */

  if (bitcoinrpc_resp_check (resp, method) != BITCOINRPCE_OK)
    bitcoinrpc_RETURN (e, BITCOINRPCE_CHECK, "response id does not match post id");

  bitcoinrpc_RETURN_OK;
}



/*
Convenience functions
*/

#define bitcoinrpc_convenience_init_MACRO_(method) \
  bitcoinrpc_method_t *m = NULL; \
  bitcoinrpc_resp_t *r = NULL; \
  json_t *j = NULL; \
  \
  m = bitcoinrpc_method_init(method); \
  if (NULL == m) \
  { \
    bitcoinrpc_RETURN (e, BITCOINRPCE_ALLOC, "cannot initialise a new method"); \
  } \
  r = bitcoinrpc_resp_init(); \
  if (NULL == r) \
  { \
    bitcoinrpc_method_free (m); \
    bitcoinrpc_RETURN (e, BITCOINRPCE_ALLOC, "cannot initialise a new response object"); \
  }

#define bitcoinrpc_convenience_call_MACRO_\
  bitcoinrpc_call (cl, m, r, e);\
  if (e->code != BITCOINRPCE_OK)\
  {\
    bitcoinrpc_method_free (m);\
    bitcoinrpc_resp_free (r);\
    return e->code;  /* pass error further */\
  } \
  j = bitcoinrpc_resp_get (r);

#define bitcoinrpc_convenience_free_MACRO_ \
  json_decref(j); \
  bitcoinrpc_method_free (m); \
  bitcoinrpc_resp_free (r);



BITCOINRPCEcode
bitcoinrpc_cget_getconnectioncount (bitcoinrpc_cl_t *cl, unsigned int *c,
                                    bitcoinrpc_err_t *e)
{
  json_t *ctmp;

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETCONNECTIONCOUNT);
  bitcoinrpc_convenience_call_MACRO_;

  // fprintf (stderr, "%s\n", json_dumps (j, JSON_INDENT(2)));
  ctmp =   json_object_get (j, "result");
  if (NULL == ctmp)
  {
    bitcoinrpc_RETURN (e, BITCOINRPCE_JSON, "cannot parse the result");
  }
  *c = json_integer_value(ctmp);
  json_decref(ctmp);

  bitcoinrpc_convenience_free_MACRO_;
  bitcoinrpc_RETURN_OK;
}
