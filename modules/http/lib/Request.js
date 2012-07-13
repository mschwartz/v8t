/*global require, exports */
(function() {
    "use strict";

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
