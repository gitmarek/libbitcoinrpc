# bitcoinrpc

C language interface to JSON-RPC Bitcoin API.

The library provides basic routines to send RPC queries to a listening
Bitcoin node, fetch responses and analyse errors. Its main features include:

* Reusable components, allowing to perform many queries through one open
connection, as well as to query many listening servers with the same method
without reallocating resources.
* Proper error handling.
* Convenience functions to perform simple one-time tasks in an efficient way.


## To build the shared library

Please make sure that you have all the required dependencies installed.
Then type in the project folder:

    make

to compile a dynamically linked library and put it in `.lib/`. It should
also produce the executable `bin/bitcoinprc_test`.

If you want to clean the directory of compiled files and start from scratch,
use:

    make clean  


### Build dependencies

These dependencies are required, if you want to use the code:

 Library     | Purpose             | Description
 ------------|---------------------|-----------------------------------------
  curl       | send data over HTTP | tested with libcurl-7.35.0       
  jansson    | JSON parsing        | tested with >=libjansson-2.5
  libuuid    | generate UUIDs      |

To install the build dependencies on Ubuntu, please type the following
command:

    sudo apt-get install libcurl4-openssl-dev libjansson-dev uuid-dev

(or other libcurl4-\*-dev flavour).


## Installation

To install the successfully compiled source on Unix type systems, please type:

    make install

as a privileged user  (on Debian-like systems: `sudo make install`).
By default, it will install the library in `/usr/local/lib` and the header
file in `/usr/local/include`.  It will also install documentation files in
`/usr/share/doc/bitcoinrpc`, as well as man pages that all start with:
'bitcoinrpc'.

On some systems, especially Ubuntu, the directory `/usr/local/lib` is not
included by default to the `ldconfig` search path.  So either you change
the `Makefile`'s `INSTALL_PREFIX` value to `/usr` (which may be not a very good
idea, since most of the libraries installed by `apt-get` are there),
or add the following line to your `/etc/ld.so.conf`:

    /usr/local/lib

and run:

    sudo ldconfig

See `man 8 ldconfig` for more information.

To remove the files, type:

    make uninstall


## Usage

To use the library, it is enough to include the header file:

    #include <bitcoinrpc.h>

in your source code and provide the following linker flag during compilation:

    -lbitcoinrpc

For further information, see documentation in [doc/](./doc/README.md)
and the header file: [`src/bitcoinrpc.h`](./src/bitcoinrpc.h).

*Please notice that the code is in the very early stage of development.*


## History

See [Changelog.md](./Changelog.md) and [CREDITS](./CREDITS).


## License

The source code is released under the terms of the MIT license.  Please, see
[LICENSE](./LICENSE) for more information.


*last updated: 2016-01-28*
