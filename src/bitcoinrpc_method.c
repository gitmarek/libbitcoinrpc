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

#include <jansson.h>
#include <uuid/uuid.h>

#include "bitcoinrpc.h"
#include "bitcoinrpc_global.h"


/*
Names of Bitcoin RPC methods and how to get them from BITCOINRPC_METHOD codes.
*/
struct BITCOINRPC_METHOD_struct_
{
    BITCOINRPC_METHOD m;
    char *str;
    unsigned char requires_params;
};

#define BITCOINRPC_METHOD_names_len_ 4   /* remember to update it! */

const struct BITCOINRPC_METHOD_struct_
BITCOINRPC_METHOD_names_[BITCOINRPC_METHOD_names_len_] =
{
  { BITCOINRPC_METHOD_GETINFO,            "getinfo",             0 },
  { BITCOINRPC_METHOD_GETNETWORKINFO,     "getnetworkinfo",      0 },
  { BITCOINRPC_METHOD_GETWALLETINFO,      "getwalletinfo",       0 },
  { BITCOINRPC_METHOD_HELP,               "help",                0 },
};


const struct BITCOINRPC_METHOD_struct_ *
bitcoinrpc_method_st_ (const BITCOINRPC_METHOD m)
{
  for (int i = 0; i < BITCOINRPC_METHOD_names_len_; i++)
  {
    if (BITCOINRPC_METHOD_names_[i].m == m)
      return &BITCOINRPC_METHOD_names_[i];
  }
  return NULL;
}


struct bitcoinrpc_method
{

  BITCOINRPC_METHOD   m;

  uuid_t  uuid;
  char    uuid_str[37];   /* why 37? see: man 3 uuid_unparse */

  json_t  *params_json;
  json_t  *post_json;
  char    *post_str;

  /*
  This is a legacy pointer. You can point to an auxilliary structure,
  if you prefer not to touch this one (e.g. not to break ABI).
  */
  void *legacy_ptr_025ed4e5_7a59_4086_83b5_abc3a4767894;

};


BITCOINRPCEcode
bitcoinrpc_method_make_postjson_(bitcoinrpc_method_t *method,
                                bitcoinrpc_err_t *e)
{

  const struct BITCOINRPC_METHOD_struct_ * ms = bitcoinrpc_method_st_(method->m);
  json_object_set_new (method->post_json, "method", json_string(ms->str));
  if (ms->requires_params)
    json_object_set (method->post_json, "params", method->params_json);
  else
    json_object_set (method->post_json, "params", json_array());

  return BITCOINRPCE_OK;
}


BITCOINRPCEcode
bitcoinrpc_method_make_poststr_(bitcoinrpc_method_t *method,
                                bitcoinrpc_err_t *e)
{
  bitcoinrpc_method_make_postjson_(method, e);
  if (e->code != BITCOINRPCE_OK)
    return e->code; /* pass error furher */

  free(method->post_str);
  method->post_str = json_dumps(method->post_json, JSON_COMPACT);
  if (NULL == method->post_str)
    return bitcoinrpc_err_set_(e, BITCOINRPCE_JSON, "JSON error while writing POST data");

  return BITCOINRPCE_OK;
}


bitcoinrpc_method_t *
bitcoinrpc_method_init (const BITCOINRPC_METHOD m,
                        bitcoinrpc_err_t *e)
{
  return bitcoinrpc_method_init_params (m, NULL, e);
}


bitcoinrpc_method_t *
bitcoinrpc_method_init_params (const BITCOINRPC_METHOD m,
                              json_t * const params, bitcoinrpc_err_t *e)
{
  bitcoinrpc_method_t *method = bitcoinrpc_global_allocfunc (sizeof *method);

  if (NULL == method)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_ALLOC, "cannot allocate memory");
    return NULL;
  }

  method->m = m;
  method->post_json = NULL;
  method->post_str = NULL;

  uuid_generate_random(method->uuid);
  uuid_unparse_lower(method->uuid, method->uuid_str);

  json_t *jp;

  if (NULL == params)
  {
    jp = json_array();
  }
  else
  {
    jp = json_deep_copy(params);
  }

  if (NULL == jp)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_JSON, "libjansson cannot copy an object");
    return NULL;
  }
  method->params_json = jp;

  method->post_json = json_object();
  if (NULL == method->post_json)
  {
    bitcoinrpc_err_set_(e, BITCOINRPCE_JSON, "libjansson cannot allocate a new object");
    return NULL;
  }

  /* make post_str */
  if (bitcoinrpc_method_make_poststr_(method, e) != BITCOINRPCE_OK)
  {
    /* pass error further */
    return NULL;
  }

  return method;
}


BITCOINRPCEcode
bitcoinrpc_method_free (bitcoinrpc_method_t *method)
{
  if (NULL == method)
    return BITCOINRPCE_PARAM;

  json_decref(method->post_json);
  json_decref(method->params_json);
  free(method->post_str);

  bitcoinrpc_global_freefunc(method->post_str);

  return BITCOINRPCE_OK;
}


/* Set a new json object as method parameters */
BITCOINRPCEcode
bitcoinrpc_method_set_params (bitcoinrpc_method_t *method, json_t *params)
{
  json_t *jp;

  if (NULL == method)
    return BITCOINRPCE_PARAM;

  if (NULL == params)
  {
    jp = json_array();
  }
  else
  {
    jp = json_deep_copy(params);
  }

  if (NULL == jp)
    return BITCOINRPCE_JSON;

  json_decref(method->params_json);
  method->params_json = jp;

  return bitcoinrpc_method_make_poststr_(method, NULL);
}

/* Update method parameters with new ones, overwrite existing keys. */
BITCOINRPCEcode
bitcoinrpc_method_update_params (bitcoinrpc_method_t *method, json_t *params)
{
  json_t *jp;

  if (NULL == method)
    return BITCOINRPCE_PARAM;

  if (NULL == params)
  {
    jp = json_array();
  }
  else
  {
    jp = json_deep_copy(params);
  }

  if (NULL == jp)
    return BITCOINRPCE_JSON;

  if (json_object_update(method->params_json, jp) != 0)
  {
      json_decref(jp);
      return BITCOINRPCE_JSON;
  }
  json_decref(jp);

  return bitcoinrpc_method_make_poststr_(method, NULL);
}

/* Only the values of existing keys are updated */
BITCOINRPCEcode
bitcoinrpc_method_update_existing_params (bitcoinrpc_method_t *method, json_t *params)
{
    json_t *jp;

    if (NULL == method)
      return BITCOINRPCE_PARAM;

    if (NULL == params)
    {
      jp = json_array();
    }
    else
    {
      jp = json_deep_copy(params);
    }

    if (NULL == jp)
      return BITCOINRPCE_JSON;

    if (json_object_update_existing(method->params_json, jp) != 0)
    {
        json_decref(jp);
        return BITCOINRPCE_JSON;
    }
    json_decref(jp);

    return bitcoinrpc_method_make_poststr_(method, NULL);
}


BITCOINRPCEcode
bitcoinrpc_method_get_params (bitcoinrpc_method_t *method, json_t **params)
{
  json_t *jp;

  if (NULL == params || NULL == method)
    return BITCOINRPCE_PARAM;

  jp = json_deep_copy(method->params_json);
  if (NULL == jp)
    return BITCOINRPCE_JSON;

  *params = jp;

  return BITCOINRPCE_OK;
}
