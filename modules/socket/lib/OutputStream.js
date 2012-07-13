/*global require, exports */
(function() {
    "use strict";

    var Memory = require('binary').Memory;

    function OutputStream(fd) {
        this.fd = fd;
        this.buffer = new Memory(4096);
        this.size = 4096;
        this.offset = 0;
    }
    OutputStream.prototype.extend({
        destroy: function() {
            this.buffer.destroy();
        },
        write: function(s) {
            var len = s.length;
            if (this.offset + len > this.size) {
                var new_size = this.size + length + 4096;
                this.buffer.resize(new_size);
                this.size = new_size;
            }
            this.buffer.append(s, this.offset);
            this.offset += len;
        },
        flush: function() {
            if (this.offset) {
                this.buffer.write(this.fd, 0, this.offset);
                this.offset = 0;
            }
        }
    });

    exports.extend({
        OutputStream: OutputStream
    });
    
}());