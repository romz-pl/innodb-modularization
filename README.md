1. The goal of this project is to enhance the structure of **InnoDb storage engine** taken from **Oracle MySQL 8.0.16**

2. The base source code used in this project is [mysql-cluster-8.0.16](https://github.com/mysql/mysql-server/releases/tag/mysql-cluster-8.0.16)

3. By enhancement it is meant:
   - Remove redundant inclusions of header files.
   - Remove circular dependencies in headers files.
   - Provide clear, layered structure of code.
   - Divide the engine into separate libraries (modules).

4. The current version of InnoDb storage engine is compiled as one library, mostly because of circular dependencies in source files.

5. Each enhancement (source code modification) is defined as [Issue](https://github.com/romz-pl/innodb-modularization/issues)

