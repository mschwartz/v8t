(function() {
    var pthread = require('builtin/pthread');

    function Condition() {
        this._cond = pthread.init_cond();
    }
    Condition.prototype.extend({
        destroy: function() {
            if (this._cond) {
                pthread_destroy_cond(this._cond);
                this._cond = null;
            }
        },
        wait: function() {
            if (!this._cond) {
                throw new Error('Condition not initialized');
            }
            return pthread.cond_wait(this._cond);
        },
        signal: function() {
            if (!this._cond) {
                throw new Error('Condition not initialized');
            }
            return pthread.cond_signal(this._cond);
        },
        broadcast: function() {
            if (!this._cond) {
                throw new Error('Condition not initialized');
            }
            return pthread.cond_broadcast(this._cond);
        }
    });
    exports = Condition;
}());
