#include "SilkJS.h"
#include <pthread.h>

// using namespace v8;

// extern Persistent<ObjectTemplate> globalObject;
// extern Persistent<ObjectTemplate> builtinObject;
// extern Persistent<Context> context;

unsigned short nextThreadId = 0;

struct ThreadInfo {
public:
    pthread_t t;
    unsigned short threadId;
    Persistent<Object>o;
};

static pthread_key_t tls_key;

extern void ReportException (v8::TryCatch* try_catch);

static void *runner(void *p) {
    // Context::Scope cscope(context);
    // HandleScope scope;
    ThreadInfo *t = (ThreadInfo *)p;
    // printf("%08lx > runner\n", pthread_self());
    pthread_setspecific(tls_key, t);
    pthread_detach(pthread_self());
    {
        Locker loc;
        HandleScope scope;
        Handle<Value>v = t->o->Get(String::New("runHandler"));
        if (v.IsEmpty()) {
            printf("No runHandler\n");
            pthread_exit(NULL);
        }
        else if (!v->IsFunction()) {
            printf("No runHandler (not a function)\n");
            pthread_exit(NULL);
        }
        Handle<Function>func = Handle<Function>::Cast(v);
        Handle<Value>av[1];
        av[0] = t->o;
        // TryCatch tryCatch;
        // printf("CALL\n");
        // context->Enter();
        v = func->Call(context->Global(), 1, av);
        // context->Exit();

        // if (v.IsEmpty()) {
            // printf("Exception!");
            // ReportException(&tryCatch);
        // }
    }
    // printf("exitx\n");
    pthread_exit(NULL);
}

static JSVAL create(JSARGS args) {
    Locker l;
    // printf("%08lx > create\n", pthread_self());
    ThreadInfo *t;
    t = new ThreadInfo;
    // t->threadId = nextThreadId++;
    t->o = Persistent<Object>::New(args[0]->ToObject());
    {
        // Unlocker ul;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setguardsize(&attr, 4 * 1024);
        pthread_attr_setstacksize(&attr, 64 * 1024);
        pthread_create(&t->t, & attr, runner, t);
    }
    return Integer::New(t->threadId);
}

static JSVAL set_tid(JSARGS args) {
    ThreadInfo *t = (ThreadInfo *)pthread_getspecific(tls_key);
    t->threadId = args[0]->IntegerValue();
    return Undefined();
}

static JSVAL exit(JSARGS args) {
    Locker l;
    // printf("%08lx > exit\n", pthread_self());
    // context->Exit();
    // sleep(1);
    ThreadInfo *t = (ThreadInfo *)pthread_getspecific(tls_key);
    if (t) {
        // Locker l;
        t->o.Dispose();
    }
    delete t;
    pthread_setspecific(tls_key, NULL);
    Unlocker u;
    pthread_exit(NULL);
}

static JSVAL tid(JSARGS args) {
    ThreadInfo *t;
    {
        Unlocker ul;
        t = (ThreadInfo *)pthread_getspecific(tls_key);
    }
    if (!t) {
        return False();
    }
    return Integer::New(t->threadId);
}

static JSVAL t(JSARGS args) {
    char buf[128];
    sprintf(buf, "%08lx", pthread_self());
    return String::New(buf);
}
static Handle<Value> mutex_init(const Arguments& args) {
    pthread_mutex_t *mutex;
    {
        Unlocker ul;
        mutex = new pthread_mutex_t;
        pthread_mutex_init(mutex, NULL);
    }
    return External::New(mutex);
}

static JSVAL mutex_destroy(JSARGS args) {
    Unlocker ul;
    pthread_mutex_t *mutex = (pthread_mutex_t *)JSEXTERN(args[0]);
    pthread_mutex_destroy(mutex);
    delete mutex;
    return Undefined();
}

static JSVAL mutex_lock(JSARGS args) {
    int ret;
    {
        Unlocker ul;
        pthread_mutex_t *mutex = (pthread_mutex_t *)JSEXTERN(args[0]);
        ret = pthread_mutex_lock(mutex);
    }
    return Integer::New(ret);
}

static JSVAL mutex_trylock(JSARGS args) {
    Unlocker ul;
    pthread_mutex_t *mutex = (pthread_mutex_t *)JSEXTERN(args[0]);
    return Integer::New(pthread_mutex_trylock(mutex));
}

static JSVAL mutex_unlock(JSARGS args) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)JSEXTERN(args[0]);
    int ret;
    {
        Unlocker ul;
        ret = pthread_mutex_unlock(mutex);
    }
    return Integer::New(ret);
}

///////////////////
// Condition Variables

static JSVAL cond_init(JSARGS args) {
    pthread_cond_t *cond;
    {
        Unlocker ul;
        cond = new pthread_cond_t;
        pthread_cond_init(cond, NULL);
    }
    return External::New(cond);
}

static JSVAL cond_destroy(JSARGS args) {
    Unlocker ul;
    pthread_cond_t *cond = (pthread_cond_t *)JSEXTERN(args[0]);
    pthread_cond_destroy(cond);
    delete cond;
    return Undefined();
}

static JSVAL cond_wait(JSARGS args) {
    Unlocker ul;
    pthread_cond_t *cond = (pthread_cond_t *)JSEXTERN(args[0]);
    pthread_mutex_t *mutex = (pthread_mutex_t *)JSEXTERN(args[1]);
    return Integer::New(pthread_cond_wait(cond, mutex));
}

static JSVAL cond_signal(JSARGS args) {
    Unlocker ul;
    pthread_cond_t *cond = (pthread_cond_t *)JSEXTERN(args[0]);
    return Integer::New(pthread_cond_signal(cond));    
}

static JSVAL cond_broadcast(JSARGS args) {
    Unlocker ul;
    pthread_cond_t *cond = (pthread_cond_t *)JSEXTERN(args[0]);
    return Integer::New(pthread_cond_broadcast(cond));    
}

void init_pthread_object() {
    pthread_key_create(&tls_key, NULL);

    Handle<ObjectTemplate>o = ObjectTemplate::New();
    o->Set(String::New("create"), FunctionTemplate::New(create));
    o->Set(String::New("exit"), FunctionTemplate::New(exit));
    o->Set(String::New("tid"), FunctionTemplate::New(tid));
    o->Set(String::New("setThreadId"), FunctionTemplate::New(set_tid));
    o->Set(String::New("t"), FunctionTemplate::New(t));
    // mutexes
    o->Set(String::New("mutex_init"), FunctionTemplate::New(mutex_init));
    o->Set(String::New("mutex_destroy"), FunctionTemplate::New(mutex_destroy));
    o->Set(String::New("mutex_lock"), FunctionTemplate::New(mutex_lock));
    o->Set(String::New("mutex_trylock"), FunctionTemplate::New(mutex_trylock));
    o->Set(String::New("mutex_unlock"), FunctionTemplate::New(mutex_unlock));
    // condition variables
    o->Set(String::New("cond_init"), FunctionTemplate::New(cond_init));
    o->Set(String::New("cond_destroy"), FunctionTemplate::New(cond_destroy));
    o->Set(String::New("cond_wait"), FunctionTemplate::New(cond_wait));
    o->Set(String::New("cond_signal"), FunctionTemplate::New(cond_signal));
    o->Set(String::New("cond_broadcast"), FunctionTemplate::New(cond_broadcast));
    //
    builtinObject->Set(String::New("pthread"), o);
}
