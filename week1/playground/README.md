# Playground 

This is where we will try things out. 

### Libraries.

- Download a copy of [glfw](http://www.glfw.org/docs/latest/index.html) in the libraries folder. I think compiling is not needed and the top-level makefile will build it automatically.
- Download a copy of [glad](https://glad.dav1d.de/) in the libraries folder. Download options: 
  - lang: C/CPP
  - gl version: 3.3+
  - specification: openGl
  - profile: core
  - I added all the plugins as well
  
Finally, libraries folder should be like this: 

```bash
- libraries
    - glad 
        - include
        - src
    - glfw
        - include
        - src
        - ... 
```

# run 

- run `cmake . ` and then `make` in the source folder
- or open in clion


# Useful OpenGL resources

- https://paroj.github.io/gltut/Texturing/Tutorial%2014.html
- https://learnopengl.com/Getting-started/Textures