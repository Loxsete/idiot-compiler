# IC — Idiot Compiler

A simple compiler written in C, built to understand how compilers work from scratch.  
Inspired by my earlier [QCC](https://github.com/Loxsete/qcc) project, but more ambitious.

---

## Build
```bash
gcc main.c -o compiler
```

## Usage
```bash
./compiler file.ic
./program
```

---

## Syntax

### Print
```
print("Hello, world!\n")
print(x)
```

### Variables & Math
```
x = 10
x = 20 + 5
x = 20 - 5
x = 20 * 5
x = 20 / 5
```

### If / Else
```
if x == 10 {
    print("yes\n")
} else {
    print("no\n")
}
```

### Functions
```
fn greet(name) {
    print(name)
}

fn main() {
    greet(42)
}
```

### Comments
```
// this is a comment
```
