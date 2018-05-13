let gifNodeAddOn = require('../../build/release/gifNodeAddOn.node');
let fs = require('fs');
let stream = require('stream');
let config = require('./config.js');

let filenames = config.filenames;
let outputFilename = "c_row_out.gif";

let generateGIF = async function (pixelBuffers, options) {

    return new Promise(function (resolve, reject) {

        let height = options.height || 800;
        let width = options.width || 600;
        let quality = options.quality || 10;
        let interval = options.interval || 800;
        let repeat = options.repeat === true ? 0 : -1;
        let parallel = options.parallel || 0;
        
        gifNodeAddOn.picsToGIF(interval, repeat, pixelBuffers, parallel, function (err, gifBuffer) {
            
            if(err){
                console.log(err);
                reject(err);
            }else{
                
                resolve(gifBuffer);
            }
        });
    });

}


var num = 0;

async function runFun(taskId){
    
    let imgBuffers = [];
    for(let i = 0; i < filenames.length; i++){
        imgBuffers.push( fs.readFileSync(filenames[i]) );
    }

    for(let i = 0; i < 10000; ++i){
        // let parallel = (i % 2 == 0) ? 0 : 1;
        // let parallel = 0;
        let parallel = 1;
        let begin = Date.now();
        let gifBuffer = await generateGIF(imgBuffers, {interval: 10, repeat: true, parallel: parallel });
        console.log('taskId: ' + taskId + ', count: ' + i + ', parallel: ' + parallel + ', cost time: ', Date.now() - begin + ' ms');
        fs.writeFileSync('./out/out.' + taskId + '.' + i + '.gif', gifBuffer);
    }
}

runFun(0);
// runFun(1);
// runFun(2);
// runFun(3);
// runFun(4);
// runFun(5);
// runFun(1);
// runFun(2);
// runFun(3);
// runFun(4);




