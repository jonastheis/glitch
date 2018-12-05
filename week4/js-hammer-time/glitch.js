let gl;
let shader;
let framebuffer;

const KB = 1024;
const KB4 = KB * 4;
const PAGE_TEXTURE_W = 32;
const PAGE_TEXTURE_H = 32;

function glitch() {
    initGL();
    initFramebuffer();

    initDebug();

    gl.drawArrays(gl.TRIANGLES, 0, 6);

    viewFramebuffer();
}

function viewFramebuffer() {
    var pixels = new Uint8Array(KB4);
    gl.readPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
    
    for(let i=0; i<KB4-4; i+=4) {
        console.log(`${pixels[i]},${pixels[i+1]},${pixels[i+2]},${pixels[i+3]}  `);
    }
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

function initDebug() {
    
    const targetTexture = createTexture2DRGBA(null, PAGE_TEXTURE_W, PAGE_TEXTURE_H);

    // attach the texture as the first color attachment
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, targetTexture, 0);
    if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE) {
        throw new Error('Framebuffer not complete! - ' + gl.checkFramebufferStatus(gl.FRAMEBUFFER));
    }

    createRectangle();
}

function initFramebuffer() {
    framebuffer = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
}

function createRectangle() {
    const positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    // draw 2 triangles
    const positions = [
        1,  1,   // top right
        1, -1,  // bottom right
        -1, -1,  // bottom left

        -1, -1,  // bottom left
        -1,  1,   // top left
        1,  1,   // top right
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

    const vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
    
    gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(0);
}

function initGL() {
    const canvas = document.getElementById('c');
    gl = canvas.getContext('webgl2');
    if (!gl) {
        console.log('No webgl2 for you!');
        throw new Error('WebGL2 not available!');
    }

    shader = new Shader(gl, 'vertex-shader', 'fragment-shader');
    shader.use();
}

class Shader {
    constructor(gl, vertexShaderId, fragmentShaderId) {
        this.gl = gl;

        const vertexShaderSource = document.getElementById(vertexShaderId).text;
        const fragmentShaderSource = document.getElementById(fragmentShaderId).text;

        const vertexShader = this.createShader(gl.VERTEX_SHADER, vertexShaderSource);
        const fragmentShader = this.createShader(gl.FRAGMENT_SHADER, fragmentShaderSource);  
        
        this.id = this.createProgram(vertexShader, fragmentShader);
    }

    createShader(type, source) {
        const shader = this.gl.createShader(type);
        this.gl.shaderSource(shader, source);
        this.gl.compileShader(shader);
        const success = this.gl.getShaderParameter(shader, this.gl.COMPILE_STATUS);
        if (success) {
          return shader;
        }
       
        console.log(this.gl.getShaderInfoLog(shader));
        this.gl.deleteShader(shader);
    }
    
    createProgram(vertexShader, fragmentShader) {
        const program = this.gl.createProgram();
        this.gl.attachShader(program, vertexShader);
        this.gl.attachShader(program, fragmentShader);
        this.gl.linkProgram(program);
        const success = this.gl.getProgramParameter(program, this.gl.LINK_STATUS);
        if (success) {
          return program;
        }
       
        console.log(this.gl.getProgramInfoLog(program));
        this.gl.deleteProgram(program);
    }

    use() {
        this.gl.useProgram(this.id);
    }
}