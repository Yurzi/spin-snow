## Spin Snow

一个简单 OpenGL 课程的作业，绘制旋转的雪花。

实现了以下功能：
- [x] 1. 着色器的封装
- [ ] 2. 模型的封装
- [ ] 3. 使用assimp加载加载模型
	- [x] 3.1 实现对单顶点多纹理坐标的支持
- [ ] 4. 相机系统
- [ ] 5. 光照与阴影系统

## 如何构建
 
本项目使用 CMake 和 vcpkg 作为项目管理工具，使用 MSVC 工具链（可以通过修改 vcpkg 的 triplet 来使用其他工具链）

故而需要修改 `CMakeList.txt` 中 `CMAKE_TOOLCHAIN_FILE` 的设置后，按常规流程进行构建即可。

例：
```sh
$ cmake -B build -G Ninja
```
