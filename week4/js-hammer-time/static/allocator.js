class Allocator {
  constructor(pages) {
    this.pages = pages;
    this.SLEEP = 1000
    this.SLEEP_STEP = PAGES_PER_MB * 32
    this.JS_ARRAY_MB = 8
    
    this.initKGSL = []
    this.initKGSLIds = []
    this.newKGSL = []

    this.textures = []
    this.offset = 0
  }

  async init_incremental(start) {
    let cont_found = 0
    let pages_to_allocate = start
    while (!cont_found) {
      console.log(`[Allocator] ++ attempting to fill memory with ${pages_to_allocate} pages`);
      
      this.initKGSL = await this.getKGSL()
      this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

      await this.allocate_inc(pages_to_allocate)

      this.newKGSL = await this.getKGSL()
      this.newKGSL = this.newKGSL.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)

      if (this.newKGSL.length !== pages_to_allocate) {
        console.warn(`[Allocator] wrong texture count[Expected ${this.pages}] != [Got ${this.newKGSL.length}]`)
        console.warn('[Allocator] retrying...');
        continue
      }
      this.pages = pages_to_allocate

      //assume they are in sunc, sorted 
      this.sortSelf()

      let cont_pages = this.SearchContPages_inc()
      if (cont_pages.length) {
        return Promise.resolve(cont_pages[0])
      }
      else {
        pages_to_allocate *= 2
      }
    }
  }

  async _init(safe) {
    if ( safe ) {
      console.log('[Allocator] Safe mode.')
      await this.allocateSafe()
      return Promise.resolve(true) 
    }
    console.log(`[Allocator] ++ Fetching ground truth values.`);
    this.initKGSL = await this.getKGSL()
    this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

    console.log(`[Allocator] ++ ${this.initKGSLIds.length} textures already existed. generating ${this.pages} page-textures. [${this.pages*KB4/MB} MB]`);
    
    await this.allocate()
    console.debug('[Allocator] ++ Allocation Done.')
    gl.finish()
    
    // filter out thos that already existed from before.
    this.newKGSL = await this.getKGSL()
    this.newKGSL = this.newKGSL.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)
    if (this.newKGSL.length !== this.pages) {
      console.warn(`[Allocator] wrong texture count[Expected ${this.pages}] != [Got ${this.newKGSL.length}]`)
      this.clean()
      return
    }

    console.log(`[Allocator] ++ ${this.newKGSL.length} unique new textures found.`);

    // modifies this.newKGSL
    this.sortSelf()
    
    for (let i = 0; i < this.pages; i++) {
      this.newKGSL[i].texture = this.textures[i]
      this.newKGSL[i].estValue = i % 256
    }

    // await this.check()
    
    return Promise.resolve(true)
  }

  async check() {
    let offsetIdx = 0
    let offsets = []
    let correct = 0
    let wrong = 0
    for (let i = 0; i < this.newKGSL.length; i += (64)) {
      let realValue = (await readPtr(this.newKGSL[i].v_addr)).val;
      let estValue = this.newKGSL[i].estValue
      if (realValue != estValue) {
        let offset = realValue - estValue
        wrong += 1
        if (offsets.indexOf(offset) == -1) { offsets.push(offset) }
      } else {
        correct += 1
      }
    }
    console.log(`++ Check Done. ${correct} correct | ${wrong} wrong | ${offsets}`);
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
        contPageChunks.push(tmp) 
        i += MIN_CONTIMUOUS_PAGES-1
      }
    }
    return contPageChunks
  }

  SearchContPages_inc() {
    console.log(`[Allocator] ++ Searching for ${MIN_CONTIMUOUS_PAGES} pages.`);
    let contPageChunks = []
    for (let i = 0; i < this.pages - MIN_CONTIMUOUS_PAGES; i++) {
      let found = 1
      let j;
      for (j = 0; j < MIN_CONTIMUOUS_PAGES; j++) {
        if (this.newKGSL[i].pfn + j !== this.newKGSL[i + j].pfn) {
          found = 0
          break
        }
      }
      this.newKGSL[i].alloc_order = j
      if (found) {
        let tmp = []
        for (let k = 0; k < j; k++) {
          tmp.push(this.newKGSL[i + k])
        }
        contPageChunks.push(tmp)
        i += MIN_CONTIMUOUS_PAGES - 1
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
    const STEP = 256
    for (let i = 0; i < this.pages; i+= 0) {
      this.initKGSL = await this.getKGSL()
      this.initKGSLIds = this.initKGSL.map(obj => obj.tex_id)

      let newTexs = [] 
      for (let i = 0; i < STEP; i++ ){
        const t = createTexture2DRGBA(createUint8Array(KB4, i%256), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
        newTexs.push(t)
      }

      await sleep(200)
      let newKGSLList = await this.getKGSL()
      newKGSLList = newKGSLList.filter(obj => this.initKGSLIds.indexOf(obj.tex_id) == -1)
      
      if ( newKGSLList.length !== STEP ) {
        console.error("[Allocator] ++ More than STEP page is created?...")
        console.log(newKGSLList.length, STEP)
        continue
      }
      else {
        newKGSLList.sort((a, b) => {
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
        console.log(`[Allocator] ++ [${i+STEP}/${this.pages}]new verified page created'`);
        let CHECK_PASSED = true
        for (let j = 0; j < STEP; j++) {
          let realValue = (await readPtr(newKGSLList[j].v_addr)).val;
          let estValue = j % 256
          if (realValue != estValue) {
            console.log('real', realValue,'est', estValue);
            
            console.log(`But a value is wrong...`);
            CHECK_PASSED = false
          }
          else if ( j > 10 ) {
            break
          }
        }
        if ( CHECK_PASSED ) {
          for (let j = 0; j < STEP; j++) {
            newKGSLList[j].texture = newTexs[j]
          }
          this.newKGSL = this.newKGSL.concat(newKGSLList)
          this.textures = this.textures.concat(newTexs)
          i+= STEP
        }
      }
    }
    return Promise.resolve(1)
  }

  async allocate() {
    for (let i = 0; i < this.pages; i++) {
      const t = createTexture2DRGBA(createUint8Array(KB4, i%256), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
      this.textures.push(t);
      continue
      if (i % this.SLEEP_STEP == 0 && i > 0) {
        console.debug(`[Allocator] [${i}/${this.pages}] pages created so far. Taking a short break.`);
        await sleep(this.SLEEP);
        allocatePages(this.JS_ARRAY_MB * PAGES_PER_MB)
      }
    }
    return Promise.resolve(1)
  }

  async allocate_inc(pages) {
    for (let i = 0; i < pages; i++) {
      const t = createTexture2DRGBA(createUint8Array(KB4, i % 256), PAGE_TEXTURE_W, PAGE_TEXTURE_H);
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