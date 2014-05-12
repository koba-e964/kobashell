# Kobashell
## Overview
Kobashell is a shell that will be submitted as an assignment of IS (Department of Information Science, School of Science).
It uses a parser that was provided by IS (in the directory `parser/`).

## Functions
Functions and their status.

| item | status |
| --- | --- |
| redirection| yes |
| pipe | yes |
| searching $PATH for executable | yes |
| expanding wildcard | no | 
| reading from file | no |
| rewriting parser | no |
| background/foreground | no |
| using readline | yes |

## How to use it
If **cmake** is installed on your machine,
```
cd build/
cmake ..
make
```
creates an executable file `bin/ish`. 
Otherwise, `make` creates an executable file `bin/ish` (the same position).

## Requirement
* gcc
* cmake (optional)
* readline (optional)

## TODO
If some processes connected with a pipe is run simultaneously, and when one of them was terminated, the other processes continue execution. It is not a correct behavior.
