# IC — Idiot Compiler

A simple compiler written in C, built to understand how compilers work from scratch.  
Inspired by my earlier [QCC](https://github.com/Loxsete/qcc) project, but more ambitious.

---

## Build
```bash
make
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
int x = 10
int x = 20 + 5
int = 20 - 5
int = 20 * 5
int = 20 / 5
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
