############################################################
                           CETRIS
############################################################

Date : January, 2018

Authors : David Devant
          Aurélien Trompat

================
 How to start ?
================

> Download the project
> Compile it:
	- "make all" for normal version
	- "make debug" for debug version
> Launch binary to play:
	- "./cetris"

================
 Make's options
================
> make all
  - Standard version of Cetris
> make debug
  - No change from standard
> make spymode
  - Enable AI_SPY_MODE to look how AI process tests

=======================
 Compilation's options
=======================

> NB_TETRI_COMMING : Number of tetri in the vision (Default 3)
  - Also the number of tetri which are compute by Jarvis
> POS_GUIDE_LVL_MAX : Level where guide lines are removed (Default 2)
> DROP_SPEED : Speed of tetri when they are drop. Faster : 1 (Default 8)
> AI_SPY_MODE : Force render when AI works (Default Not defined)
> LOG_USE_COLOR : Use color in log file

================
  Requirements
================

> Make to build the app
> Curses library
> Color suported terminal

================
  Good to kown
================

> Board has a fixed area of 255 pixels
> Config file are saved near the binary
  - Multi instance of Cetris is possible
> 2500 lines of code
> Jarvis is not a real Artificial Inteligence
  - No deep learning, he just tests all possibilities
  - No optimisation when a branch seems to be a bad idea
> Mesure working time and sleep (1ms - work_time) to get 1ms period





