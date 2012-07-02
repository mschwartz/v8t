var NUM_THREADS = 1000;
var nnn = 0;

var pthread = builtin.pthread;

log(pthread);

function Thread(fn) {
	var args = [];
	for (var i=1; i<arguments.length; i++) {
		args.push(arguments[i]);
	}
	this.fn = fn;
	this.args = args;
	this.t = pthread.spawn(this.run, this);
}
Thread.prototype.run = function(me) {
	me.fn.apply(me, me.args);
};

function thread(n) {
	var nnnn = 0;
	log('thread ' + n);
	while (true) {
		nnn++;
		nnnn++;
		log('thread ' + this.t + ' here ' + nnn + ' ' + nnnn);
	}
}

function tester(i, n) {
	log(i + ' ' + n);
	sleep(n);
	log('TESTER ' + i + ' EXITING');
}

function main() {
	var threads = [];
	if (true ) {
		log('spawning');
		new Thread(tester, 1, 5);
		new Thread(tester, 2, 5);
		new Thread(tester, 3, 6);
		for (var i=0; i<NUM_THREADS; i++) {
			// threads.push(spawn(thread, i));
			threads.push(new Thread(thread, i));
		}

		while (true) {
			log('parent hogging');
			// var tid = pthread.wait();
			// log('parent: ' + tid + ' exited');
		}
	}
	else {
		log('here');
	}
}
