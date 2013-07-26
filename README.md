# EasySCTP

A really simple (and currently crappy) C++ SCTP interface.

Currently there is enough to send and receive messages.

Caveats:
 - On linux message size is limited because lksctp does not support SCTP_EXPLICIT_EOR
   - There is a hack to work around this `-DEXPLICIT_EOR_HACK` but it isn't and
     never will be thread safe on either side.
 - Not currently thread safe for large messages (messages that don't fit in one
   send/recieve buffer).  This may be able to be implemented by duping the
   socket descriptor and writing the whole message to that.
 - Whole message must be in memory at one time.

I wouldn't use this for production but it is fun to play with.  It will get
better as I fool around with it and SCTP implementations mature (I develop on
linux so it is hard to test sending large messages when it isn't supported).

See an example client/server in the test directory.  They simply send data from
client to server.  Not too useful but shows the API.
