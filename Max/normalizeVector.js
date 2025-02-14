function list() {
	var norm = 0;
	var res = arrayfromargs(arguments);
	// var res = a.slice(0);
	
	for (var i = 0; i < res.length; ++i) {
		norm += res[i] * res[i];
	}
	
	var oneOverNorm = 1 / Math.sqrt(norm);
	
	for (var i = 0; i < res.length; ++i) {
		res[i] *= oneOverNorm;
	}
	
	outlet(0, res);
}