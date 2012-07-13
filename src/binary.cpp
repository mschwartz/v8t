#include "SilkJS.h"

struct MEMINFO {
    int size;
    unsigned char *mem;
};

#ifndef O_BINARY
#define O_BINARY 0
#endif

static JSVAL mem_alloc(JSARGS args) {
    int size = args[0]->IntegerValue();
    MEMINFO *m = new MEMINFO;
    if (!m) {
        return Null();
    }
    m->size = size;
    m->mem = new unsigned char[size];
    if (!m->mem) {
        delete m;
        return Null();
    }
    return External::New(m);
}

static JSVAL mem_free(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    delete [] m->mem;
    delete m;
    return Undefined();
}

static JSVAL mem_size(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    return Integer::New(m->size);
}

static JSVAL mem_realloc(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int new_size = args[1]->IntegerValue();
    unsigned char *n;
    {
        Unlocker ul;
        n = (unsigned char *)realloc(m->mem, new_size);
    }
    if (!n) {
        return Null();
    }
    m->mem = n;
    m->size = new_size;
    return External::New(m);
}

static JSVAL mem_dup(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    MEMINFO *n = new MEMINFO;
    n->size = m->size;
    n->mem = new unsigned char[n->size];
    memcpy(n->mem, m->mem, n->size);
    return External::New(n);
}

static JSVAL mem_substr(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int offset = args[1]->IntegerValue();
    int size = m->size - offset;
    if (args.Length() > 2) {
        size = args[2]->IntegerValue();
    }
    MEMINFO *n = new MEMINFO;
    if (!n) {
        return Null();
    }
    n->size = size;
    n->mem = new unsigned char[size];
    if (!n->mem) {
        delete n;
        return Null();
    }
    memcpy(n->mem, &m->mem[offset], size);
    return External::New(n);
}

static JSVAL mem_asString(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int offset = 0,
        size = m->size;
    if (args.Length() > 1) {
        offset= args[1]->IntegerValue();
    }
    if (args.Length()> 2) {
        size = args[2]->IntegerValue();
    }
    return String::New((const char *)&m->mem[offset], size);
}

static JSVAL mem_read(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int fd = args[1]->IntegerValue();
    int offset = 0,
        size = m->size;
    if (args.Length() > 2) {
        offset= args[2]->IntegerValue();
    }
    if (args.Length()> 3) {
        size = args[3]->IntegerValue();
    }
    int n;
    { 
        Unlocker u;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        switch (select(fd + 1, &fds, NULL, NULL, &timeout)) {
        // switch (select(fd + 1, &fds, NULL, NULL, NULL)) {
            case -1:
                perror("select");
                return ThrowException(String::Concat(String::New("Read Error: "), String::New(strerror(errno))));
            case 0:
                // printf("Read timed out\n");
                return Null();
        }
        n = read(fd, &m->mem[offset], size);
    }
    return Integer::New(n);
}

static JSVAL mem_write(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int fd = args[1]->IntegerValue();
    int offset = 0,
        size = m->size;
    if (args.Length() > 2) {
        offset= args[2]->IntegerValue();
    }
    if (args.Length()> 3) {
        size = args[3]->IntegerValue();
    }
    int n ;
    {
        Unlocker u;
        n = write(fd, &m->mem[offset], size);
    }
    return Integer::New(n);
}

static JSVAL mem_getat(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    int offset = args[1]->IntegerValue();
    return String::New((const char *)&m->mem[offset], 1);
}

// append(mem, s, offset, len);
// copy s to mem at offset for len bytes
static JSVAL mem_append(JSARGS args) {
    MEMINFO *m = (MEMINFO *)JSEXTERN(args[0]);
    String::AsciiValue s(args[1]->ToString());
    int offset = args[2]->IntegerValue();
    int length = args[3]->IntegerValue();
    {
        // Unlocker u;
        memcpy(&m->mem[offset], *s, length);
    }
    return Undefined();
}

void init_mem_object() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    o->Set(String::New("alloc"), FunctionTemplate::New(mem_alloc));
    o->Set(String::New("free"), FunctionTemplate::New(mem_free));
    o->Set(String::New("size"), FunctionTemplate::New(mem_size));
    o->Set(String::New("realloc"), FunctionTemplate::New(mem_realloc));
    o->Set(String::New("dup"), FunctionTemplate::New(mem_dup));
    o->Set(String::New("substr"), FunctionTemplate::New(mem_substr));
    o->Set(String::New("asString"), FunctionTemplate::New(mem_asString));
    o->Set(String::New("read"), FunctionTemplate::New(mem_read));
    o->Set(String::New("write"), FunctionTemplate::New(mem_write));
    o->Set(String::New("getAt"), FunctionTemplate::New(mem_getat));
    o->Set(String::New("append"), FunctionTemplate::New(mem_append));

    builtinObject->Set(String::New("mem"), o);
}