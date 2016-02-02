# Examples

This is a short list of examples of how to use **bitcoinrpc** library.
For a detailed description of all the functions that make bitcoinrpc interface,
please refer to the [Reference](./reference.md).  You may also want to check
the [tutorial](./tutorial.md) and a document outlining the general
[design](./design.md).

## A simple program

Get connection number and a new address in the most simple way:

```
    #include <bitcoinrpc.h>

    int main(void)
    {
      bitcoinrpc_global_init();

      bitcoinrpc_cl_t *cl;
      cl = bitcoinrpc_cl_init_params ("", "bitcoinrpc", "127.0.0.1", 8332);

      bitcoinrpc_err_t e;

      unsigned int c = bitcoinrpc_getconnectioncount (cl, &e);
      if (e.code == BITCOINRPCE_OK)
      {
        fprintf (stdout, "getconnectioncount = %d\n", c);
      }
      else
      {
        fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
      }

      char *a = bitcoinrpc_getnewaddress(cl, &e, NULL);
      if (e.code == BITCOINRPCE_OK)
      {
        fprintf (stdout, "getnewaddress = %s\n", a);
        free (a);
      }
      else
      {
        fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
      }

      bitcoinrpc_cl_free (cl);
      bitcoinrpc_global_cleanup();

      return 0;
    }
```

*last updated: 2016-02-02*
