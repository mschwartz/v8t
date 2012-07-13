/*
 * CommonJS require 1.1 implementation
 */

(function() {
	var fs = builtin.fs;
	var v8 = builtin.v8;
	var console = builtin.console;

	function runScript(src, fn) {
		var script = v8.compileScript(src, fn);
		var exports = v8.runScript(script);
		v8.freeScript(script);
		return exports;
	}
	function locateFile(module) {
		function tryFile(path) {
			var tryPath = fs.realpath(path);
            if (tryPath) {
                if (fs.isFile(tryPath)) {
                    return tryPath;
                }
                else if (fs.isDir(tryPath)) {
                    if (!tryPath.endsWith('/')) {
                        tryPath += '/';
                    }
                    tryPath += 'index.js';
                }
                if (fs.isFile(tryPath)) {
                    return tryPath;
                }
            }
			tryPath = fs.realpath(require.fsPath + path);
			if (tryPath) {
                if (fs.isFile(tryPath)) {
                    return tryPath;
                }
                else if (fs.isDir(tryPath)) {
                    if (!tryPath.endsWith('/')) {
                        tryPath += '/';
                    }
                    tryPath += 'index.js';
                }
                if (fs.isFile(tryPath)) {
                    return tryPath;
                }
            }
			return false;
		}
		if (module.substr(0,1) == '/' || module.substr(0,2) == './' || module.substr(0,3) == '../') {
			return tryFile(module) || tryFile(module + '.js' || tryFile(module + '.coffee'));
		}
		else {
			var paths = require.path;
			for (var i=0, len=paths.length; i<len; i++) {
				var path = paths[i];
				if (path.substr(path.length-1, 1) != '/') {
					path += '/';
				}
				path += module;
				var found = tryFile(path) || tryFile(path+'.js') || tryFile(path + '.coffee');
				if (found) {
					return found;
				}
			}
		}
		throw new Error('Could not locate require file ' + module);
	}
	// coffeescript support
	var suffix = '.coffee',
		suffixLen = suffix.length;

	function loadFile(modulePath) {
		var contents = fs.readFile(modulePath);
		if (modulePath.indexOf(suffix, modulePath.length - suffixLen) !== -1) {
			contents = CoffeeScript.compile(contents, { bare: true });
		}
		return contents;
	}
	require = function(module) {
		if (module.substr(0, 8) == 'builtin/') {
			var m = builtin[module.substr(8)];
			return m;
		}
		var modulePath = locateFile(module);
		if (require.cache[modulePath]) {
			return require.cache[modulePath];
		}
		var content = loadFile(modulePath);
		require.dirStack.push(require.fsPath);
		var fsPath = modulePath.split('/');
		fsPath.pop();
		require.fsPath = fsPath.join('/') + '/';
		var exports = require.cache[modulePath] = {};
		var script = [
			'(function() {',
			'	var exports = {}', //  module.exports;',
			'	var module = {',
			'		id: "' + module + '",',
			'		url: "' + modulePath + '"',
			'	};',
			content,
			'	return exports;',
			'}())'
		].join('\n');
		// prepare function arguments
		var context = {};
		var _module = { id: module, uri: modulePath };
		require.cache[modulePath] = runScript(script, modulePath);
//		content += '\nreturn exports;';
//		var f = new Function("require", "exports", "module", content);
//		require.cache[modulePath] = f.call({}, require, exports, { id: module, uri: modulePath, exports: exports });
		require.fsPath = require.dirStack.pop();
		return require.cache[modulePath];
	};

    require.isRequiredFile = function(fn) {
        return require.cache[fn] ? true : false;
    },
    
	require.main = this;
	require.dirStack = [];
	require.fsPath = '';
	require.cache = {};
	require.path = [
		'./',
		'modules',
        '/usr/local/silkjs',
        '/usr/local/silkjs/modules',
        '/usr/local/silkjs/contrib',
		'/usr/share/silkjs/modules'
	];

}());
