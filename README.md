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

![](https://drive.google.com/uc?export=view&id=1kn_tXQekm1aGAUYhuADN3UZHPHeh4MBQ)

![](https://drive.google.com/uc?export=view&id=1oI9NenLUeu7WHwaPbFXytBzoA2qsT1Rt)

![](https://drive.google.com/uc?export=view&id=1YSJKYmmttCpz7fYkgR4PP1uWKPnmTc9y)

![](https://drive.google.com/uc?export=view&id=1-n0TN0HmKztfBdSchap_FjbiZ-qHbsSg)

![](https://drive.google.com/uc?export=view&id=1fuo36302poXQryqdjfZfYWNwKtPZT_Ka)

### FYI

This project was created as an educational project in order to get familiar with the C programming language, back in 2017. The code is not very well written and can be improved a lot.

If you are new to programming and want a small project to start learning this one can be a good start.

Hope that the comments from the code helps new programmers to get started.

### TODO
* Improve memory usage
* Solve resize bugs
