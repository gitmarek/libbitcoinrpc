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
   bitcoinrpc_err_t e;
};


size_t
bitcoinrpc_call_write_callback_ (char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t n = size * nmemb;
  struct bitcoinrpc_call_json_resp_ *j = (struct bitcoinrpc_call_json_resp_ *) userdata;
  j->e.code = BITCOINRPCE_OK;
  j->json = json_loads (ptr, JSON_REJECT_DUPLICATES, &(j->err));
  if (j->json == NULL)
  {
    // cannot load json; report an erro
    j->e.code = BITCOINRPCE_CON;
    snprintf(j->e.msg, BITCOINRPC_ERRMSG_MAXLEN,
             "connection error; server response:\n%s", ptr);
  }
  return n;
}


BITCOINRPCEcode
bitcoinrpc_call (bitcoinrpc_cl_t * cl, bitcoinrpc_method_t * method,
                 bitcoinrpc_resp_t *resp, bitcoinrpc_err_t *e)
{

  json_t *j = NULL;
  char *data = NULL;
  char url[BITCOINRPC_URL_MAXLEN];
  char user[BITCOINRPC_PARAM_MAXLEN];
  char pass[BITCOINRPC_PARAM_MAXLEN];
  char credentials[2 * BITCOINRPC_PARAM_MAXLEN + 1];
  struct bitcoinrpc_call_json_resp_ jresp;
  BITCOINRPCEcode ecode;
  CURL *curl = NULL;
  CURLcode curl_err;
  char errbuf[BITCOINRPC_ERRMSG_MAXLEN];
  char curl_errbuf[CURL_ERROR_SIZE];

  if (NULL == cl || NULL == method || NULL == resp )
    return BITCOINRPCE_ARG;

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
    snprintf (errbuf, BITCOINRPC_ERRMSG_MAXLEN, "curl error: %s", curl_errbuf);
    bitcoinrpc_RETURN (e, BITCOINRPCE_CURLE, errbuf);
  }

  /* Check if server returned valid json object */
  if (jresp.e.code != BITCOINRPCE_OK)
  {
    bitcoinrpc_RETURN (e, BITCOINRPCE_CON, jresp.e.msg);
  }

  // fprintf(stderr, "%s\n", json_dumps(bitcoinrpc_method_get_postjson_ (method), JSON_INDENT(2)));
  // fprintf(stderr, "%s\n", json_dumps(jresp.json, JSON_INDENT(2)));

  bitcoinrpc_resp_set_json_ (resp, jresp.json);
  json_decref(jresp.json); /* no longer needed, since we have deep copy in resp */

  if (bitcoinrpc_resp_check (resp, method) != BITCOINRPCE_OK)
    bitcoinrpc_RETURN (e, BITCOINRPCE_CHECK, "response id does not match post id");

  bitcoinrpc_RETURN_OK;
}



/*
Convenience functions
*/

#define bitcoinrpc_convenience_init_MACRO_(method_id9e43, params_id83a2c1, err_return) \
  json_t *jresp = NULL; \
  json_t *jerr = NULL; \
  bitcoinrpc_method_t *m = NULL; \
  bitcoinrpc_resp_t *r = NULL; \
  json_t *j = NULL; \
  \
  m = bitcoinrpc_method_init(method_id9e43); \
  if (NULL == m) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_ALLOC, "cannot initialise a new method"); \
    return err_return;\
  } \
  if (NULL != params_id83a2c1) \
  { \
    BITCOINRPCEcode ecode = bitcoinrpc_method_set_params(m, params_id83a2c1); \
    if (ecode != BITCOINRPCE_OK) \
    { \
      bitcoinrpc_err_set_ (e, ecode, "cannot set parameters"); \
      return err_return;\
    } \
  }\
  r = bitcoinrpc_resp_init(); \
  if (NULL == r) \
  { \
    bitcoinrpc_method_free (m); \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_ALLOC, "cannot initialise a new response object"); \
    return err_return;\
  }


#define bitcoinrpc_convenience_call_MACRO_(err_return) \
  bitcoinrpc_call (cl, m, r, e);\
  if (e->code != BITCOINRPCE_OK)\
  {\
    bitcoinrpc_method_free (m);\
    bitcoinrpc_resp_free (r);\
    return err_return;  /* pass error further */\
  } \
  j = bitcoinrpc_resp_get (r);


#define bitcoinrpc_convenience_errcheck_MACRO_(err_return) \
  jerr  = json_object_get (j, "error"); \
  if (jerr != json_null()) \
  { \
    char errmsg[BITCOINRPC_ERRMSG_MAXLEN]; \
    char *server_errmsg; \
    server_errmsg = json_dumps(jerr, JSON_COMPACT); \
    snprintf(errmsg, BITCOINRPC_ERRMSG_MAXLEN, \
             "%s", server_errmsg); \
    free(server_errmsg); \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_SERV, errmsg); \
    return err_return;\
  } \
  else \
  { \
    jresp =   json_object_get (j, "result"); \
    if (NULL == jresp) \
    { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse the result"); \
      return err_return;\
    } \
  }


#define bitcoinrpc_convenience_free_MACRO_(err_return) \
  json_decref(j); \
  json_decref(jerr); \
  json_decref(jresp); \
  bitcoinrpc_method_free (m); \
  bitcoinrpc_resp_free (r); \
  bitcoinrpc_err_set_ (e, BITCOINRPCE_OK, NULL);



unsigned int
bitcoinrpc_getconnectioncount (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETCONNECTIONCOUNT, NULL, -1);
  bitcoinrpc_convenience_call_MACRO_ (-1);
  bitcoinrpc_convenience_errcheck_MACRO_ (-1);

  unsigned int c = json_integer_value(jresp);
  bitcoinrpc_convenience_free_MACRO_ (-1);

  return c;
}


char*
bitcoinrpc_getnewaddress (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e,
                          const char* account)
{

  json_t *params = json_array();
  if (NULL == params)
  {
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "libjansson cannot initialise a new object");
    return NULL;
  }
  if (NULL != account)
  {
    if (json_array_append_new (params, json_string (account)) != 0)
    {
      bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse account string");
      return NULL;
    }
  }

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETNEWADDRESS, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  json_decref(params);

  /* body of the function: use jresp */
  // fprintf (stderr, "%s\n", json_dumps(j, JSON_INDENT(2)));
  size_t n = strlen (json_string_value (jresp)) + 1;
  char *a = malloc (n);
  if (NULL == a)
  {
    bitcoinrpc_err_set_ (e, BITCOINRPCE_ALLOC, "cannot allocate more memory");
    return NULL;
  }
  strncpy (a, json_string_value(jresp), n);

  bitcoinrpc_convenience_free_MACRO_ (NULL);

  return a; /* return value of BITCOINRPC_METHOD_GETNEWADDRESS*/
}
