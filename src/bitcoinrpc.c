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



struct bitcoinrpc_call_curl_resp_
{
   char* data;
   unsigned long long int data_len;
   int called_before;
   bitcoinrpc_err_t e;
};


size_t
bitcoinrpc_call_write_callback_ (char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t n = size * nmemb;
  struct bitcoinrpc_call_curl_resp_ *curl_resp = (struct bitcoinrpc_call_curl_resp_ *) userdata;

  if ( ! curl_resp->called_before )
  {
    /* initialise the data structure */
    curl_resp->called_before = 1;
    curl_resp->data_len = 0;
    curl_resp->data = NULL;
    curl_resp->e.code = BITCOINRPCE_OK;
  }

  char * data = NULL;
  unsigned long long int data_len = curl_resp->data_len + n;
  data = bitcoinrpc_global_allocfunc (data_len);
  if (NULL == data)
  {
    curl_resp->e.code = BITCOINRPCE_ALLOC;
    snprintf(curl_resp->e.msg, BITCOINRPC_ERRMSG_MAXLEN,
             "cannot allocate more memory");
    return n;
  }
  /* concatenate old and new data */
  if (NULL != curl_resp->data)
  {
    /* do not copy last '\0' */
    strncpy (data, curl_resp->data, curl_resp->data_len);
  }

  /* break at '\n' */
  size_t i;
  for (i = 0; i < n; i++)
  {
    if (ptr[i] == '\n')
    {
      data[curl_resp->data_len + i] = '\0';
      break;
    }
    data[curl_resp->data_len + i] = ptr[i];
  }


  if (NULL != curl_resp->data)
  {
    bitcoinrpc_global_freefunc(curl_resp->data);
  }

  curl_resp->data = data;
  curl_resp->data_len += i;

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
  struct bitcoinrpc_call_curl_resp_ curl_resp;
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
  //fprintf (stderr, "%s\n", data);

  curl = bitcoinrpc_cl_get_curl_ (cl);

  if (NULL == curl)
    bitcoinrpc_RETURN (e, BITCOINRPCE_BUG, "this should not happen; please report a bug");

  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen (data));
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, bitcoinrpc_call_write_callback_);
  curl_resp.called_before = 0;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_resp);

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
  if (curl_resp.e.code != BITCOINRPCE_OK)
  {
    bitcoinrpc_RETURN (e, BITCOINRPCE_CON, curl_resp.e.msg);
  }

  // fprintf (stderr, "curl_resp.data =  %s\n", curl_resp.data);
  // fprintf (stderr, "\nbytes written: %d\n", (int) curl_resp.data_len);

  /* parse read data into json */
  json_error_t jerr;
  json_t *jtmp = json_loads (curl_resp.data, 0, &jerr);
  if (NULL == jtmp)
  {
    bitcoinrpc_RETURN (e, BITCOINRPCE_JSON, "cannot parse data from server");
  }
  bitcoinrpc_resp_set_json_ (resp, jtmp);
  json_decref(jtmp);

  if (bitcoinrpc_resp_check (resp, method) != BITCOINRPCE_OK)
    bitcoinrpc_RETURN (e, BITCOINRPCE_CHECK, "response id does not match post id");

  bitcoinrpc_RETURN_OK;
}



/*
Convenience functions
*/

#define bitcoinrpc_convenience_init_MACRO_(method_id9e43, params_id83a2c1, err_return) \
  json_t *j = NULL; \
  json_t *jresp = NULL; \
  json_t *jerr = NULL; \
  bitcoinrpc_method_t *m = NULL; \
  bitcoinrpc_resp_t *r = NULL; \
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


#define bitcoinrpc_convenience_set_params_char_MACRO_(parchar_id0e2c12, err_return) \
json_t *params = json_array(); \
  if (NULL == params) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "libjansson cannot initialise a new object"); \
    return err_return; \
  } \
  if (NULL != parchar_id0e2c12) \
  { \
    if (json_array_append_new (params, json_string (parchar_id0e2c12)) != 0) \
    { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse account string"); \
      return err_return; \
    } \
  }


#define bitcoinrpc_convenience_set_params_int_MACRO_(parint_id234a11, err_return) \
json_t *params = json_array(); \
  if (NULL == params) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "libjansson cannot initialise a new object"); \
    return err_return; \
  } \
  if (json_array_append_new (params, json_integer (parint_id234a11)) != 0) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse account string"); \
    return err_return; \
  }


#define bitcoinrpc_convenience_set_params_json_MACRO_(parint_id34ea1f, err_return) \
json_t *params = json_array(); \
  if (NULL == params) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "libjansson cannot initialise a new object"); \
    return err_return; \
  } \
  if (json_array_append_new (params, json_deep_copy (parint_id34ea1f)) != 0) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse account string"); \
    return err_return; \
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
    jresp = json_object_get (j, "result"); \
    if (NULL == jresp) \
    { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot parse the result"); \
      return err_return;\
    } \
  }


#define bitcoinrpc_convenience_copy_resp_string_MACRO_(err_return) \
  size_t n;\
  char* resp_string;\
  if (json_is_string (jresp)) { \
    n = strlen (json_string_value (jresp)) + 1; \
    resp_string = malloc (n); \
    if (NULL == resp_string) \
    { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_ALLOC, "cannot allocate more memory"); \
      return err_return; \
    } \
    strncpy (resp_string, json_string_value(jresp), n); \
  }\
  else if (json_is_object (jresp)) /* jresp is a json object */ \
  { \
    resp_string = json_dumps (jresp, JSON_COMPACT); \
    if (NULL == resp_string) \
    { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "cannot dump a json object"); \
      return err_return; \
    } \
  } \
  else /* this should not happen */ \
  { \
      bitcoinrpc_err_set_ (e, BITCOINRPCE_BUG, "this should not have happened. Please report a bug"); \
      return err_return; \
  }


#define bitcoinrpc_convenience_copy_resp_json_MACRO_(err_return) \
  json_t *resp_json = json_deep_copy (jresp); \
  if (NULL == resp_json) \
  { \
    bitcoinrpc_err_set_ (e, BITCOINRPCE_JSON, "error could not decode data"); \
    return err_return; \
  }


#define bitcoinrpc_convenience_free_MACRO_(err_return) \
  json_decref(j); \
  json_decref(jerr); \
  json_decref(jresp); \
  bitcoinrpc_method_free (m); \
  bitcoinrpc_resp_free (r); \
  bitcoinrpc_err_set_ (e, BITCOINRPCE_OK, NULL);



char*
bitcoinrpc_getbestblockhash (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBESTBLOCKHASH, NULL, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_string_MACRO_ (NULL);
  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_string;
}


char*
bitcoinrpc_getblock (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e,
                      const char *header)
{

  bitcoinrpc_convenience_set_params_char_MACRO_ (header, NULL);
  json_array_append_new (params, json_false()); /* serialized block */

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBLOCK, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_string_MACRO_ (NULL);
  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_string;
}


json_t*
bitcoinrpc_getblock_json (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e,
                          const char *header)
{

  bitcoinrpc_convenience_set_params_char_MACRO_ (header, NULL);
  json_array_append_new (params, json_true()); /* block in json format */

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBLOCK, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_json_MACRO_(NULL);

  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_json;
}


json_t*
bitcoinrpc_getblockchaininfo (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBLOCKCHAININFO, NULL, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_json_MACRO_(NULL);

  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_json;
}


unsigned int
bitcoinrpc_getblockcount (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBLOCKCOUNT, NULL, -1);
  bitcoinrpc_convenience_call_MACRO_ (-1);
  bitcoinrpc_convenience_errcheck_MACRO_ (-1);

  unsigned int c = json_integer_value(jresp);
  bitcoinrpc_convenience_free_MACRO_ (-1);

  return c;
}


char*
bitcoinrpc_getblockhash (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e,
                         unsigned long int height)
{

  bitcoinrpc_convenience_set_params_int_MACRO_ (height, NULL);
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETBLOCKHASH, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_string_MACRO_ (NULL);
  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_string;
}


json_t*
bitcoinrpc_getchaintips (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETCHAINTIPS, NULL, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_json_MACRO_(NULL);

  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_json;
}


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


double
bitcoinrpc_getdifficulty (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETDIFFICULTY, NULL, -1);
  bitcoinrpc_convenience_call_MACRO_ (-1);
  bitcoinrpc_convenience_errcheck_MACRO_ (-1);

  double d = json_real_value(jresp);
  bitcoinrpc_convenience_free_MACRO_ (-1);

  return d;
}


json_t*
bitcoinrpc_getmempoolinfo (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETMEMPOOLINFO, NULL, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_json_MACRO_(NULL);

  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_json;
}



char*
bitcoinrpc_getnewaddress (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e,
                          const char* account)
{

  bitcoinrpc_convenience_set_params_char_MACRO_ (account, NULL);

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETNEWADDRESS, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  json_decref(params);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_string_MACRO_ (NULL);

  bitcoinrpc_convenience_free_MACRO_ (NULL);

  return resp_string;
}


json_t*
bitcoinrpc_getrawmempool (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e, int verbose)
{

  json_t *p = verbose? json_true() : json_false();
  bitcoinrpc_convenience_set_params_json_MACRO_ (p, NULL);
  json_decref(p);

  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_GETRAWMEMPOOL, params, NULL);
  bitcoinrpc_convenience_call_MACRO_ (NULL);
  bitcoinrpc_convenience_errcheck_MACRO_ (NULL);

  /* body of the function: use jresp */
  bitcoinrpc_convenience_copy_resp_json_MACRO_(NULL);
  bitcoinrpc_convenience_free_MACRO_ (NULL);
  return resp_json;
}


void
bitcoinrpc_stop (bitcoinrpc_cl_t *cl, bitcoinrpc_err_t *e)
{
  bitcoinrpc_convenience_init_MACRO_ (BITCOINRPC_METHOD_STOP, NULL, );
  bitcoinrpc_convenience_call_MACRO_ ();
  bitcoinrpc_convenience_errcheck_MACRO_ ();

  /* body of the function: use jresp */
  bitcoinrpc_convenience_free_MACRO_ ();
  return;
}
