# bitcoinrpc Reference

The reference consists of two parts: ABI and API.


## ABI

Much from the libcurl ABI description applies here too,
see [libcurl ABI](http://curl.haxx.se/libcurl/abi.html).

ABI stands for *Application Binary Interface* and it
*determines such details as how functions are called and in which binary format
information should be passed from one program component to the next*
([Wikipedia has it all](https://en.wikipedia.org/wiki/Application_binary_interface)).

### Upgrades

The code is currently in beta stage and some updates could break ABI.
You are advised to follow the development and upgrade your code from time to
time until the project goes into a stable phase.

### Version number

The version numbers of the library follow
[Semantic versioning](http://semver.org/#semantic-versioning-200)
scheme, i.e. each version consists of three numbers: MAJOR.MINOR.PATCH.
The major version 0 denotes beta stage. *Except for major version 0*, only
major version updates could (and usually do) break backwards compatibility.
In the beta stage also minor version updates could break ABI, whereas in the
usual case they are reserved for extensions and major bug fixes.  
See `Changelog.md` for the detailed history and changes introduces by each
version.

### Soname bumps

Whenever there are changes done to the library that will cause an ABI breakage,
a soname major number of the library is bumped to a higher one. Again,
**except for beta**.  In fact, soname number and major version number should
stay the same in the future. The history of soname bumps looks as follows:

  * libbitcoinrpc.so.0, *January 2016*


## API

The library API consists of five segments:

* `bitcoinrpc_global`
* `bitcoinrpc_cl`
* `bitcoinrpc_method`
* `bitcoinrpc_resp`
* `bitcoinrpc_err`

Additionally, there is one function called `bitcoinrpc_call()` and a handful
of 'convenience' functions.  See [design](./design.md) document for
a general overview.

The whole user interface is contained in the main header, so to use the
library it is enough to

    #include <bitcoinrpc.h>

What follows is a detailed description of each method and data structure
within the library.  For a quick and easy introduction to typical use cases,
see [tutorial](./tutorial.md) and [examples](./examples.md).

### Error codes

The error codes are defined as enum type `BITCOINRPCEcode`:

```
    typedef enum {

      BITCOINRPCE_OK,                /* Success */
      BITCOINRPCE_CHECK,             /* see: bitcoinrpc_resp_check() */
      BITCOINRPCE_ERR,               /* unspecific error */
      BITCOINRPCE_ALLOC,             /* cannot allocate more memory */
      BITCOINRPCE_BUG,               /* a bug in the library (please report) */
      BITCOINRPCE_CON,               /* connection error */
      BITCOINRPCE_CURLE,             /* libcurl returned some error */
      BITCOINRPCE_JSON,              /* error parsing json data */
      BITCOINRPCE_PARAM              /* wrong parameter, e.g. NULL */

    } BITCOINRPCEcode;
```

### bitcoinrpc_global

Routines to initialise the global state of the library and to clean up the
library's internal garbage at the end of the program.


* `BITCOINRPCEcode`
  **bitcoinrpc_global_init** `(void)`

  The global initialisation function.
  Please call this function from your main thread before any other call. <br>
  *Return*: `BITCOINRPCE_OK` in case of success.


* `BITCOINRPCEcode`
  **bitcoinrpc_global_cleanup** `(void)`

  The global cleanup function.
  Please call this function at the end of your program to collect
  library's internal garbage. <br>
  *Return*: `BITCOINRPCE_OK`.


* `BITCOINRPCEcode`
  **bitcoinrpc_global_set_allocfunc** `( void* (* const f) (size_t size) )`

  Set a memory allocating function for the library routines.
  (the default is just standard malloc). <br>
  *Return*: `BITCOINRPCE_OK`.


* `BITCOINRPCEcode`
  **bitcoinrpc_global_set_freefunc** `( void (* const f) (void *ptr) )`

  Set a memory freeing function for the library routines.
  (the default is just standard free). <br>
  *Return*: `BITCOINRPCE_OK`.


### bitcoinrpc_cl

Routines to handle RPC client.

* **bitcoinrpc_cl_t**

  Type definition of the RPC client data structure.


* `bitcoinrpc_cl_t*`
  **bitcoinrpc_cl_init** `(void)`

  Initialise a new RPC client with default values:
  `BITCOINRPC_USER_DEFAULT`, `BITCOINRPC_PASS_DEFAULT` etc. <br>
  *Return*: a newly allocated handle or NULL in case of error.


* `bitcoinrpc_cl_t*`
  **bitcoinrpc_cl_init_params**
      `(const char *user, const char *pass,
        const char *addr, const unsigned int port )`

  Initialise a new RPC client and set some parameters
  (may not be `NULL`; in that case the function returns `NULL` as well).
  The parameter values are copied, so the original pointers are no longer
  needed. At most `BITCOINRPC_PARAM_MAXLEN` chars are copied to store
  a parameter. <br>
  *Return*: a newly allocated handle or NULL in case of error.


* `BITCOINRPCEcode`
  **bitcoinrpc_cl_free** `(bitcoinrpc_cl_t *cl)`

  Free the handle. <br>
  *Return*: `BITCOINRPCE_OK`.


* `BITCOINRPCEcode`
  **bitcoinrpc_cl_get_user** `(bitcoinrpc_cl_t *cl, char *buf)`

* `BITCOINRPCEcode`
  **bitcoinrpc_cl_get_pass** `(bitcoinrpc_cl_t *cl, char *buf)`

* `BITCOINRPCEcode`
  **bitcoinrpc_cl_get_addr** `(bitcoinrpc_cl_t *cl, char *buf)`

* `BITCOINRPCEcode`
  **bitcoinrpc_cl_get_port** `(bitcoinrpc_cl_t *cl, unsigned int *bufi)`

  Copy a value of the client `cl` parameter to `buf`. The buffer is assumed
  to contain at least `BITCOINRPC_PARAM_MAXLEN` chars.
  At most `BITCOINRPC_PARAM_MAXLEN` chars are copied. <br>
  *Return*: `BITCOINRPCE_OK` or `BITCOINRPCE_PARAM` in case of wrong arguments.


* `BITCOINRPCEcode`
  **bitcoinrpc_cl_get_url** `(bitcoinrpc_cl_t \*cl, char \*buf)`

  Copy value to `buf`. The buffer is assumed to contain at least
  `BITCOINRPC_URL_MAXLEN` chars.
  At most `BITCOINRPC_URL_MAXLEN` chars are copied. <br>
  *Return*: `BITCOINRPCE_OK` or `BITCOINRPCE_PARAM` in case of wrong arguments.


### bitcoinrpc_method

Routines to handle an RPC method.

The argument `json_t *params` to functions below is always copied
to library internals and the original pointer is no longer needed
after the function returns.  It is the obligation of the user to free
the original pointer by decreasing its reference count:
[`json_decref(params)`](https://jansson.readthedocs.org/en/2.7/apiref.html#c.json_decref).


* **bitcoinrpc_method_t**

  Type definition of a method object.


* **BITCOINRPC_METHOD**

  The enum type storing RPC method names (*not fully implemented yet!*):
```
typedef enum {

/* Control RPC */
  BITCOINRPC_METHOD_GETINFO,            /* deprecated */
  BITCOINRPC_METHOD_HELP,
  BITCOINRPC_METHOD_STOP,

/* Network */
  BITCOINRPC_METHOD_GETNETWORKINFO,
  BITCOINRPC_METHOD_GETCONNECTIONCOUNT,

  BITCOINRPC_METHOD_GETWALLETINFO,
  BITCOINRPC_METHOD_SETTXFEE

} BITCOINRPC_METHOD;
```


* `bitcoinrpc_method_t *`
  **bitcoinrpc_method_init** `(const BITCOINRPC_METHOD m)`

  Initialise a new bare method. <br>
  *Return*: a newly allocated method or `NULL` in case of error.


* `bitcoinrpc_method_t *`
  **bitcoinrpc_method_init_params**
      `(const BITCOINRPC_METHOD m, json_t * const params)`

  Initialise a new method with `json_t` array: `params`.
  If `params == NULL`, this is the same as `bitcoinrpc_method_init()`. <br>
  *Return*: a newly allocated method or `NULL` in case of error.


* `BITCOINRPCEcode`
  **bitcoinrpc_method_free** `(bitcoinrpc_method_t *method)`

  Destroy the method.
  *Return*: `BITCOINRPCE_OK`.


* `BITCOINRPCEcode`
  **bitcoinrpc_method_set_params**
      `(bitcoinrpc_method_t *method, json_t *params)`

  Set a new `json_t` object as method parameters. <br>
  *Return*: `BITCOINRPCE_OK`, or `BITCOINRPCE_JSON` if `params` cannot
  be parsed.


* `BITCOINRPCEcode`
  **bitcoinrpc_method_get_params**
      `(bitcoinrpc_method_t *method, json_t **params)`

  Get a deep copy of the method's parameters and store it in `params`. <br>
  *Return*: `BITCOINRPCE_OK`, or `BITCOINRPCE_JSON` if `params` cannot
  be copied by libjansson.


### bitcoinrpc_resp

Store JSON responses from the server.


* **bitcoinrpc_resp_t**

  Type definition of the response struct.


* `bitcoinrpc_resp_t *`
  **bitcoinrpc_resp_init** `(void)`

  Initialise a new bare response. <br>
  *Return*: a newly allocated response or `NULL` in case of error.


* `BITCOINRPCEcode`
  **bitcoinrpc_resp_free** `(bitcoinrpc_resp_t *resp)`

  Destroy the response `resp`.
  *Return*: `BITCOINRPCE_OK`.


* `json_t *`
  **bitcoinrpc_resp_get** `(bitcoinrpc_resp_t *resp)`

  Get a deep copy of the JSON object representing the response from the server.
  *Return*: a newly allocated `json_t` object or `NULL` in case of error.


* `BITCOINRPCEcode`
  **bitcoinrpc_resp_check**
      `(bitcoinrpc_resp_t *resp, bitcoinrpc_method_t *method)`

  Check, if the `resp` comes as a result of calling `method`.
  This check is already performed by `bitcoinrpc_call()`.
  *Returns*: `BITCOINRPCE_OK` or `BITCOINRPCE_CHECK`, if check fails.


### bitcoinrpc_call()

* `BITCOINRPCEcode`
  **bitcoinrpc_call**
      `(bitcoinrpc_cl_t * cl, bitcoinrpc_method_t * method,
                 bitcoinrpc_resp_t *resp, bitcoinrpc_err_t *e)`

 Use client `cl` to call the server with `method`. Save response in `resp`
 and report errors. If `e == NULL`, it is ignored.
 *Return*: `BITCOINRPCE_OK` in case of success, or other error code.


### Convenience functions

Routines to conveniently get data from the server.
If `e == NULL` below, then it is ignored and only standard error codes are
returned.  Arguments are in the order:
*client*, *pointers to where store received data*, *error handle*.


* `BITCOINRPCEcode`
  **bitcoinrpc_cget_getconnectioncount**
      `(bitcoinrpc_cl_t *cl, unsigned int *c, bitcoinrpc_err_t *e)`

  Get the total number of connections (both inbound and outbound)
  via 'getconnectioncount' method and save it in `c`.
