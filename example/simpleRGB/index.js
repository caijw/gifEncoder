var gifNodeAddOn = require('../index.js')
var RGBABuffers = require('./generateRGBABuffers.js').getBuffers()
var fs = require('fs')




gifNodeAddOn.RGBAToGIF(RGBABuffers.width, RGBABuffers.height, RGBABuffers.delay, RGBABuffers.buffers).then(function (buf) {
	fs.writeFileSync('./test.gif', buf);
});
