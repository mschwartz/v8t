/*global require, exports, log */
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
        this.on('exit', function() {
            log('exit');
            new Thread(Child, serverSocket, fn).start();
        });
        while (true) {
            serverSocket.mutex.lock();
            var sock = serverSocket.accept();
            serverSocket.mutex.unlock();

            var is = new InputStream(sock.fd),
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
            is.destroy();
            os.destroy();
            sock.destroy();
        }
    }

    exports.extend({
        Child: Child
    });

}());
