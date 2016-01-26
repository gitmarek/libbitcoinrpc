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

#ifndef BITCOINRPC_H_51fe7847_aafe_4e78_9823_eff094a30775
#define BITCOINRPC_H_51fe7847_aafe_4e78_9823_eff094a30775

/* Name and version */
#define BITCOINRPC_LIBNAME "bitcoinrpc"
#define BITCOINRPC_VERSION "0.0.0"

/* Defalut port and address a RPC client connects to */
#define BITCOINRPC_ADDR_DEFAULT "127.0.0.1"
#define BITCOINRPC_PORT_DEFAULT 8332

/*
Maximal length of a string that holds client's parameters
(including the terminating '\0' character).
*/
#define BITCOINRPC_STR_MAXLEN 250

/*
Maximal length of the server url:
"http://%s:%s@%s:%d" = 3*BITCOINRPC_STR_MAXLEN + 15
*/
#define BITCOINRPC_URL_LEN 765

/* Maximal length of an error message */
#define BITCOINRPC_ERRMSG_MAXLEN 1000

/* Error codes */
typedef enum {

  BITCOINRPCE_OK,                   /* Success */
  BITCOINRPCE_ALLOC,                /* cannot allocate more memory */
  BITCOINRPCE_BUG,                  /* a bug in the library (please report) */
  BITCOINRPCE_CON,                  /* connection error */
  BITCOINRPCE_CURLE,                /* libcurl returned some error */
  BITCOINRPCE_JSON,                 /* error parsing json data */
  BITCOINRPCE_PARAM                 /* wrong parameter */

} BITCOINRPCEcode;


/* RPC methods */
typedef enum {

  BITCOINRPC_METHOD_GETINFO,            /* "getinfo" */
  BITCOINRPC_METHOD_GETNETWORKINFO,     /* "getnetworkinfo" */
  BITCOINRPC_METHOD_GETWALLETINFO,      /* "getwalletinfo" */
  BITCOINRPC_METHOD_HELP

} BITCOINRPC_METHOD;

/* -------------bitcoinrpc_global --------------------- */
/*
The global initialisation function.
Please call this function from your main thread before any other call.
*/
BITCOINRPCEcode
bitcoinrpc_global_init (void);

/*
The global cleanup function.
Please call this function at the end of your program to collect library's
internal garbage.
*/
BITCOINRPCEcode
bitcoinrpc_global_cleanup (void);


/* -------------bitcoinrpc_cl --------------------- */
/* The handle. */
struct bitcoinrpc_cl;

typedef
  struct bitcoinrpc_cl
bitcoinrpc_cl_t;



#endif  /* BITCOINRPC_H_51fe7847_aafe_4e78_9823_eff094a30775 */
