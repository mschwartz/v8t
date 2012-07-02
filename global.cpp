#include <unistd.h>
#include <string.h>
#include <v8.h>

using namespace v8;

extern Persistent<ObjectTemplate> globalObject;
Persistent<ObjectTemplate> builtinObject;

static Handle<Value> Log (const Arguments& args) {
    {
        String::AsciiValue str(args[0]);
        Unlocker ul;
        printf("%ld %s\n", (unsigned long) getpid(), *str);
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
            strcpy(buf, *str);
            if (buf[0] != '/') {
                strcpy(buf, "/usr/share/silkjs/");
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

extern void InitThreads();

void InitGlobalObject() {
    globalObject = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
    builtinObject = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
    InitThreads();

    globalObject->Set(String::New("builtin"), builtinObject);

    globalObject->Set(String::New("log"), FunctionTemplate::New(Log));
    globalObject->Set(String::New("sleep"), FunctionTemplate::New(Sleep));
    globalObject->Set(String::New("include"), FunctionTemplate::New(Include));
}
