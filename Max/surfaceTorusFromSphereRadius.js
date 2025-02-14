// NB : we consider the right triangle inscribed
// in the unit circle (projection of the unit sphere),
// formed by the source's sphere center, its
// diametrically opposite point, and a point of the circle
// resulting in the intersection of the unit sphere and the
// source's sphere : the torus we want to display.
// tr is this triangle's height, orthogonally crossing
// the unit sphere's diameter

// to understand how we compute tr, see :
// https://www.youtube.com/watch?v=_2n4bougty4
// and notably @harikatragadda's comment
// then td is simply derived from pythagore

var globalScale = 1;
var sourceRadius = 0;

function global_scale(s) {
	globalScale = s;
	bang();
}

function msg_float(f) {
	sourceRadius = f;
	bang();
}

function bang() {
	var sr = sourceRadius;
	var r = 1;
	var diam = 2 * r;
	var xSquared = diam * diam - sr * sr;
	var x = Math.sqrt(xSquared);
	var tr = sr * x / diam;
	var td = Math.sqrt(xSquared - tr * tr) - r;
	
	outlet(0, "radius", tr);
	outlet(0, "distance", td);
}
