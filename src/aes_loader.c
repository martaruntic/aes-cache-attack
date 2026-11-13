#include "aes_loader.h"

#include <dlfcn.h>
#include <stdio.h>

int aes_lib_load(aes_lib *lib, const char *path)
{
    lib->handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!lib->handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return -1;
    }

    lib->set_key = (void (*)(aes128_ctx *, const uint8_t *))dlsym(lib->handle, "aes128_set_key");
    lib->encrypt = (void (*)(const aes128_ctx *, const uint8_t *, uint8_t *))dlsym(lib->handle, "aes128_encrypt");
    lib->impl_name = (const char *(*)(void))dlsym(lib->handle, "aes128_impl_name");

    if (!lib->set_key || !lib->encrypt || !lib->impl_name) {
        fprintf(stderr, "%s: missing AES functions\n", path);
        dlclose(lib->handle);
        return -1;
    }
    return 0;
}

void aes_lib_close(aes_lib *lib)
{
    if (lib->handle)
        dlclose(lib->handle);
    lib->handle = NULL;
}
