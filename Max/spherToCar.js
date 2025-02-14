// in 3 dimensions this returns z, x, y :
// z = r * cos(theta)
// x = r * sin(theta) * cos(phi)
// y = r * sin(theta) * sin(phi)

// also :
// angles[0] is elevation
// angles[1] is azimuth (last angle is the only one that ranges from 0 to 2π)

sphericalToCartesian.local = 1;
function sphericalToCartesian(angles) {
	if (!Array.isArray(angles)) return null;
	if (angles.length < 1) return null;
	
	var sinProduct = 1;
	var res = [];
	
	for (var i = 0; i < angles.length; ++i) {
		res.push(sinProduct * Math.cos(angles[i]));
		sinProduct *= Math.sin(angles[i]);
	}
	
	res.push(sinProduct);
	return res;
}

function list() {
	outlet(0, sphericalToCartesian(arrayfromargs(arguments)));
}