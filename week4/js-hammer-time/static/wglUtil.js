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

function initGL() {
  const canvas = document.getElementById('c');
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
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
  if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE) {
      throw new Error('Framebuffer not complete! - ' + gl.checkFramebufferStatus(gl.FRAMEBUFFER));
  }

  var pixels = ARRAY_UINT8_READ_TEXTURE;
  gl.readPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, pixels);

  for (let i = 0; i < KB4; i++) {
    if(pixels[i] != 0xFF) {
      console.log(`++++ BIT FLIP IDENTIFIED`);
      console.log(`Value: ${pixels[i]}, byte offset: ${i}`);
      return;
    }
  }
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

function allocatePages(pages) {
  console.log(`++ Bulk allocating ${pages} pages | ${pages*KB4 / MB} MB of memory `)
  for (let i = 0; i < pages; i++) {
    allocatePage()
  }
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}