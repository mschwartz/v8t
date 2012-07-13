/*global require, exports */
(function() {
    "use strict";
 
    var console = require('console'),
        Thread = require('threads').Thread,
        InputStream = require('socket').InputStream,
        OutputStream = require('socket').OutputStream,
        Request = require('Request').Request,
        http = require('builtin/http'),
        Response = require('Response').Response;

    function Child(serverSocket, fn) {
        // console.log(this.threadId + ' alive');
        this.on('exit', function() {
            log('exit');
            new Thread(Child, serverSocket, fn).start();
        });
        while (true) {
            // console.log(this.threadId + ' locking');
            serverSocket.mutex.lock();
            // console.log(this.threadId + ' got lock');
            var sock = serverSocket.accept();
            // console.log(this.threadId + ' accepted connection ' + sock.fd + ' from ' + sock.remote_addr);
            // console.dir(sock);
            serverSocket.mutex.unlock();

            var is = http.openStream(sock.fd), // new InputStream(sock.fd),
                os = new OutputStream(sock.fd);

            var keepAlive = true;
            while (keepAlive) {
                try {
                    var request = new Request(is),
                        response = new Response(os, request.proto);

                    if (request.headers['connection']) {
                        response.headers['Connection'] = 'Keep-Alive';
                        response.headers['keep-alive'] = 'timeout: 5; max = 10000000';
                    }
                    else {
                        response.headers['Connection'] = 'close';
                        keepAlive = false;
                    }
                    fn(request, response);
                }
                catch (e) {
                    if (e === 'EOF') {
                        break;
                    }
                    console.dir(e);
                    console.dir(e.stack);
                }
            }
            http.closeStream(is);
            sock.destroy();
        }
    }

    exports.extend({
        Child: Child
    });

}());
