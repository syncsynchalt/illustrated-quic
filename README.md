# The Illustrated QUIC Connection

Published at https://quic.ulfheim.net

- `site/`: page source for the finished product
- `server/main.c`: server code
- `client/main.c`: client code
- `quiche/`: patch of Cloudflare Quiche / BoringSSL that removes any random aspects of the documented connection
- `captures/`: PCAP and keylog files

See also https://github.com/syncsynchalt/illustrated-tls13 for a similar TLS version of this project.

### Build instructions

If you'd like a working example that reproduces the exact handshake documented on the site:

```
git clone https://github.com/syncsynchalt/illustrated-quic.git
cd illustrated-quic/
cd quiche/
make
cd ../server/
make
cd ../client/
make
```

Then open two terminals and run `./server` in the server/ subdir and `./client` in the client/ subdir.

This has been shown to work on macOS 12 and only has a few easy-to-find dependencies: gcc or clang, rust, cmake, make, patch.
