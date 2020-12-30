# 🐾 Neko

<p align="center">
  <img src="Resources/neko.png?raw=true" alt="Neko"/>
</p>

[![License](https://img.shields.io/github/license/nyalloc/neko)](https://github.com/nyalloc/neko/blob/main/LICENSE)

## 🎮 A New Graphics API

Neko is an early-stage 3D graphics API wrapper. It is implemented as a dependency-free [STB-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) header-only C library. It will sit on top of Vulkan, D3D12 and Metal and define a higher-level WebGPU-style interface. The intended users are developers who want to prototype or implement small game engines. Neko will bring the most important capabilities of modern low-level graphics APIs, but restore the joy of working with high-level APIs. Neko is influenced by the development of WebGPU and will develop alongside it.

## 🔬 Why C?
* Easier integration with other languages
* Easier integration into other projects
* Adds only minimal size overhead to executables

## 🛠️ Current Work

The Vulkan backend is currently under development. Vulkan is the most portable backend to target, so it is a worthwhile starting point. Development work is currently focused on Windows. Once the Vulkan backend is in good shape, development will shift to introduce Linux support while introducing testing and continuous integration.

[![Twitter](https://img.shields.io/twitter/follow/nyalloc?label=follow)](https://twitter.com/intent/user?screen_name=nyalloc)
[![GitHub](https://img.shields.io/github/followers/nyalloc?label=follow&style=social)](https://github.com/nyalloc)
[![Nyalloc](https://img.shields.io/badge/nyalloc-blog-ff69b4?style=flat)](https://nyalloc.io)
