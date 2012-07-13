/*global require, exports */

(function() {
    "use strict";

    var console = require('console'),
        mem = require('builtin/mem');

    /**
     * @param {int} p - size of memory block.
     * @param {Memory} p - existing memory block to clone.
     */
    function Memory(p) {
        if (typeof p === 'number') {
            this.handle = mem.alloc(p);
            this.length = p;
        }
        else {
            this.handle = mem.dup(p);
            this.length = mem.size(this.handle);
        }
    }
    Memory.prototype.extend({
        destroy: function() {
            mem.free(this.handle);
        },
        resize: function(newSize) {
            this.handle = mem.realloc(this.handle, newSize);
            if (!this.handle) {
                throw new Error('Could not resize memory');
            }
            this.length = newSize;
        },
        append: function(s, offset) {
            return mem.append(this.handle, s, offset, s.length);
        },
        getAt: function(offset) {
            return mem.getAt(this.handle, offset);
        },
        substr: function(offset, size) {
            offset = offset || 0;
            size = size || (this.length - offset);
            return new Memory(mem.substr(this.handle, offset, size));
        },
        asString: function(offset, size) {
            offset = offset || 0;
            size = size || (this.length - offset);
            return mem.asString(this.handle, offset, size);
        },
        read: function(fd, offset, size) {
            offset = offset || 0;
            size = size || this.length;
            return mem.read(this.handle, fd, offset, size);
        },
        write: function(fd, offset, size) {
            offset = offset || 0;
            size = size || this.length;
            return mem.write(this.handle, fd, offset, size);
        }
    });

    exports.extend({
        Memory: Memory
    });
}());