# Peer Table System
* Created by Bernardo Meneghini, Lucas Ramon and Thiago Alexandre.
* This program implements a Hash Table and the operations:
 - find <key>: find the value corresponding to a key.
 - store <key, value>: stores a new pair in the system.

This program sends local logs from a client to the server based of some given expression.
You can use and modify the source code as you pleased.

# Compile
* gcc server-side.c -o server
* gcc client-side.c -o client

# Running
* ./server
* ./client -i 127.0.0.1 -f logs.txt -e expression

Find:
* ./client -i 127.0.0.1 -find key

Store:
* ./client -i 127.0.0.1 -store key value

