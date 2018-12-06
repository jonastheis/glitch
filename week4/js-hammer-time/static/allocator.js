class Allocator {
  constructor(pages) {
    this.pages = pages;
    
    this.initKGSL = []
    this.initKGSLIds = []
    this.newKGSL = []

    this.textures = [] 
  }

  async _init(safe) {
    console.log(`[Allocator] ++ Fetching ground truth values.`);
    this.initKGSL = await this.getKGSL()
    this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

    console.log(`[Allocator] ++ ${this.initKGSLIds.length} textures already existed. generating ${this.pages} page-textures.`);
    
    safe ? await this.allocateSafe() : await this.allocate()
    
    // filter out thos that already existed from before.
    this.newKGSL = await this.getKGSL()
    this.newKGSL = this.newKGSL.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)
    
    
    console.log(`[Allocator] ++ ${this.newKGSL.length} unique new textures found.`);
    for (let i = 0; i < this.textures.length; i++) {
      console.log(this.newKGSL[i])
    }
    
    return Promise.resolve(true)
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
    for (let i = 0; i < this.textures[i]; i++) {
      gl.deleteTexture(this.textures[i]);
    }
  }

  async getKGSL() {
    return await fetch('get_tex_infos').then(data => data.json())
  }
}