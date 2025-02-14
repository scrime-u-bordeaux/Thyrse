// var car2geo = [];
var oneOverPi = 1 / Math.PI;
var twoOverPi = 2 / Math.PI;

/*
function generateTransferFunction(npts) {
	car2geo = [];
	
	// max distance is 2 between pts on the unit sphere
	var step = 2 / (npts - 1);
	var dc = 0;
	
	for (var i = 0; i < npts; ++i) {
		dc = i * step;
    }	
	
}
*/

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

function spherToCar(azi, ele) {
	var res = sphericalToCartesian([azi, ele]);
	outlet(0, "ccoords", res);
}

function cartesianToGeodesic(dc) {
	dc = Math.min(Math.max(dc, -2), 2);
	var dg = Math.atan2(dc * 0.5, Math.sqrt(1 - (dc * dc) * 0.25));
	outlet(0, "geo", dg * twoOverPi);
}