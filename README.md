# CETRIS

A Tetris game developed in C.
You can play in solo or you can watch Jarvis (The name of our AI) play alone.
This project was developed at ENSEIRB-MATMECA (Bordeaux, FRANCE) in a
course of initiation in C.

## Getting Started

This is a C project and we use Make to compile it.
Please follow the nexts steps.

### Prerequisites

+ Make to build the app
+ Curses library
+ Color suported terminal

### Installing

Download the project and extract it.
Go in the main folder with a terminal and type 'make all' to compile the app
Launch the game by typing './Cetris'

### Play !

Differents modes are available in the main menu :
+ *LOGIN* : Edit your login, it will appear in the scoreboard
+ PLAY : Normal mode
+ B MODE : Begin the game with an half filled board
+ NO WALL : Play without right and left walls
+ QUIT : Quit the game :(

In the 3 gameplay, you can play in solo or give the commands to Jarvis the AI
He plays very well to the game.
You can increase his speed by pressing 'H', take care of your processor !
In our last test, he placed 10 millions of tetri in 1 day and 6 hours. (Speed avg: 90tetri/sec)

## Good to know

+ Board has a fixed area of 255 pixels
+ Config file are saved near the binary
  - Multi-instance of Cetris is possible
+ 2500 lines of code
+ Jarvis is not a real Artificial Inteligence
  - No deep learning, he just tests all possibilities
  - No optimisation when a branch seems to be a bad idea
+ Mesure working time and sleep '1ms - work_time' to get 1ms period

## Make's options

+ 'make all'
  - Standard version of Cetris
+ 'make debug'
  - No change from standard
+ 'make spymode'
  - Enable AI_SPY_MODE to look how AI process tests

## Build options

+ NB_TETRI_COMMING : Number of tetri in the vision (Default 3)
  - Also the number of tetri which are compute by Jarvis
+ POS_GUIDE_LVL_MAX : Level where guide lines are removed (Default 2)
+ DROP_SPEED : Speed of tetri when they are drop. Faster : 1 (Default 8)
+ AI_SPY_MODE : Force render when AI works (Default Not defined)
+ LOG_USE_COLOR : Use color in log file
+ DEBUG : Enable debug mode


## Built With

* [NCurses](https://www.gnu.org/software/ncurses/) - The graphic library used

## Authors

* **David DEVANT** - *Initial work* - [Devdevdavid](https://github.com/Devdevdavid)
* **Aurélien TROMPAT** - *Initial work* - [DjNounours](https://github.com/DjNounours)

## Improvements

If you want to add functions to the game you are welcome !




