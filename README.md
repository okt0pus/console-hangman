# console-hangman

This project aims to create a simple Hangman Console Game for Debian/ Ubuntu Linux users.

### Build Process

In order to build the little game you should have already installed the ` build-essential ` package and the NCURSES library ` libncurses5 `.

In order to get those you can simply run:

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev build-essential
```

After you make sure you have all the needed packages, then you can build the little game:

```bash
make build
```

### Run the game

In order to run the game you have to pass a text file as argument, where all you words are store. In the repo you have one example of such file in ` test/input.in `. Run the command as follows:

```bash
./hangman test/input.in
```

### Game Pictures

![alt text](https://drive.google.com/file/d/1-bfnjuuJxX_hx95TmXCz-ENzAT2ZeUXA/view?usp=sharing)

![alt text](https://drive.google.com/file/d/1PFUShtQYURejYSnEISN2wqdiRAmaBhZB/view?usp=sharing)

![alt text](https://drive.google.com/file/d/1gWT8U1gu6TLaeU1KFjcrUSbu5YZQ9qaZ/view?usp=sharing)

![alt text](https://drive.google.com/file/d/1nqvWACB93vteAPnRqo-DLAs_fQmrY19z/view?usp=sharing)

![alt text](https://drive.google.com/file/d/1tOyOCbb_uSsu6cX8jfSurs3MJV8Wgj4A/view?usp=sharing)


### FYI

This project was created as an educational project in order to get familiar with the C programming language, back in 2017. The code is not very well written and can be improved a lot.

If you are new to programming and want a small project to start learning this one can be a good start.

Hope that the comments from the code helps new programmers to get started.