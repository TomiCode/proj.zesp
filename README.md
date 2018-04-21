# proj.zesp
Just a silly university _group_ project.



## Getting started
You only need to have `cmake` and `ncurses` installed. To build the client, first you have to create a build directory and then run `cmake` to generate makefiles.

#### Client

```shell
mkdir client/build
cd client/build
cmake ..
make
```

After the *successful* compilation, there should be a executable in the build directory named `client`.

#### Server

```shell
mkdir server/build; cd server/build
cmake ..
make
```

A successful compilation should create a server executable named `server` in the current build directory.



## Why?

Yes.