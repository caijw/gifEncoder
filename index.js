const common = require('./common/common.js');
const gifNodeAddOn = require(`./build/${common.buildType}/gifEncoder.node`);


exports.picsToGIF = async function (picBuffers, options) {

    return new Promise(function (resolve, reject) {

        let quality = options.quality || 10;
        let delay = options.delay || options.interval || 100;  //elay for animation in hundredths of a second
        let repeat = options.repeat === false ? -1 : 0;
        let parallel = options.parallel === true ? true : false;
        //delay(Number), repeat(Bool), quality(Number), buffers(Array of Buffer), parallel(Bool), callback(Function)
        gifNodeAddOn.picsToGIF(delay, repeat, quality, picBuffers, parallel, function (err, gifBuffer) {
            if(err){
                reject(err);
            }else{
                resolve(gifBuffer);
            }
        });
    });

};