#include "SilkJS.h"

Persistent<ObjectTemplate> globalObject;
Persistent<Context> context;

char *readFile (const char *fn) {
    int fd = open(fn, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    long size = lseek(fd, 0, 2);
    lseek(fd, 0, 0);
    char *file = new char[size + 1];
    size = read(fd, file, size);
    file[size] = '\0';
    close(fd);
    return file;
}

const char* ToCString (const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

void ReportException (v8::TryCatch* try_catch) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value exception(try_catch->Exception());
    const char* exception_string = ToCString(exception);
    v8::Handle<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        printf("%s\n", exception_string);
    }
    else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptResourceName());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber();
        printf("%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceline_string = ToCString(sourceline);
        printf("%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn();
        for (int i = 0; i < start; i++) {
            printf(" ");
        }
        int end = message->GetEndColumn();
        for (int i = start; i < end; i++) {
            printf("^");
        }
        printf("\n");
        v8::String::Utf8Value stack_trace(try_catch->StackTrace());
        if (stack_trace.length() > 0) {
            const char* stack_trace_string = ToCString(stack_trace);
            printf("%s\n", stack_trace_string);
        }
        else {
            printf("no stack trace available\n");
        }
    }
}

void AnsiSignalHandler (int sig) {
    signal(sig, AnsiSignalHandler);
    if (sig != SIGINT) {
        printf("Caught Signal %d for process: %ld\n", sig, (size_t) getpid());
    }
    if (sig == SIGPIPE) {
        signal(SIGPIPE, AnsiSignalHandler);
        return;
    }
    exit(sig);
}

extern void InitGlobalObject();

int main (int argc, char *argv[]) {
    char *startup;
    const char *progName;

    signal(SIGSEGV, AnsiSignalHandler);
    signal(SIGINT, AnsiSignalHandler);
    signal(SIGPIPE, AnsiSignalHandler);

    if (argc < 2) {
        startup = readFile("/usr/local/silkjs/builtin/interpreter.js");
        if (!startup) {
            startup = readFile("/usr/share/silkjs/builtin/interpreter.js");
        }
        progName = "interpreter";
    }
    else {
        startup = readFile(argv[1]);
        progName = argv[1];
    }

    if (!startup) {
        printf("%s not found\n", argv[1]);
        exit(1);
    }
    if (startup[0] == '#' && startup[1] == '!') {
        startup[0] = startup[1] = '/';
    }

    // v8 command line switches
    const char *switches = "--harmony";
    V8::SetFlagsFromString(switches, strlen(switches));

    Locker locker;
    Locker::StartPreemption(1);
    HandleScope scope;
    InitGlobalObject();
    context = Context::New(NULL, globalObject);
    Context::Scope context_scope(context);
// { Locker l;
    TryCatch tryCatch;
    Handle<Script>init = Script::New(String::New("global=this; module = {}; include('builtin/all.js');"), String::New("builtin"));
    init->Run();
    Persistent<Script> mainScript = Persistent<Script>::New(Script::Compile(String::New(startup), String::New(progName)));
    if (mainScript.IsEmpty()) {
       ReportException(&tryCatch);
       exit(1);
    }
    Handle<Value>v = mainScript->Run();
    if (v.IsEmpty()) {
       ReportException(&tryCatch);
       exit(1);
    }
    Handle<String> process_name = String::New("main");
    Handle<Value> process_val = context->Global()->Get(process_name);
    if (!process_val.IsEmpty() && process_val->IsFunction()) {
       Handle<Function> process_fun = Handle<Function>::Cast(process_val);
       Persistent<Function> mainFunc = Persistent<Function>::New(process_fun);
       int ac = argc - 2;
       if (ac < 0) {
           ac = 0;
       }
       Handle<Value>av[ac];
       for (int i = 2; i < argc; i++) {
           av[i - 2] = String::New(argv[i]);
       }
       // Locker l;
       v = mainFunc->Call(context->Global(), ac, av);
       if (v.IsEmpty()) {
           ReportException(&tryCatch);
           exit(1);
       }
    }
// }
    context.Dispose();
}
