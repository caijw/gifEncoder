let gifNodeAddOn = require('../../build/release/gifNodeAddOn.node');
let fs = require('fs');
let JPEG = require('jpeg-js');
let GIFEncoder = require('gifencoder');
let stream = require('stream');
let config = require('./config.js');

let filenames = config.filenames;
let outputFilename = "c_out.gif";

let generateGIF = async function (pixelBuffers, options) {

    return new Promise(function (resolve, reject) {

        let height = options.height || 800;
        let width = options.width || 600;
        let quality = options.quality || 10;
        let interval = options.interval || 800;
        let repeat = options.repeat === true ? 0 : -1;
        
        gifNodeAddOn.RGBAToGIF(width, height, interval, repeat, pixelBuffers, function (err, gifBuffer) {
            
            if(err){
                reject(err);
            }else{
                resolve(gifBuffer);
            }
        });
    });

}



async function runFun(){

    let jpgBuffers = [];
    for(let i = 0; i < filenames.length; i++){
        jpgBuffers.push( fs.readFileSync(filenames[i]) );
    }

    let pixelBuffers = [];
    for(let i = 0; i < jpgBuffers.length; i++){
        pixelBuffers.push(JPEG.decode(jpgBuffers[i]).data);
    }

    let gifBuffer = await generateGIF(pixelBuffers, {width: 820, height: 620, interval: 100, repeat: true});

    fs.writeFileSync(outputFilename, gifBuffer);
}


runFun().then(function () {
    
}).catch(function (err) {
    console.log(err);
})




