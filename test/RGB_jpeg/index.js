const fs = require('fs');
const path = require('path');

const gifEncoder = require('./../../index.js');
let imgBuffers = [];

imgBuffers.push(fs.readFileSync(path.resolve(__dirname, './img/in/blue.jpeg') ));
imgBuffers.push(fs.readFileSync(path.resolve(__dirname, './img/in/green.jpeg') ));
imgBuffers.push(fs.readFileSync(path.resolve(__dirname, './img/in/red.jpeg') ));

console.log(imgBuffers[0].length);
console.log(imgBuffers[1].length);
console.log(imgBuffers[2].length);

gifEncoder.picsToGIF(imgBuffers, {}).then(function (gifBuffer) {
	var tmp = [];
	for(var i = 0; i < 100; ++i){
		tmp.push(gifBuffer[i]);
	}
	console.log(tmp.join(' '));
	fs.writeFileSync(path.resolve(__dirname, './img/out/out.gif'), gifBuffer);

}).catch(function (err) {
	throw err;
});