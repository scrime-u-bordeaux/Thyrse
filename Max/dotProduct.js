dotProduct.local = 1;
function dotProduct(u, v) {
	var res = 0;
	for (var i = 0; i < u.length; ++i) {
		res += u[i] * v[i];
	}
	return res;
}

function list() {
	var a = arrayfromargs(arguments);
	if (a.length % 2 !== 0) return;
	outlet(0, dotProduct(a.slice(0, a.length / 2), a.slice(a.length / 2)));
}