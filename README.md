# GLitch - remote rowhammer on ARM via JavaScript & WebGL

[GLitch](https://www.vusec.net/wp-content/uploads/2018/05/glitch.pdf) is a remote rowhammer exploit on ARM Android devices. In order to do so it makes use of the [WebGL](https://www.khronos.org/webgl/) interface in JavaScript to use the GPU to do the actual hammering. It does not rely on any software bug to obtain remote code execution. As a response to the publication of GLitch [browsers have disabled specific timer functionality](https://developer.mozilla.org/en-US/docs/Web/API/EXT_disjoint_timer_query#Browser_compatibility).

The exploit in this repository was implemented in the course *Hardware Security* lectured by Kaveh Razavi at Vrije Universiteit Amsterdam in Period 2 of the academic year 2018/2019. The focus of the course was to obtain in-depth knowledge of state of the art hardware exploits of various sorts (exploitation, leaking data, defenses).

As a result of university course work the exploit in this repository is not completely end-to-end as it is described in the paper. Because of time constraints we skipped the detection of contiguous memory using a timing side-channel and obtained the information directly from `/proc/{pid}/pagemap`. Furthermore, the repository is structured in several weeks of consecutive steps to first get familiar with [OpenGL](https://www.khronos.org/opengl/) and the GPU with [Android's NDK](https://developer.android.com/ndk/), implement cache eviction and hammering pattern with [GLSL](https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language) and finally translate everything to JavaScript and [WebGL](https://www.khronos.org/webgl/). 

## Week 1: Building tools

## Week 2: Understanding the GPU architecture

## Week 3: Native bit flips

## Week 4: Porting to JavaScript


## License
This project is licensed under the [Apache Software License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).

See [`LICENSE`](LICENSE) for more information.

    Copyright 2018 Kian Peymani, Jonas Theis
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
       http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
