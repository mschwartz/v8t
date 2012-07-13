/*global require, exports */
(function() {
    "use strict";

    exports.extend({
        Socket: require('lib/Socket').Socket,
        InputStream: require('lib/InputStream').InputStream,
        OutputStream: require('lib/OutputStream').OutputStream
    });

}());
