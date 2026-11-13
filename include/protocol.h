#ifndef PROTOCOL_H
#define PROTOCOL_H

// The victim is an "encryption service": a separate process with a secret
// key that encrypts whatever it gets over a Unix socket.
//
//   client -> victim : 16 bytes plaintext
//   victim -> client : 16 bytes ciphertext
//
// The attacker is just a client. It never sees the key, only plaintexts,
// ciphertexts and the timing of its own memory accesses.

#define DEFAULT_SOCKET_PATH "/tmp/aes_victim.sock"

#endif
