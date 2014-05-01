# Kobashell
## Overview
Kobashell is a shell that will be submitted as an assignment of IS (Department of Information Science, School of Science).
It uses a parser that was provided by IS (in the directory `parser/`).

## Functions
Functions and their status.

| item | status |
| --- | --- |
|redirection| yes |
|pipe | yes |
| searching $PATH for executable | no |
| expanding wildcard | no | 
| reading from file | no |
| rewriting parser | no |
|using readline | yes |

## How to use it
```
cd build/
cmake ..
make
```
creates an executable file `build/bin/ish`. 

## Requirement
* gcc
* cmake
* readline (optional)


