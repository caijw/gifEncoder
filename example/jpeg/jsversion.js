let fs = require('fs');
let JPEG = require('jpeg-js');
let GIFEncoder = require('gifencoder');
let stream = require('stream');
let config = require('./config.js');

let filenames = config.filenames;
let outputFilename = "js_out.gif";



const generateGIF = (pixelBuffers, options) => {

    return new Promise(resolve => {
        
        const height = options.height || 800
        const width = options.width || 600
        const quality = options.quality || 10
        const interval = options.interval || 800
        const repeat = options.repeat === true ? 0 : -1

        let gifBuf = Buffer.alloc(0)
        const encoder = new GIFEncoder(width, height)
        const readStream = encoder.createReadStream()

        readStream.on('data', data => {
            
            gifBuf = Buffer.concat([gifBuf, data]);

        })
        readStream.on('end', _ => {
            
            resolve(gifBuf)
        })

        encoder.start()
        encoder.setRepeat(repeat)
        encoder.setDelay(interval)
        encoder.setQuality(quality)

        pixelBuffers.forEach(pixelBuffer => {
            encoder.addFrame(pixelBuffer)
        })

        encoder.finish()
    })
}
var num = 0;
async function runFun(){

	let jpgBuffers = [];
	for(let i = 0; i < filenames.length; i++){
		
		jpgBuffers.push( fs.readFileSync(filenames[i]) );
	}

	let pixelBuffers = [];
	for(let i = 0; i < jpgBuffers.length; i++){
		pixelBuffers.push(JPEG.decode(jpgBuffers[i]).data);
	}
    for(var i = 0; i < 10000; i++){
        
            let gifBuffer = await generateGIF(pixelBuffers, {width: 820, height: 620, interval: 100, repeat: true});
            // console.log(...gifBuffer)
            // fs.writeFileSync(outputFilename, gifBuffer);
            console.log('finish encode', num++);
    }


}


runFun()


