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
    initFramebuffer();

    // enable shaders too be used
    shader = new Shader('vertex-shader', 'fragment-shader');
    shader.use();


    // create debug requirements
    initDebug();
    prepareHammerTime();
    gl.drawArrays(gl.TRIANGLES, 0, 6);
    viewFramebuffer();
}

function prepareHammerTime() {
    // TODO: this is only for testing purposes
    const textures = [];
    for(let i=0; i<5; i++) {
        const t = createTexture2DRGBA(createUint8Array(KB4, i), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
        textures.push(t);
        shader.bindTexture(t, i, 'H', i);
    }
    console.log(textures);

    // TODO: find out why bound textures are shifted by 1
}

function createUint8Array(elements, fill=0x00) {
    const arr = new Uint8Array(elements);

    for(let i=0; i<elements; i++) {
        arr[i] = fill;
    }

    console.log(arr);
    return arr;
}