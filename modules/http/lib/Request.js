/*global require, exports, log */
(function() {
    "use strict";

    var http = require('builtin/http');

    // function Request(inputStream) {
    //     var raw = http.readHeaders(inputStream);
    //     if (raw === null) {
    //         throw 'EOF'; // return false;
    //     }
    //     var lines = raw.split('\n');
    //     var first = lines.shift().split(/\s+/);
    //     var headers = {};
    //     lines.each(function(line) {
    //         var colon = line.indexOf(':');
    //         var key = line.substr(0, colon);
    //         var value = line.substr(colon+1).replace(/^\s+/, '');
    //         headers[key.toLowerCase()] = value;
    //     });
    //     this.headers = headers;
    //     var host = 'localhost';
    //     var uriParts = first[1].split('?');
    //     this.method = first[0];
    //     this.uri = uriParts[0];
    //     this.proto = first[2] || 'http/0.9';
    // }
    function Request(inputStream) {
        var line;
        line = inputStream.readLine();
        if (line === false) {
            throw 'EOF';
        }
        // parse 1st line
        var parts = line.split(/\s+/);
        this.method = parts[0].toUpperCase();
        this.uri = parts[1];
        this.proto = (parts[2] || 'http/0.9').toUpperCase();
        // parse headers
        var headers = {};
        var done = false;
        for (var i=0; i<64; i++) {
            line = inputStream.readLine();
            if (line === false) {
                throw 'EOF';
            }
            if (line.length === 0) {
                done = true;
                break;
            }
            parts = line.split(/\:\s+/);
            headers[parts[0].toLowerCase()] = parts[1];
        }
        while (!done) {
            line = inputStream.readLine();
            if (line === false) {
                throw 'EOF';
            }
            if (line.length === 0) {
                done = true;
                break;
            }
        }
        this.headers = headers;
    }

    exports.extend({
        Request: Request
    });

}());
