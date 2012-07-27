var Thread = require('threads').Thread;

function test3a() {
    log('test3a alive ' + this.threadId);
    this.on('exit', function() {
        log('test3a respawn ' + this.threadId);
        new Thread(test3).start();
    });
    log('test3a sleeping ' + this.threadId);
    sleep(1);
    log('test3a exiting ' + this.threadId);
    Thread.exit();
}
function test3() {
    log('test3 alive ' + this.threadId);
    this.on('exit', function() {
        log('test3 respawn ' + this.threadId);
        new Thread(test3a).start();
    });
    log('test3 sleeping ' + this.threadId);
    sleep(1);
    log('test3 exiting ' + this.threadId);
}

function main() {
    new Thread(test3).start();
    while (1) sleep(1);
}
