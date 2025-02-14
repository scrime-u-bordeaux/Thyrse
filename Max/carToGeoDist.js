cartesianToGeodesic.local = 1;
function cartesianToGeodesic(dc) {
	dc = Math.min(Math.max(dc, -2), 2);
	var dg = Math.atan2(dc * 0.5, Math.sqrt(1 - (dc * dc) * 0.25));
	return dg;
	//outlet(0, "geo", dg * twoOverPi);
}

function list() {
	outlet(0, cartesianToGeodesic(arrayfromargs(arguments)));
}