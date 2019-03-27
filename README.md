# A HTTP caching proxy in C++

To run the proxy, first you need to compile the source code with the `Makefile`:

```bash
$ cd proxy
$ make
```

Then you can run the proxy within Docker:

```bash
$ cd ..
$ sudo docker-compose-up
```

The default port is 12345