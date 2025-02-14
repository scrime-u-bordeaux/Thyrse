crossProduct.local = 1;
function crossProduct(u, v) {
	return [
		u[1] * v[2] - u[2] * v[1],
		u[2] * v[0] - u[0] * v[2],
		u[0] * v[1] - u[1] * v[0]
	];
}

function list() {
	var a = arrayfromargs(arguments);
	if (a.length !== 6) return;
	outlet(0, crossProduct(a.slice(0, 3), a.slice(3)));
}