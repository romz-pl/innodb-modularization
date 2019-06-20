1. The goal of this project is to enhance the structure of InnoDb storage engine taken from Oracle MySQL 8.0.16

2. By anhancement it is meant:
   - remove reduntand inclusions of header files
   - remove circular dependencies of headers
   - provide clear layer structure of code
   - divide the engin in separate libraries (modules)

3. The current version of InnoDb storage engine is compiled as one library, mostly beacuse of circular dependencies in source files.

4. Each anhancement (source code modification) is defined as Issue

