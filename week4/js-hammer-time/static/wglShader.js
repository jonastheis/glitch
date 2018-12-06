class Shader {
  constructor(vertexShaderId, fragmentShaderId) {
    const vertexShaderSource = document.getElementById(vertexShaderId).text;
    const fragmentShaderSource = document.getElementById(fragmentShaderId).text;

    const vertexShader = this.createShader(gl.VERTEX_SHADER, vertexShaderSource);
    const fragmentShader = this.createShader(gl.FRAGMENT_SHADER, fragmentShaderSource);

    this.id = this.createProgram(vertexShader, fragmentShader);
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
    console.log(`++ Binding [${name}] -- offset: ${offset}`);

    const location = gl.getUniformLocation(this.id, name);
    console.log(gl.TEXTURE0 + number, gl.TEXTURE0, gl.TEXTURE1);
    gl.activeTexture(gl.TEXTURE0 + number);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(location, number);
  }
}