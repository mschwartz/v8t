/*global require, exports */
(function() {
    "use strict";

    var Memory = require('binary').Memory,
        console = require('console');

    function InputStream(fd) {
        this.fd = fd;
        this.eof = false;
        this.buffer = new Memory(4096);
        this.offset = 0;
        this.end = 0;
    }
    InputStream.prototype.extend({
        destroy: function() {
            this.buffer.destroy();
        },
        read: function() {
            if (this.eof) {
                throw 'EOF';
            }
            if (this.offset >= this.end) {
                this.offset = 0;
                this.end = 0;
                while (this.end === 0) {
                    this.end = this.buffer.read(this.fd, 0, 4096);
                    if (this.end <= 0 || this.end === null) {
                        this.eof = true;
                        throw 'EOF';
                    }
                }
            }
            return this.buffer.getAt(this.offset++);
        },
        readLine: function() {
            // console.dir('readLine');
            var line = '',
                inp;
            while ((inp = this.read())) {
                // console.dir('line = "' + line + '"');
                switch (inp) {
                    case '\r':
                        continue;
                    case '\n':
                        return line;
                    default:
                        line += inp;
                }
            }
            this.eof = true;
            throw 'EOF';
        }
    });

    exports.extend({
        InputStream: InputStream
    });

}());