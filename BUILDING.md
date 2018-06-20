# Building libdev

## Supported compilers
* Visual Studio 2017 with any Windows 10 SDK

## Supported platforms
* x86 (platform x86)
* x86_64 (platform x64)

## Build libdev
From a Visual Studio shell:
```console
msbuild LibDev.sln /p:Configuration=Debug /p:Platform=x86
msbuild LibDev.sln /p:Configuration=Debug /p:Platform=x64
msbuild LibDev.sln /p:Configuration=Release /p:Platform=x86
msbuild LibDev.sln /p:Configuration=Release /p:Platform=x64
```

or directly from Visual Studio.
