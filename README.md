# What is this?

An experimental TCP server using multiple threads and IO multiplexing.

- Multiple theading with `pthread`
  - thread per request
  - thread pool
- IO multipexing with `epoll`
- Command protocol
  - (client) `add 2 3\r\n` -> (server) `add ("2 3\r\n")` -> (client) 5
- Dynamically loading shared library with `dlopen`
  - When receive command `f`, the server loads `f` from libf.so dynamically.
  - This enables to update command without restart the server.
