# t6-gsc-utils
T6 version of [iw5-gsc-utils](https://github.com/fedddddd/iw5-gsc-utils).  
If you wish for any feature to be added please create an [issue](https://github.com/fedddddd/t6-gsc-utils/issues/new).

# Installation
* Download the latest version from the [releases](https://github.com/fedddddd/t6-gsc-utils/releases)
* Copy it to `Plutonium/storage/t6/plugins`

# Features

## Bitwise operators
The T6 GSC compiler is missing support for bitwise operators (or, xor, and, ...), instead you can use the corresponding functions:
```c
init()
{
    flags = 0;
    flags = or(flags, 1);       // flags |= 1;
    flags = and(flags, not(1)); // flags &= ~1;
    flags = xor(flags, 1);      // flags ^= 1;
    
    if (and(flags, 1))          // flags & 1
    {
        // [...]
    }
}
```

## Misc
* `getFunction(filename, name)`: Gets a function from a GSC script.

  ```c
  init()
  {
      function = getFunction("maps/mp/gametypes/_callbacksetup", "callbackVoid");
      [[ function ]]();
  }
  ```
* `getFunctionName(function)`: Returns the function's name.
  
  ```c
  init()
  {
      function = getFunction("maps/mp/gametypes/_callbacksetup", "callbackVoid");
      print(getFunctionName(function)); // "maps/mp/gametypes/_callbacksetup::callbackVoid"
  }
  ```
* `arrayRemoveKey(array, key)`: Removes an array element by its (string) key.

  ```c
  init()
  {
      array = [];
      array["foo"] = 1;
      arrayRemoveKey(array, "foo");
  }
  ```
* `structGet(struct, key)`: Equivalent to `struct.field_name`.

  ```c
  init()
  {
      object = spawnStruct();
      object.foo = 1;
      print(structGet(object, "foo"));
  }
  ```
* `structSet(struct, key, value)`: Equivalent to `struct.field_name = value`.

  ```c
  init()
  {
      object = spawnStruct();
      structSet(object, "foo", 1);
      print(structGet(object, "foo"));
  }
  ```
* `structRemove(struct, key)`: Removes an element from a struct.

  ```c
  init()
  {
      object = spawnStruct();
      object.foo = 1;
      structRemove(object, "foo");
  }
  ```
* `getStructKeys(struct)`: Returns an array containing all the struct's keys.

  ```c
  init()
  {
      object = spawnStruct();
      object.foo = 1;
      object.bar = 2;
      
      keys = getStructKeys(object);
      
      for (i = 0; i < keys.size; i++)
      {
          print(keys[i], structGet(object, keys[i]));
      }
  }
  ```
* `isFunctionPtr(value)`: Returns true if value is a function.

  ```c
  init()
  {
      assert(isFunctionPtr(::func) == true);
      assert(isFunctionPtr("func") == false);
  }
  
  func()
  {
      print("Hello world");
  }
  ```
* `isEntity(value)`: Returns true if value is an entity (ex. player, script_model, hudelem)

  ```c
  init()
  {
      model = spawn("script_model", (0, 0, 0));
      array = [];
      
      assert(isEntity(model) == true);
      assert(isEntity(array) == false);
  }
  ```
* `isStruct(value)`: Return true if value is a struct.

  ```c
  init()
  {
      object = spawnStruct();
      array = [];
      
      assert(isStruct(object) == true);
      assert(isStruct(array) == false);
  }
  ```
* `typeof(value)`: Returns the name of a value's type.

  ```c
  init()
  {
      foo = 1;
      bar = "hello world";
      baz = [];
      
      assert(typeof(foo) == "int");
      assert(typeof(bar) == "string");
      assert(typeof(baz) == "array");
  }
  ```
* `entity set(field, value)`: Sets an entity's field.

  ```c
  init()
  {
      ent = spawn("script_origin", (0, 0, 0));
      ent set("origin", (100, 100, 100));
  }
  ```
* `entity get(field)`: Gets an entity's field.

  ```c
  init()
  {
      ent = spawn("script_origin", (0, 0, 0));
      print(ent get("origin"));
  }
  ```

## Entity fields
* `flags`: Get/Set a player's entity flags.
  
  ```c
  onPlayerSpawned()
  {
      while (true)
      {
          self waittill("spawned_player");
          self.flags = xor(self.flags, 1); // FL_GODMODE
      }
  }
  ```
  
  Enum of all flags:
  
  ```c
  {
      FL_GODMODE = 0x1,
      FL_DEMI_GODMODE = 0x2,
      FL_NOTARGET = 0x4,
      FL_NO_KNOCKBACK = 0x8,
      FL_DROPPED_ITEM = 0x10,
      FL_NO_BOTS = 0x20,
      FL_NO_HUMANS = 0x40,
      FL_TOGGLE = 0x80,
      FL_SOFTACTIVATE = 0x100,
      FL_LOW_PRIORITY_USEABLE = 0x200,
      FL_NO_TACTICAL_INSERTION = 0x400,
      FL_DYNAMICPATH = 0x800,
      FL_SUPPORTS_LINKTO = 0x1000,
      FL_NO_AUTO_ANIM_UPDATE = 0x2000,
      FL_GRENADE_TOUCH_DAMAGE = 0x4000,
      FL_GRENADE_MARTYRDOM = 0x8000,
      FL_MISSILE_DESTABILIZED = 0x10000,
      FL_STABLE_MISSILES = 0x20000,
      FL_REPEAT_ANIM_UPDATE = 0x40000,
      FL_VEHICLE_TARGET = 0x80000,
      FL_GROUND_ENT = 0x100000,
      FL_CURSOR_HINT = 0x200000,
      FL_USE_TURRET = 0x400000,
      FL_MISSILE_ATTRACTOR = 0x800000,
      FL_TARGET = 0x1000000,
      FL_WEAPON_BEING_GRABBED = 0x2000000,
      FL_OBSTACLE = 0x4000000,
      FL_DODGE_LEFT = 0x8000000,
      FL_DODGE_RIGHT = 0x10000000,
      FL_BADPLACE_VOLUME = 0x20000000,
      FL_AUTO_BLOCKPATHS = 0x40000000,
      FL_MOVER_SLIDE = 0x80000000,
  };
  ```
* `clientflags`: Get/Set a player's client flags.
  
  ```c
  onPlayerSpawned()
  {
      while (true)
      {
          self waittill("spawned_player");
          self.clientflags = xor(self.clientflags, 1); // NOCLIP
      }
  }
  ```
  
## Command
* `executeCommand(command)`: Executes a console command.

  ```c
  fast_restart()
  {
      executeCommand("fast_restart");
  }
  ```
* `addCommand(name, callback)`: Adds a console command.

  ```c
  init()
  {
      addCommand("test_cmd", ::test_cmd);
  }
  
  test_cmd(args)
  {
      assert(args[0] == "test_cmd");
      print("Hello world", args.size);
  }
  ```
  
## Chat
* `entity setname(name)`: Sets a player's name.
  
  ```c
  onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player setName("hello world");
      }
  }
  ```
* `entity rename(name)`: Same as `setname`.
* `entity resetName()`: Resets a player's name to the original.
  
  ```c
    onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player setName("hello world");
          player resetName();
      }
  }
  ```
* `entity setClantag(clantag)`: Sets a player's clantag.

  ```c
  onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player setClantag("hello world");
      }
  }
  ```
* `entity resetClantag()`: Resets a player's clantag to the original.

  ```c
  onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player setClantag("hello world");
          player resetClantag();
      }
  }
  ```
* `say(message)`: Sends a chat message to all players.

  ```c
  onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          say(player.name + " connected!");
      }
  }
  ```
* `entity tell(message)`: Sends a chat message to a player.

  ```c
  onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player tell("Welcome back " + player.name + "!");
      }
  }
  ```
* `sendServerCommand(clientnum, cmd)`: Executes SV_GameSendServerCommand.

  ```c
    onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          sendServerCommand(-1, "j \"Hello world\""); // -1 -> all clients
      }
  }
  ```
  
* `entity sendServerCommand(cmd)`: Executes SV_GameSendServerCommand on a specified client.

  ```c
    onPlayerConnected()
  {
      while (true)
      {
          level waittill("connected", player);
          player sendServerCommand("j \"Hello world\"");
      }
  }
  ```
* `onPlayerSay`: Adds a player say callback.

  ```c
  init()
  {
      onPlayerSay(::callbackPlayerSay);
  }
  
  callbackPlayerSay(text, mode)
  {
      // mode == 0 -> all
      // mode == 1 -> team
      // self -> player that sent the message
      
      self tell("You said " + text);
      
      // returning `false` will hide the message, anything else will not
      return false;
  }
  ```
  
# IO
The basepath for all IO functions is `Plutonium/storage/t6`

* `fopen(path, mode)`: Opens a file of given name with given mode, returns a file stream.
* `fwrite(stream, text)`: Writes a string to a stream.
* `fread(stream)`: Reads entire file.
* `fclose(stream)`: Closes a file stream.
* `fremove(path)`: Deletes a file.

  ```c
  init()
  {
      file = fopen("test.txt", "w");
      fwrite(file, "test");
      fclose(file);

      file = fopen("test.txt", "r");
      print(fread(file));
      fclose(file);
  }
  ```
 * `fileExists(path)`: Returns true if the file exists.
 * `writeFile(path, data[, append])`: Creates a file if it doesn't exist and writes/appends text to it.
 * `readFile(path)`: Reads a file.
 * `fileSize(path)`: Returns file size in bytes.
 * `createDirectory(path)`: Creates a directory.
 * `directoryExists(path)`: Returns true if the directory exists.
 * `directoryIsEmpty(path)`: Returns true if the directory is empty.
 * `listFiles(path)`: Returns the list of files in the directory as an array.
 * `copyFolder(source, target)`: Copies a folder.
 * `jsonPrint(...)`: Prints values as json.
 * `httpGet(url)`: Creates a GET HTTP request-
    
    ```c
    init()
    {
        req = httpGet("https://example.com");
        req waittill("done", result);
        print(result);
    }
    ```
# JSON

* `jsonSerialize(variable[, indent])`: Converts GSC variables (such as arrays) into JSON:

  ```c
  init()
  {
      array = [];
      array[0] = 1;
      array[1] = 2;
      json = jsonSerialize(array, 4);
      
      print(json);
      
      /*
      [
          2,
          1
      ]
      */
  }
  ```
  
  This function can also be useful to reveal contents of existing arrays such as `game`:
  ```c
  init()
  {
      print(jsonSerialize(game["allies_model"], 4));
      
      /*
      {
          "ASSAULT": "[function]",
          "GHILLIE": "[function]",
          "JUGGERNAUT": "[function]",
          "LMG": "[function]",
          "RIOT": "[function]",
          "SHOTGUN": "[function]",
          "SMG": "[function]",
          "SNIPER": "[function]"
      }
      */
      
      print(jsonSerialize(game["music"], 4));
      
      /*
      {
          "defeat_allies": "UK_defeat_music",
          "defeat_axis": "IC_defeat_music",
           "losing_allies": "UK_losing_music",
           "losing_axis": "IC_losing_music",
           "losing_time": "mp_time_running_out_losing",
           "nuke_music": "nuke_music",
           "spawn_allies": "UK_spawn_music",
           "spawn_axis": "IC_spawn_music",
           "suspense": [
               "mp_suspense_06",
               "mp_suspense_05",
               "mp_suspense_04",
               "mp_suspense_03",
               "mp_suspense_02",
               "mp_suspense_01"
           ],
           "victory_allies": "UK_victory_music",
           "victory_axis": "IC_victory_music",
           "winning_allies": "UK_winning_music",
           "winning_axis": "IC_winning_music"
       }
      */
  }
  ```
* `jsonParse(json)`: Converts JSON into a GSC variable:

  ```c
  init()
  {
      array = jsonParse("[1,2,3,4]");
      print(array[0] + " " + array[1] + " " + array[2] + " " + array[3]);
      
      /*
      1 2 3 4
      */
  }
  ```
* `createMap(...)`: Creates a string-indexed array:
  
  ```c
  init()
  {
      array = map("first", 1, "second", 2);
      print(jsonSerialize(array, 4));
      /*
      {
          "first": 1,
          "second": 2
      }
      */
  }
  ```
* `jsonDump(file, value[, indent])`: Dumps a json value to a file.

  ```c
  init()
  {
      jsonDump("level.txt", level, 4);
  }
  ```

# Debugging
* `crash()`: Crashes the server.
* `breakpoint(message)`: Pauses the server's execution and shows a message box with the passed message and call stack.
* `assert(condition)`: Throws an error if condition is false.  

None of these functions will do anything unless `developer_script` is set to `1`, so make sure to set it before attempting to use them.  

Additionally, running the server with the `--gsc-debug` flag will set a custom crash handler to the server which will dump the GSC VM stack trace (along with local variables) to a text file (called `gsc_vm_dump.txt`) along with the regular minidump. These dumps will be saved to `Plutonium/storage/t6/minidumps`.

# Threads
* `killThread(function[, owner])`: Kills a running/wait/waittill thread based on the function it's executing and optionally even the thread owner object/entity.
 
  ```c
  /*
      This will kill the execution of "test_thread" after 5 seconds.
  */
  
  init()
  {
      level thread test_thread();
      level thread test_kill_thread();
  }

  test_kill_thread()
  {
      wait 5;
      killThread(::test_thread, level);
  }

  test_thread()
  {
      while (true)
      {
          wait 1;
          print("Hello world");
      }
  }
  ```
* `killAllThreads(function[, owner])`: Same as `killThread` but kills all matching threads.

# Bots

Custom bot names and clantags can be set by creating a file in `Plutonium/storage/t6/bots/bots.json` (create the folders if they don't exist) with the following json:

```json
{
    "names": {
        "name": "clantag",
        "name2": "clantag2"
    }
}
```

As shown by this example the pairs of name and clantag must be stored in an object called `names`.  
When a bot connects it will assign the pair of name and clantag based on the order they are in the object starting from the top.
