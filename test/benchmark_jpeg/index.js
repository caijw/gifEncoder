const gifEncoder = require('./../../index.js');
let path = require('path');
let fs = require('fs');
let stream = require('stream');
let config = require('./config.js');

let filenames = config.filenames;
let outputFilename = config.outputGifFilename;

    
let imgBuffers = [];
for(let i = 0; i < filenames.length; i++){
    imgBuffers.push( fs.readFileSync( path.resolve(__dirname, filenames[i]) ) );
}


for(let i = 0; i < 10; ++i){
	(function (i) {
		let begin = Date.now();
		gifEncoder.picsToGIF(imgBuffers, {delay: 10, repeat: true, parallel: false }).then(function (gifBuffer) {
			let end = Date.now();
			console.log("finish task: " + i + ', cost :' + (end - begin) + 'ms');
		    fs.writeFileSync(path.resolve(__dirname, './img/out/out-' + i + '.gif'), gifBuffer);
		}).catch(function (err) {
		    throw err;
		});
	})(i)
}








