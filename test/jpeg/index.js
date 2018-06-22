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


let begin = Date.now();

gifEncoder.picsToGIF(imgBuffers, {delay: 10, repeat: true, parallel: false }).then(function (gifBuffer) {
    fs.writeFileSync(path.resolve(__dirname, outputFilename), gifBuffer);
}).catch(function (err) {
    throw err;
});









