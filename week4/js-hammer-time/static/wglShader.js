class Shader {
  constructor(vertexShaderId, fragmentShaderId) {
    const vertexShaderSource = document.getElementById(vertexShaderId).text;
    const fragmentShaderSource = document.getElementById(fragmentShaderId).text;

    this.vertexShader = this.createShader(gl.VERTEX_SHADER, vertexShaderSource);
    this.fragmentShader = this.createShader(gl.FRAGMENT_SHADER, fragmentShaderSource);

    this.id = this.createProgram(this.vertexShader, this.fragmentShader);
  }

  createShader(type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    const success = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (success) {
      return shader;
    }

    console.log(gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
  }

  createProgram(vertexShader, fragmentShader) {
    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    const success = gl.getProgramParameter(program, gl.LINK_STATUS);
    if (success) {
      return program;
    }

    console.log(gl.getProgramInfoLog(program));
    gl.deleteProgram(program);
  }

  use() {
    gl.useProgram(this.id);
  }

  bindTexture(texture, number, type, offset) {
    const name = `${type}Tex0${number}`;
    // console.log(`++ Binding [${name}] -- offset: ${offset}`);

    const location = gl.getUniformLocation(this.id, name);
    gl.activeTexture(gl.TEXTURE0 + number);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(location, number);
  }

  setInt(name, value) {
    gl.uniform1i(gl.getUniformLocation(this.id, name), value);
  }
}