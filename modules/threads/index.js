(function() {
    exports.extend({
        Thread: require('lib/Thread').Thread,
        Mutex: require('lib/Mutex').Mutex,
        Condition: require('lib/Condition').Condition
    });
}());