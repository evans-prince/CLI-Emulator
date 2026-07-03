## 🖥️ SimpleRISC CLI Interpreter

A lightweight and minimal **SimpleRISC v21** interpreter written in pure C 🧡. This command-line tool reads SimpleRISC assembly instructions and interprets them directly — no intermediate machine code, just pure CPU mimicry!

---

### ✨ Features

* 🧠 **Real-time Interpretation**
  Parses and executes SimpleRISC `.s` files directly.

* ⚡ **Minimalist & Fast**
  Single-file architecture for quick builds and easy tweaks.

* 📄 **Handles Registers & Memory**
  Simulates general-purpose registers, instruction behavior, and memory ops.

* 🐞 **Debugging Output**
  Optional mode to print register and memory states after each instruction.

---

### 📁 File Structure

```bash
.
├── main.c      # All logic packed in one mighty file
├── Makefile    # Optional build support (if you’ve added it)
```

---

### 🛠️ Build Instructions

```bash
gcc -o simpleRISC main.c -lm
```

Or if you’ve added a Makefile (optional, cutie):

```bash
make
```

---

### ▶️ How to Run

```bash
./simpleRISC <program.asm> 
```

**Arguments:**

* `<program.asm>` : Your SimpleRISC assembly file.

---

### 🧪 Example

```bash
./simpleRISC test/fib.s
```


---

### LIVE DEMO

https://evans-prince.github.io/CLI-Emulator/

---
### 📌 Notes

* Supports signed and unsigned immediates.
* Covers major 3-address instruction formats.
* Halfword and upper immediate support included 

---

### 💡 To-Do

* [ ] Add support for branching and pipelining
* [ ] Modularize interpreter into separate files
* [ ] Add UI or web-based interface (👀 future upgrade?)

---

### 🧑‍💻 Author

By **Prince (IIT Delhi)** 🧡
Drop a ⭐ if you like it or wanna collab.

---

### 📜 License

MIT License — free to use, modify, and share!


