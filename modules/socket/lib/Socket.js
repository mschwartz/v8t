/*global require, exports */
(function() {
    "use strict";

    var net = require('builtin/net'),
        AF_INET = net.AF_INET,
        SOCK_STREAM = net.SOCK_STREAM;

    function Socket(fd, remote_addr) {
        this.fd = fd || net.socket(AF_INET, SOCK_STREAM, 0);
        this.remote_addr = remote_addr;
    }
    Socket.prototype.extend({
        destroy: function() {
            net.close(this.fd);
        },
        listen: function(port, listenAddress, backlog) {
            listenAddress = listenAddress || net.INADDR_ANY;
            backlog = backlog || 10;
            return net.listen(this.fd, port, listenAddress, backlog);
        },
        error: function() {
            return net.error();
        },
        accept: function() {
            var s = net.accept(this.fd);
            return s !== false ? new Socket(s.fd, s.remote_addr) : false;
        }
    });
    
    exports.Socket = Socket;
}());
