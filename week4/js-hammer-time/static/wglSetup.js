// Global vars 
KB = 1024;
KB4 = KB * 4;
PAGE_TEXTURE_W = 32;
PAGE_TEXTURE_H = 32;

function initGL() {
  const canvas = document.getElementById('c');
  gl = canvas.getContext('webgl2');
  if (!gl) {
    console.log('-- No webgl2 for you!');
    throw new Error('-- WebGL2 not available!');
  }
  else {
    console.log('++ global gl object initialized');
  }
}