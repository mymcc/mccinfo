# mccinfo

![image](https://github.com/mymcc/mccinfo/assets/67705843/7ef19bad-d3e0-47a0-89d3-33755b540d50)

mccinfo is a C++20 helper library to accomplish providing information covering installation, versioning, and both static and runtime information regarding Halo: The Master Chief Collection&trade;

## Install and Build
To do so you can clone the repository and its submodule:
```bash
 git clone --recursive https://github.com/Stehfyn/mccinfo
```
Or do the following if you cloned the repo non-recursively already:
```bash
 cd mccinfo
 git submodule update --init --recursive
```
Then generate and build the `Visual Studio` .sln:
```bash
 .\scripts\build_solution.bat && msbuild /m /p:Configuration=Release .
```

Documentation can be generated with:
```bash
 .\scripts\generate_docs.bat
```

