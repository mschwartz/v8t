(function() {
    "use struct";

    var pthread = require('builtin/pthread');

    var threads = {},
        nextThreadId = 0;

    function allocThreadId() {
        while (true) {
            ++nextThreadId;
            nextThreadId %= 65536;
            if (!threads[nextThreadId]) {
                return nextThreadId;
            }
        }
    }
    function Thread(fn) {
        var args = [];
        for (var i=1; i<arguments.length; i++) {
            args.push(arguments[i]);
        }
        this.fn = fn;
        this.args = args;
        this.lockCount = 0;
        this.listeners = {};
        this.data = {};
        this.threadId = allocThreadId();
        threads[this.threadId] = this;
        // log('Thread created');
    }
    Thread.currentThread = function() {
        var threadId = pthread.tid();
        if (threadId === false) {
            return false;
        }
        return threads[threadId];
    };
    Thread.exit = function() {
        log('THREAD.EXIT');
        throw 'THREAD.EXIT';
    };
    Thread.prototype.extend({
        on: function(event, fn) {
            this.listeners[event] = this.listeners[event] || [];
            this.listeners[event].push(fn);
        },
        fire: function(event) {
            var me = this;
            if (me.listeners[event]) {
                me.listeners[event].each(function(fn) {
                    fn.apply(me, []);
                });
            }
        },
        start: function() {
            this.tid = pthread.create(this);
        },
        runHandler: function(me) {
            pthread.setThreadId(me.threadId);
            try {
                me.fn.apply(me, me.args);
                me.exitHandler(me);
            }
            catch (e) {
                if (e !== 'THREAD.EXIT') {
                    console.dir(e);
                    console.dir(e.stack);
                }
                me.exitHandler(me);
            }
            // pthread.exit();
        },
        exitHandler: function(me) {
            this.fire('exit');
            if (me.lockCount) {
                var Mutex = require('threads').Mutex;
                Mutex.unlock(me);
            }
            delete threads[me.threadId];
            // pthread.exit();
        }
    });

    exports.Thread = Thread;
}());
