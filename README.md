# VUT-IZG-projects (2022/2023)

This repository consists of tasks for IZG classes on BUT FIT for the year 2022/2023.
Also there is one small code from Processing that was done in one lecture, so I tried to code it myself.
So, if you want, you can also check it out, it is definitely helpful.
In this repository, there are both:

- exercises/labs
- project
- processing (pong)

## Exercises/Labs

In `exercises/.`, there are all the finished codes for all the labs but **use them only as an inspiration**.
**Make sure you understand the given task** before you just `Ctrl + C`, `Ctrl + V` anything from this repository.
You might be tested or examined by the exercise leader, that you understand the code you have written (at least we were).
So please, look at the code **only as the last resort**.
Also, understanding the topic will help you later when writing the exam.

### Note

The whole code is not mine. The labs work in the way that we get a skeleton of a project, that we then have to correctly fill with code (meaning I only wrote certain functions or generally parts of the source file).
Names of true authors of the labs are mentioned in every lab file with name matching `cv[1-5].cpp`. For `cv6.glsl` we were given the task verbally, then wrote the code ourselves.

### Disclaimer

These solutions are only for the year 2022/2023, I do not guarantee, that the assignments won't change and the codes might not work in the future.

## Project

The project (in `project/.`) of this class was aimed on basically programming the graphics card.
To understand how the *OpenGL pipeline* works, what should the optimizations look like, how fast can it be.
Note that this project is **not very well optimized** and can be greatly sped up.
The whole project is written in `C++` and is tested on:

- Windows 11
- MacOS Ventura
- Ubuntu 22.04

## Processing

In `processing/.` there is a tiny (about 100 lines) code in processing language that is a basically a very simple game of Pong for one player.
The ball just bouces of the wall on the other side of the player.
If the ball goes behind the player, the game ends and you can just quit it and the run it again to play the game.

### How to run

You need to download and install [Processing](https://processing.org/download) that is available on basically all the platforms.
After the installation, a little editor will open. Then in the `File` tab, you click `Open` and select the `lecture_pong.pde` file from this repository.
After this, you can just click the play button, that will compile code and run the game.

**Please note that the game was tested on Processing version 4.2.**

## Exercises evaluations

- cv1: 3/3
- cv2: 3/3
- cv3: 3/3
- cv4: 3/3
- cv5: 3/3
- cv6: 1.5/3

## Project evaluation

17.65/20 (clipping is not working correctly)
