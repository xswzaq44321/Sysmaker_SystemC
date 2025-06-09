# SystemC Project Description

This is a simple SystemC project structure managed using GNU Make.

---

## 📁 Project Structure Guidelines

- All `.cpp` source files should be placed in the `source/` directory (subdirectories are allowed)
- All `.h` header files should be placed in the `include/` directory (subdirectories are allowed)
- Compiled `.o` object files and intermediate files will be stored in the `build/` directory
- The final executable will be placed in the project root directory

---

## 🔧 Build Instructions

Please ensure that SystemC is installed and the `$SYSTEMC_HOME` environment variable is correctly set.  
Then run the following commands:

```
make clean  # Remove intermediate files and the executable
make        # Compile
```

The default name of the executable is `main`.
