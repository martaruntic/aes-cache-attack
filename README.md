# aes-cache-attack

A Flush+Reload cache attack on T-table AES: an unprivileged process recovers
the AES key of another process only by timing its own memory accesses.

Work in progress.

## Status

- [x] T-table AES-128 as a shared library, tested against FIPS-197
- [ ] AES-NI baseline and benchmark
- [ ] Victim encryption service
- [ ] Flush+Reload primitives
- [ ] Key recovery
- [ ] Countermeasure
- [ ] Evaluation

## Build

Needs Linux on x86-64, gcc, make and Python 3.

```
make
make test
```
