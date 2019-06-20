1. In this directory the result of test suits are listed.

2. The test suite is stated by command:
```
    ./mysql-test-run.pl --do-suite=innodb --force
```

3. The very first test has been done for unchanged version of **Oracle-MySQL version 8.0.16**

4. Please, note that some test from this test suits fails (!!!) even for Oracle-MySQL version 8.0.16

5. After changes made during modularization process of `InnoDB` the result of the tests must be the same.

6. Every files name containing the result starts form four digit number followed by commit id obtained by command:
```
    git rev-parse HEAD
```

7. The very first result is called `0000.Oracle-MySQL-8.0.16.txt`
