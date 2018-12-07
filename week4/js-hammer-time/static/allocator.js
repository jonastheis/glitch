
class Allocator {
  constructor(pages) {
    this.pages = pages;
    
    this.initKGSL = []
    this.initKGSLIds = []
    this.newKGSL = []

    this.textures = []
    this.maxIdSearched = 0
  }

  async _init(safe) {
    console.log(`[Allocator] ++ Fetching ground truth values.`);
    this.initKGSL = await this.getKGSL()
    this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

    console.log(`[Allocator] ++ ${this.initKGSLIds.length} textures already existed. generating ${this.pages} page-textures. [${this.pages*KB4/MB} MB]`);
    
    safe ? await this.allocateSafe() : await this.allocate()
    console.debug('[Allocator] ++ Allocation Done.')
    
    console.debug('[Allocator] ++ Fetching new KGSL List.')
    // filter out thos that already existed from before.
    this.newKGSL = await this.getKGSL()
    this.newKGSL = this.newKGSL.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)
    
    
    console.log(`[Allocator] ++ ${this.newKGSL.length} unique new textures found.`);
    if (this.newKGSL.length !== this.pages) {
      this.clean()
      console.warn(`[Allocator] wrong texture filters Expected [${this.pages}] != [Got ${this.newKGSL.length}]`)
      console.warn(`[Allocator] Asusming that ${this.newKGSL.length} pages are created.`)
      this.pages = this.newKGSL.length
    }

    // modifies this.newKGSL
    this.sortSelf()
    
    // add the original index of all textures 
    for (let i = 0; i < this.pages; i++) {
      this.newKGSL[i].texture = this.textures[i]
    }

    return Promise.resolve(true)
  }

  SearchContPages() {
    console.log(`[Allocator] ++ Searching for ${MIN_CONTIMUOUS_PAGES} pages.`);
    let contPageChunks = []
    for (let i = 0; i < this.pages - MIN_CONTIMUOUS_PAGES; i++) {
      let found = 1
      let j;
      for (j = 0; j < MIN_CONTIMUOUS_PAGES; j++) {
        if (this.newKGSL[i].pfn+j !== this.newKGSL[i+j].pfn ) {
          found = 0
          break 
        }
      }
      this.newKGSL[i].alloc_order = j
      if (found) {
        let tmp = []
        for (let k = 0; k < j; k++) {
          tmp.push(this.newKGSL[i+k])
        }
        console.debug(`[Allocator] ++ found a page at index ${i}`)
        contPageChunks.push(tmp) 
        i += MIN_CONTIMUOUS_PAGES-1
      }
    }
    return contPageChunks
  }

  sortSelf() {
    this.newKGSL = this.newKGSL.sort((a, b) => {
      if (a.tex_id < b.tex_id) {
        return -1
      }
      else if (a.tex_id > b.tex_id) {
        return 1
      }
      else {
        return 0
      }
    })
  }
  
  async allocateSafe() {
    for (let i = 0; i < this.pages; i++) {
      this.initKGSL = await this.getKGSL()
      this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

      const t = createTexture2DRGBA(createUint8Array(KB4, i), PAGE_TEXTURE_W, PAGE_TEXTURE_H);

      let newKGSLList = await this.getKGSL()
      console.log(newKGSLList);
      newKGSLList = newKGSLList.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)
      
      if ( newKGSLList.length !== 1 ) {
        console.error("[Allocator] ++ More than one page is created?...")
        break
      }
      else {
        console.log(`[Allocator] ++ [${i}/${this.pages}]new verified page created', newKGSLList[0`);
        this.newKGSLSafe.push(newKGSLList[0])
        this.textures.push(t);
      }
    }
    return Promise.resolve(1)
  }

  allocate() {
    for (let i = 0; i < this.pages; i++) {
      const t = createTexture2DRGBA(createUint8Array(KB4, i), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
      this.textures.push(t);
    }
    return Promise.resolve(1)
  }

  clean() {
    for (let i = 0; i < this.textures[i].length; i++) {
      gl.deleteTexture(this.textures[i]);
    }
  }

  async getKGSL() {
    return await fetch('get_tex_infos').then(data => data.json())
  }
}