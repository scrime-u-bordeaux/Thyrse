const fs = require('fs');
const path = require('path');
const Max = require('max-api');

let rootPath = undefined;
let moons = undefined;
let currentMoon = undefined;
let currentLangue = undefined;

// convert time stamps in hh:mm:ss.msmsms format to a single ms value
const hms2ms = (dateStr) => {
	const hms = dateStr.split(':');
	if (hms.length !== 3) {
		return -1;
	}
	
	const [h, m, s] = hms;
	const res = parseInt(h) * 3.6e+6 + parseInt(m) * 6e+4 + parseInt(s * 1000);
	return res;
}


const processFiles = () => {
	if (moons !== undefined) {
		Max.outlet('polybuffer', 'clear');
		Max.outlet('coll', 'clear');
		let currentBuffer = 1;
		
		Object.keys(moons).forEach(moon => {
			Object.keys(moons[moon].langues).forEach(l => {
				// now validate csv + wav files naming and contents :
				const filenames = moons[moon].langues[l];
				const filesByBasename = {};
				const csvInfoByBasename = {};
				const validBasenames = {
					info: {
						maxseg: 0,
					},
					entries: {},
				};
				
				// check if we have .csv and .wav files with common basenames
				for (let i = 0; i < filenames.length; ++i) {
					const ext = path.extname(filenames[i]);
					const basename = path.basename(filenames[i], ext);
					
					if (filesByBasename[basename] === undefined) {
						filesByBasename[basename] = [ ext ];
					} else {
						filesByBasename[basename].push(ext);
					}
				}
				Object.keys(filesByBasename).forEach(k => {
					if (filesByBasename[k].indexOf('.csv') !== -1 &&
						filesByBasename[k].indexOf('.wav') !== -1) {
						validBasenames.entries[k] = {
							segments: [],
						};
					}
				});
				
				// inspect all csv files and verify they have the same nb of lines
				// (the same nb of consecutive marker pairs i.e segments)
				Object.keys(validBasenames.entries).forEach(k => {
					const lines = fs.readFileSync(
						path.join(rootPath, moons[moon].folder, l, `${k}.csv`),
						{ encoding: 'utf-8' }
					)
					.split(/\r?\n/)
					.filter(line => line.length > 0);
					
					if (lines.length % 2 === 0) {
						// ok, we have a pair number of markers
						const nseg = lines.length / 2;
						
						for (let i = 0; i < lines.length; i+=2) {
							const start = lines[i].split(',');
							const end = lines[i+1].split(',');
							validBasenames.entries[k].segments.push([
								hms2ms(start[0]),
								hms2ms(end[0])
							]);
						}

						const { maxseg } = validBasenames.info;
						validBasenames.info.maxseg = maxseg > nseg ? maxseg : nseg;
					}
				});
				
				Object.keys(validBasenames.entries).forEach(k => {
					if (validBasenames.entries[k].segments.length < validBasenames.info.maxseg) {
						console.log('deleting entry ' + k);
						delete validBasenames.entries[k];
					}
				});
				
				moons[moon].langues[l] = validBasenames;
			});
			
			let maxseg = 0;
			Object.keys(moons[moon].langues).forEach(l => {
				if (moons[moon].langues[l].info.maxseg > maxseg) {
					maxseg = moons[moon].langues[l].info.maxseg;
				}
			});
			Object.keys(moons[moon].langues).forEach(l => {
				if (moons[moon].langues[l].info.maxseg < maxseg ||
					Object.keys(moons[moon].langues[l].entries).length === 0) {
					console.log('deleting idiom ' + l);
					delete moons[moon].langues[l];
				} else {
					Object.keys(moons[moon].langues[l].entries).forEach(k => {
						const collEntry = path.join(moons[moon].folder, l, `${k}`);
						const filePath = path.join(rootPath, `${collEntry}.wav`);
						Max.outlet('polybuffer', 'append', filePath);
						Max.outlet('coll', 'store', collEntry, `traductions.${currentBuffer}`, l, k);
						currentBuffer++;
					});
				}
			});
			moons[moon].nseg = maxseg;
		});
		
		Max.outlet('polybuffer', 'done');
		Max.outlet('coll', 'done');
	}
};

Max.addHandler('rootPath', (p) => {
	rootPath = p;
	moons = {};

	fs.readdir(rootPath, { recursive: true }, function(err, list) {
		list.sort((a,b) => a.localeCompare(b));

		for (let i = 0; i < list.length; ++i) {
			const chunks = list[i].split(path.sep)
			const moon = chunks[0];
			const moonIndex = /^\d{2}\s$/.test(moon.substr(0,3))
							? parseInt(moon.substr(0,2))
							: -1;

			switch (chunks.length) {
				case 1:
				{
					if (moonIndex > -1) {
						moons[moonIndex] = {
							lune: moon.substr(3, moon.length - 1),
							folder: moon,
							nseg: 0,
							langues: {},
						};
					}
				}
				break;

				case 2:
				{
					const [ moon, idiom ] = chunks;
					if (moons[moonIndex] !== undefined && idiom !== '.DS_Store') {
						moons[moonIndex].langues[idiom] = [];
					}
				}
				break;

				case 3:
				{
					const [ moon, idiom, filename ] = chunks;
					if (moons[moonIndex] !== undefined
						&& moons[moonIndex].langues[idiom] !== undefined
						&& moons[moonIndex].langues[idiom] !== '.DS_Store') {
						moons[moonIndex].langues[idiom].push(filename);
						// deal with csv + wav existence here ?
					}
				}
				break;

				default:
				break; // ignore
			}			
		}
		
		processFiles();
		
		console.log(JSON.stringify(moons, null, 2));

		//const lunes = [ 'lunes' ];
		Max.outlet('lunes', 'clear');
		Object.keys(moons).forEach(k => {
			//lunes.push(moons[k].folder);
			Max.outlet('lunes', 'append', moons[k].folder);
		});
		Max.outlet('lunes', 'done');
	});
});

Max.addHandler('number', (i) => {
	const moonIndex = parseInt(i);
	
	if (moons !== undefined && moons[moonIndex] !== undefined) {
		currentMoon = moonIndex;
		const moon = moons[moonIndex];
		Max.outlet('lunes', 'nseg', moon.nseg);
		Max.outlet('langues', 'clear');
		Object.keys(moon.langues).forEach(k => {
			Max.outlet('langues', 'append', k);
		});
		Max.outlet('langues', 'done');
	}
});

Max.addHandler('langue', (dest, langue) => {
	currentLangue = langue;
	
	if (currentMoon !== undefined && moons[currentMoon] !== undefined) {
		const moon = moons[currentMoon];
		Max.outlet('basenames', dest, 'clear');
		let nseg = 0;
		if (moon.langues[langue] !== undefined) {
			nseg = moon.langues[langue].info.maxseg;
			Object.keys(moon.langues[langue].entries).forEach(basename => {
				Max.outlet('basenames', dest, 'append', basename);
			});
		}
		Max.outlet('basenames', dest, 'nseg', nseg);
		Max.outlet('basenames', dest, 'done');
	}
});

Max.addHandler('segment', (langue, basename, segmentIndex) => {
	if (moons[currentMoon] !== undefined) {
		const moon = moons[currentMoon];
		const langues = moon.langues[langue];
		console.log(JSON.stringify(langues, null, 2));
		
		if (segmentIndex < langues.info.maxseg) {
			const entry = langues.entries[basename];
			const id = path.join(moon.folder, langue, basename);
			const markers = entry.segments[segmentIndex];
			Max.outlet('player', id, markers[0], markers[1]);
			//Max.outlet('audiofiles', 'markers', markers[0], markers[1]);
		}
	}
});