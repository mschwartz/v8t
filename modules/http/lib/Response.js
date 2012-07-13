/*global require, exports */
(function() {
    "use strict";

    var responseCodeText = {
        100: 'Continue',
        101: 'Switching Protocols',
        200: 'OK',
        201: 'Created',
        202: 'Accepted',
        203: 'Non-Authoritative Information',
        204: 'No Content',
        205: 'Reset Content',
        206: 'Partial Content',
        300: 'Multiple Choices',
        301: 'Moved Permanently',
        302: 'Found',
        303: 'See Other',
        304: 'Not Modified',
        305: 'Use Proxy',
        307: 'Temporary Redirect',
        400: 'Bad Request',
        401: 'Unauthorized',
        402: 'Payment Required', // note RFC says reserved for future use
        403: 'Forbidden',
        404: 'Not Found',
        405: 'Method Not Allowed',
        406: 'Not Acceptable',
        407: 'Proxy Authentication Required',
        408: 'Request Timeout',
        409: 'Conflict',
        410: 'Gone',
        411: 'Length Required',
        412: 'Precondition Failed',
        413: 'Request Entity Too Large',
        414: 'Request-URI Too Long',
        415: 'Unsupported Media Type',
        416: 'Request Range Not Satisfiable',
        417: 'Expectation Failed',
        500: 'Internal Server Error',
        501: 'Not Implemented',
        502: 'Bad Gateway',
        503: 'Service Unavailable',
        504: 'Gateway Timeout',
        505: 'HTTP Version Not SUpported'
    };

    function Response(outputStream, proto) {
        this.outputStream = outputStream;
        this.headersSent = false;
        this.status = 200;
        this.contentLength = 0;
        this.contentType = 'text/html';
        this.cookies = {};
        this.headers = {};
        this.proto = proto;
    }
    Response.prototype.extend({
        destroy: function() {
            this.outputStream.flush();
            this.outputStream.destroy();
        },
        writeHead: function(status, headers) {
            this.headers.extend(headers);
            this.status = status;
        },
        end: function(s) {
            var out = this.outputStream;
            out.write(this.proto + ' ' + this.status + ' ' + responseCodeText[this.status] + '\r\n');
            out.write('Date: ' + new Date().toGMTString() + '\r\n');
            for (var key in this.headers) {
                if (this.headers.hasOwnProperty(key)) {
                    var value = this.headers[key];
                    out.write(key + ': ' + value + '\r\n');
                }
            }
            out.write('Content-Length: ' + s.length + '\r\n\r\n');
            out.write(s);
            out.flush();
        }
    });

    exports.extend({
        responseCodeText: responseCodeText,
        Response: Response
    });
}());
