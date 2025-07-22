# My Language
*A custom programming language designed for education process and as for project for my portfolio.*  

---

## Introduction  
This project is the last part of the course of programming on the C, which is taught by Ilya Dedinsky in the 1st semester at MIPT. It brings together everything that was studied during the semester. For me, the project became an opportunity to learn something new and gain invaluable experience.

**Key Goals**:  
- ✅ Goal 1: combine all the knowledge and skills acquired during the course.
- ✅ Goal 2: research common principles of programming languages.
- ✅ Goal 3: learn more about how real hardware works.

---

## Technical Specifications  
- **Compiler**: `g++` (GCC, version 14.2.0)  
- **IDE**: Microsoft VSCode
- **OS**: Kali Linux (version 2025.2)  
- **CPU**: Intel Core i7 13700H (2.4 GHz)  

**Build Requirements**:  
- C++17 or later  
- CMake
  
*(Example code for building):*  
```bash 
$ make clean  
$ make
$ make run
```
---

## Development Tools  
The language was built using:  
- **Lexer/Parser**: *custom*
- **IR/Codegen**: *custom*
- **Debugging**: "GDB", "Valgrind"

---

## Language Features  
### Supported Syntax  
- Variables:
  ```python
  x = 5;
  y = func(x);
  z = input();
  ``` 
- Loop:
  ```python
  while (x < 10)
  {
    x = (x + 1);
  }
  ```
- Functions:
  ```python
  def func(a, b)
  {
    a = (a + b);
    return a;
  }
  y = input();
  x = func(y, 2);
  print(x);
  ```
- Condition:
  ```python
  if (x < 25)
  {
    func(x);
  }
  else
  {
    print(x);
  }
  ```

### Example Code  
This is code of programm, which get factorial of number:
```python
def fact(n)
{
    i = 1;
    pr = 1;
    while (i <= n)
    {
        pr = (pr * i);
        i = (i + 1);
    }
    return pr;
}
x = input();
ans = fact(x);
print(ans);
$
```

### Important Features
It is important to write symbol `$` after programm. In the process of creating the language, I used my own **stack**, **assembler** and **processor**, as well as **recursive descent**.

---

## Conclusion  
This project has given me a lot of skills and knowledge. I am very grateful to [Ilya Rudolfovich Dedinsky](https://github.com/ded32?tab=overview&from=2025-07-01&to=2025-07-22) and my mentor [Kolya Kasparov](https://github.com/nniikon) for sharing their invaluable experience.

**Get Started**:  
```bash  
$ git clone https://github.com/ZEVS1206/MyLanguage.git 
$ cd MyLanguage
$ make [file = your own file name.rt2025]
$ make run
```
You need to create your programms in files as *test_file.rt2025* and in the folder MyLanguage.

**License**: "MIT" 

