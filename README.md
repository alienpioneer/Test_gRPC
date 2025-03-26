# GRPC test on Windows

Simple project that tests google gRPC on Windows in both synchronous and asynchronous modes.
Build and install gRPC first then simply build the client and the server. Tested with MSVC for Visual Studio Community 2019.

## Build GRPC notes
Google grpc can be build by following the instruction from the repository. In my case I build it on Windows with Cmake using the Visual Studio 16 2019 generator with a fair set of problems. If build fails because of stdalign.h, one solution is to copy stdalign.h here : ..\build_grpc_directory\grpc\third_party\boringssl-with-bazel\src\crypto)

## Build project
In Visual Studio 2019 a Cmake configure must launch automatically. If not, right click on CmakeLists.txt and choose "Configure Test_gRPC" or just do a save of the CmakeLists.txt. The run a "Build All" followed by "Install Test_gRPC" from the Build menu. All the stubs from the .proto file are generated automatically in the \proto project folder and must be cleaned up manually if the .proto file is changed.
