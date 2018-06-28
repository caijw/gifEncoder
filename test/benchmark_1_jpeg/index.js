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

var job = async function () {
	for(var i = 0 ; i < 30000; ++i){
		try{
			let begin = Date.now();
			let gifBuffer = await gifEncoder.picsToGIF(imgBuffers, {delay: 10, repeat: true, parallel: false });
			let end = Date.now();
			console.log("finish task: " + i + ', cost :' + (end - begin) + 'ms');
		    // fs.writeFileSync(path.resolve(__dirname, './img/out/out-' + i + '.gif'), gifBuffer);

		}catch(e){
			throw e;
		}

	}
};

job();









