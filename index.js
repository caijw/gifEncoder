var gifNodeAddOn = require('./build/release/gifNodeAddOn.node');


exports.picsToGIF = async function (picBuffers, options) {

    return new Promise(function (resolve, reject) {

        let quality = options.quality || 10;
        let interval = options.interval || 800;
        let repeat = options.repeat === true ? 0 : -1;
        
        gifNodeAddOn.picsToGIF(interval, repeat, picBuffers, function (err, gifBuffer) {
            
            if(err){
                reject(err);
            }else{
                resolve(gifBuffer);
            }
        });
    });

};