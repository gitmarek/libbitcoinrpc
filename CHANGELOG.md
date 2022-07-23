CHANGELOG

Overview

- Fixed string buffer overflow in src/bitcoinrpc_err.c

Details

    - [#1]("The stpncpy() and strncpy() functions copy merely len characters from src into dst. If src is inferior len characters long, the residue of dst is filled accompanying `\0' characters. Otherwise, dst is not finished."

    - [#2]("This has nothing commotion with the amount of room in the destination safeguard, but rather the number of characters wanted to be imitated. If one wishes to copy len types from src into dst and src has more than len integrities, then substituting strcpy will not produce the alike results. Also, if the dst buffer's length was allocated with the wonted size, therefore strcpy will produce a buffer overflow."