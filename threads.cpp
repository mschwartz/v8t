#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <v8.h>

using namespace v8;

// extern Persistent<ObjectTemplate> globalObject;
extern Persistent<ObjectTemplate> builtinObject;
extern Persistent<Context> context;

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile unsigned short nextThreadId = 0;
static volatile unsigned short threadExited = 0;

struct Thread {
public:
    struct Thread *next, *prev;
public:
    pthread_t t;
    unsigned short threadId;
    Persistent<Function>fn;
    Persistent<Value>arg;
public:
    Thread(Handle<Value>aFn, Handle<Value>aArg) {
        threadId = ++nextThreadId;
        fn = Persistent<Function>::New(Handle<Function>::Cast(aFn));
        arg = Persistent<Value>::New(aArg);
    }
};

class ThreadList {
    Thread *first, *last;
    pthread_mutex_t lock;
public:
    ThreadList()  {
        first = last = NULL;
        pthread_mutex_init(&lock, NULL);
    }
    void addTail(Thread *thread) {
        pthread_mutex_lock(&lock);
        thread->prev = last;
        if (thread->prev) {
            thread->prev->next = thread;
        }
        thread->next = NULL;
        last = thread;
        if (!first) {
            first = last;
        }
        pthread_mutex_unlock(&lock);
    }
    void addHead(Thread *thread) {
        pthread_mutex_lock(&lock);
        thread->prev = NULL;
        thread->next = first;
        first = thread;
        if (!last) {
            last = thread;
        }
        pthread_mutex_unlock(&lock);
    }
    Thread *remHead() {
        pthread_mutex_lock(&lock);
        Thread *p = first;
        if (p) {
            first = p->next;
            if (first) {
                first->prev = NULL;
            }
        }
        pthread_mutex_unlock(&lock);
        return p;
    }
    Thread *remTail() {
        pthread_mutex_lock(&lock);
        Thread *p = NULL;
        if (last) {
            p = last;
            last = p->prev;
            last->next = NULL;
        }
        pthread_mutex_unlock(&lock);
        return p;
    }
    void remove(Thread *thread) {
        pthread_mutex_lock(&lock);
        Thread  *next = thread->next,
                *prev = thread->prev;
        if (next) {
            next->prev = thread->prev;
        }
        else {
            last = prev;
        }
        if (prev) {
            prev->next = thread->next;
        }
        else {
            first = next;
        }
        pthread_mutex_unlock(&lock);
    }
    void dump() {
        pthread_mutex_lock(&lock);
        for (Thread *t = first; t; t=t->next) {
            printf("threadId: %d\n", t->threadId);
        }
        pthread_mutex_unlock(&lock);
    }
};
static ThreadList activeList, exitedList, waitList;

// struct tinfo {
//    pthread_t t;
//    unsigned short threadId;
//    Persistent<Function>fn;
//    Persistent<Value>arg;
// };

void *ThreadWrapper(void *p) {
    Thread *t = (Thread *)p;
    // tinfo *info = (tinfo *)vinfo;
    {
        Locker l;
        HandleScope scope;
        activeList.addTail(t);
        Handle<Value>av[1];
        av[0] = t->arg;
        t->fn->Call(context->Global(), 1, av);
    }
    {
        Locker l;
        t->fn.Dispose();
        t->arg.Dispose();
        activeList.remove(t);
        exitedList.addTail(t);
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

static Handle<Value> Join(const Arguments& args) {
    pthread_t t = (pthread_t)args[0]->IntegerValue();
    Unlocker ul;
    pthread_join(t, NULL);
    return Undefined();
}

static Handle<Value>Wait(const Arguments& args) {
    int ret;
    Thread *t = exitedList.remHead();
    if (t) {
        ret = t->threadId;
        delete t;
        return Integer::New(ret);
    }
    {
        Unlocker ul;
        pthread_mutex_lock(&mutex);
        while (pthread_cond_wait(&cond, &mutex));
        t = exitedList.remHead();
        pthread_mutex_unlock(&mutex);
        ret = t->threadId;
        delete t;
    }
    return Integer::New(ret);
}

static Handle<Value> Spawn(const Arguments& args) {
    Thread *t = new Thread(args[0], args[1]);
   {
       Unlocker ul;
       pthread_attr_t sched_attr;
       struct sched_param fifo_param;
       pthread_attr_init(&sched_attr);
       pthread_attr_setschedpolicy(&sched_attr, SCHED_OTHER);  
       fifo_param.sched_priority = sched_get_priority_max(SCHED_OTHER);  
       pthread_attr_setschedparam(&sched_attr, &fifo_param);  
       pthread_create(&t->t, &sched_attr, ThreadWrapper, t);
   }
   return Integer::New(t->threadId);
}

void InitThreads() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    o->Set(String::New("spawn"), FunctionTemplate::New(Spawn));
    o->Set(String::New("wait"), FunctionTemplate::New(Wait));
    builtinObject->Set(String::New("pthread"), o);
}
