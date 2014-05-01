# Kobashell
## Overview
Kobashell is a shell that will be submitted as an assignment of IS (Department of Information Science, School of Science).
It uses a parser that was provided by IS (in the directory `/parser/`).

## Functions
Functions and their status.

| item | status |
| --- | --- |
|redirection| probably ok|
|pipe | imcomplete |
| searching $PATH for executable | no|
| expanding wildcard | no | 
| reading from file | no |
| rewriting parser | no |
|using readline | yes |

## How to use it
```
make
```
or
```
make all
```
creates an executable file `ish`. `ish` does not use `readline` library, so we recommend using `rlwrap`. 

## Requirement
* gcc
* rlwrap (for convenience)

