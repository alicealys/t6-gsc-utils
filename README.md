# t6-gsc-utils
Adds some useful stuff to gsc, based on Matrix's [t6-gsc-helper](https://github.com/skiff/t6-gsc-helper)
# Features
* Adds chat notifies (eg. `level waittill("say", player, message)`)
* Adds a bunch of IO functions (`fopen`, `fread`, `fputs`...)
* Some other useful functions like `printf`, `cmdexecute`, `say` ...

IO functions work basically exactly the same as they do in C/C++
```c
init() {
  level.basepath = getDvar("fs_basepath") + "/" + getDvar("fs_basegame") + "/";
  path = level.basepath + "whatever.txt";
  
  file = fopen(path, "a+");
  
  text = fread(file); // fread reads the entire file, you can also use fgets(file, n) to read the first n chars
  
  fclose(file);
  /*
    printf also works similarly to C/C++ but only has %s
    eg. printf("%s, %s", 1, "2") -> "1, 2"
        printf(va("%s, %s", 1, "2")) -> "1, 2"
  */
  
  printf(text);
  
  // you can also use fgetc and feof if you desire
  file = fopen(path, "a+");
  
  eof = false;
  buf = "";
  
  while (!eof) {
    c = fgetc(file);
    eof = feof(file);
    
    if (!eof) {
      buf += c;
    }
  }
  
  fclose(file);
  printf(buf);
  
  // write to a file
  file = fopen(path, "a+");
  
  fputs("hello world\n", file);
  fprintf("hello world\n", file);
  
  fclose(file);
}
```

Chat notify example

```c
init () {
  level thread onPlayerMessage();
}

onPlayerMessage() {
  for (;;) {
    level waittill("say", player, message);
    
    player tell("You said: ^5" + message);
  }
}

```
