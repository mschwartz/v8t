#include "SilkJS.h"
#include <pthread.h>

extern Persistent<ObjectTemplate> globalObject;
Persistent<ObjectTemplate> builtinObject;

static Handle<Value> Log (const Arguments& args) {
    {
        String::AsciiValue str(args[0]);
        Unlocker ul;
        printf("%l08lx %s\n", (unsigned long) pthread_self(), *str);
    }
    return Undefined();
}

static Handle<Value>Sleep(const Arguments& args) {
    int n = args[0]->IntegerValue();
    {
        Unlocker ul;
        sleep(n);
    }
    return Undefined();
}

/**
 * @function global.include
 * 
 * ### Synopsis
 * 
 * include(path [,path...]);
 * 
 * Load, compile, and run the specified JavaScript files in the SilkJS context of the current process.
 * 
 * @param {string} path - the path in the file system to a file to include.
 */
static Handle<Value> Include (const Arguments& args) {
    extern char *readFile(const char *fn);
    for (int i = 0; i < args.Length(); i++) {
        String::Utf8Value str(args[i]);
        char buf[strlen(*str) + 18 + 1];
        strcpy(buf, *str);
        char *js_file = readFile(*str);
        if (!js_file) {
            strcpy(buf, *str);
            if (buf[0] != '/') {
                strcpy(buf, "/usr/local/silkjs/");
                strcat(buf, *str);
            }
            js_file = readFile(buf);
        }
        if (!js_file) {
            return ThrowException(String::Concat(String::New("include file not found "), args[i]->ToString()));
        }
        Handle<String> source = String::New(js_file);
        delete [] js_file;
        ScriptOrigin origin(String::New(*str), Integer::New(0), Integer::New(0));
        Handle<Script>script = Script::New(source, &origin);
        script->Run();
    }
    return Undefined();
}

extern void init_pthread_object(),
            init_console_object(),
            init_editline_object(),
            init_process_object(),
            init_net_object(),
            init_fs_object(),
            init_v8_object(),
            init_http_object(),
            init_mem_object();

void InitGlobalObject() {
    globalObject = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
    builtinObject = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
    
    init_pthread_object();
    init_editline_object();
    init_console_object();
    init_process_object();
    init_net_object();
    init_fs_object();
    init_v8_object();
    init_mem_object();
    init_http_object();

    globalObject->Set(String::New("builtin"), builtinObject);

    globalObject->Set(String::New("log"), FunctionTemplate::New(Log));
    globalObject->Set(String::New("sleep"), FunctionTemplate::New(Sleep));
    globalObject->Set(String::New("include"), FunctionTemplate::New(Include));
}
