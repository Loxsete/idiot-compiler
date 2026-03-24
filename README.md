# IC — Idiot Compiler

A dumb little compiler I wrote in C to figure out how compilers actually work.
Inspired by my older [QCC](https://github.com/Loxsete/qcc) project, but I wanted to go further this time.

---

## Build
```bash
make
```

## Usage
```bash
./compiler file.ic           # compiles to ./a
./compiler file.ic -o out    # compiles to ./out
./compiler file.ic -v        # verbose, shows what's going on
./compiler file.ic --debug   # keeps the .asm and .o files around
./a
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
int y = x + 5
int z = x - 3
int w = x * 2
int d = x / 2
```

### Types

| Type   | Size   | What it is                     |
|--------|--------|--------------------------------|
| `int`  | 32-bit | your everyday integer          |
| `long` | 64-bit | big number, won't overflow fast |
| `bool` | 8-bit  | 0 or 1, that's it              |
| `char` | —      | string variable                |
```
int x = 10
long big = 1000000
bool flag = 1
char msg = "hello\n"
print(msg)
```

### If / Else

Works with `==`, `!=`, `<`, `>`. That's all you get for now.
```
if x == 10 {
    print("yep\n")
} else {
    print("nope\n")
}
```

### While
```
int i = 0
while i < 10 {
    print(i)
    i = i + 1
}
```

### For

Pretty much C-style. Step is `+` or `-` only.
```
for (i = 0; i < 10; i = i + 1) {
    print("hey\n")
}
```

Heads up - declare `i` as `int` before the loop if you need it outside.

### Functions

Up to 6 args. Don't try recursion, it'll blow up (all vars are global).
```
fn add(a, b) {
    int result = a + b
    print(result)
}

fn main() {
    add(10, 20)
}
```

### Returns

Simple return. Just:
```
fn add(a, b) {
    int result = a + b
    return result
}

fn main() {
    int x = add(3, 4)
    print(x)
}

### Pointers
```
int x = 42
ptr* p = &x
print(x)     // 42
*p = 99
print(x)     // 99
```

### Comments
```
// yeah this does nothing
```

---

## Flags

| Flag        | What it does                          |
|-------------|---------------------------------------|
| `-o <file>` | name the output binary (default: `a`) |
| `-v`        | print each step as it happens         |
| `--debug`   | don't clean up .asm and .o            |

---

