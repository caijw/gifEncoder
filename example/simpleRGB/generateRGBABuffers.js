let seeds = [
	[0, 0, 0, 0],  /*black*/
	[255, 0, 0, 0],	/*red*/
	[0, 255, 0, 0], /*green*/
	[0, 0, 255, 0] /*blue*/
];

let width = 1000;
let height = 1000;
let delay = 1000;

exports.getBuffers = () => {

	let buffers = [];

	for(let i = 0; i < seeds.length; i++){

		let seed = seeds[i];

		let buffer = Buffer.alloc(seed.length * width * height);

		for(let j = 0; j < height; j++){

			for(let k = 0; k < width; k++){

				for(let l = 0; l < seed.length; l++){

					// console.log(i, j*width*seed.length + k * seed.length +l)

					buffer[ j*width*seed.length + k * seed.length +l] = seed[l]

				}

			}
		}
		buffers.push(buffer);
	}

	return {
		width: width,
		height: height,
		delay: delay,
		buffers: buffers
	}

}

