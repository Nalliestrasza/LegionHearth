<<<<<<< HEAD
<<<<<<< HEAD
#Objscale [![Build Status](https://travis-ci.org/Rochet2/TrinityCore.svg?branch=objscale_6.x)](https://travis-ci.org/Rochet2/TrinityCore)

####About
=======
# Objscale [![Build Status](https://travis-ci.org/Rochet2/TrinityCore.svg?branch=objscale_7.x)](https://travis-ci.org/Rochet2/TrinityCore)
=======
# Objscale [![Build Status](https://travis-ci.org/Rochet2/TrinityCore.svg?branch=objscale_master)](https://travis-ci.org/Rochet2/TrinityCore)
>>>>>>> beae6bdea63f11ded139d687dafde1e51716f370

#### About
>>>>>>> ec6b3c4abe8bfc8cf15521049525179ce00006b6
Objscale allows scaling individual spawns of creatures and gameobjects.
You can do this change ingame with commands or in the `creature` and `gameobject` database tables.
Source: http://rochet2.github.io/Objscale.html

<<<<<<< HEAD
####Installation

Available as:
- Direct merge: https://github.com/Rochet2/TrinityCore/tree/objscale_6.x
- Diff: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:6.x...objscale_6.x.diff
- Diff in github view: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:6.x...objscale_6.x
=======
#### Installation

Available as:
<<<<<<< HEAD
- Direct merge: https://github.com/Rochet2/TrinityCore/tree/objscale_7.x
- Diff: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:master...objscale_7.x.diff
- Diff in github view: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:master...objscale_7.x
>>>>>>> ec6b3c4abe8bfc8cf15521049525179ce00006b6
=======
- Direct merge: https://github.com/Rochet2/TrinityCore/tree/objscale_master
- Diff: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:master...objscale_master.diff
- Diff in github view: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:master...objscale_master
>>>>>>> beae6bdea63f11ded139d687dafde1e51716f370

Using direct merge:
- open git bash to source location
- do `git remote add rochet2 https://github.com/Rochet2/TrinityCore.git`
<<<<<<< HEAD
<<<<<<< HEAD
- do `git pull rochet2 objscale_6.x`
=======
- do `git pull rochet2 objscale_7.x`
>>>>>>> ec6b3c4abe8bfc8cf15521049525179ce00006b6
=======
- do `git pull rochet2 objscale_master`
>>>>>>> beae6bdea63f11ded139d687dafde1e51716f370
- use cmake and compile

Using diff:
- DO NOT COPY THE DIFF DIRECTLY! It causes apply to fail.
- download the diff by __right clicking__ the link and select __Save link as__
- place the downloaded `objscale.diff` to the source root folder
- open git bash to source location
- do `git apply objscale.diff`
- use cmake and compile

After compiling:
<<<<<<< HEAD
- Navigate to `\src\server\scripts\Custom\objscale\sql\`
- Run `world.sql` to your world database
- Run `auth.sql` to your auth database

####Usage
=======
- TrinityCore auto updater should run needed SQLs automatically.
- If you do not use the auto updater then run files named `*_objscale.sql` from `\sql\custom` to your databases.

#### Usage
>>>>>>> ec6b3c4abe8bfc8cf15521049525179ce00006b6
You can set the sizes in the `creature` and `gameobject` database tables in the world database by changing the `size` column.
In game you can change a spawned object's size by using `.gobject set scale #guid #scale` and spawned NPC sizes by selecting an NPC and using `.npc set scale #scale`
Using -1 has scale makes the gameobject and npc use the default scale from template.

<<<<<<< HEAD
Known issue: You can not resize some gameobject types. This is likely a client limitation.

####Bugs and Contact
=======
Known issues:
- You can not resize some gameobject types. This is likely a client limitation.
- You may need to relog to see the size changes for some wow patches or versions of the core.

#### Bugs and Contact
>>>>>>> ec6b3c4abe8bfc8cf15521049525179ce00006b6
Report issues and similar to https://rochet2.github.io/
