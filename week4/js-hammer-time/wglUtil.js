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

  return texture;
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
