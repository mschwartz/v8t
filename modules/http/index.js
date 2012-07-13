/*global require, exports */
(function() {
    "use strict";

    var Server = require('lib/Server').Server;

    function createServer(fn) {
        return new Server(fn);
    }

    exports.extend({
        createServer: createServer
    });
}());
