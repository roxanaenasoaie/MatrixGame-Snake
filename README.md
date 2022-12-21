# MatrixGame-Snake

## Task Requirements

###MENU
Create a menu for a game, emphasis on the game. Scroll on the LCD with the joystick. The menu should include the following functionality:
1.  When powering up a game, a greeting message should be shown fora few moments.
2.  Should contain roughly the following categories:
(a)Start game, starts the initial level of your game
(b)Highscore:1 -Initially, we have 0. –Update  it  when  the  game  is  done. –Save the top 5+ values in EEPROM with name and score.
(c)Settings: –Enter name. The name should be shown in highscore. 
	–Starting  level:  Set  the  starting  level  value.  
	–LCD contrast control (optional, it replaces the potentiome-ter).  Save it to eeprom.
	–LCD brightness control (mandatory, must change LED wire that’s directly connected to 5v).  Save it to eeprom.
	–Matrix brightness control (see function setIntesnity from the ledControl library).  Save it to eeprom.
	–Sounds on or off.  Save it to eeprom.
	–Extra stuff can include items specific to the game mechanics,or other settings such as chosen theme song etc.  Again, saveit to eeprom.
(d)About: should include details about the creator(s) of the game. At least game name, author and github link or user 
(e)How to play: short and informative description

3.While playing the game: display all relevant info. Upon game ending:
(a)  Screen 1: a message such as ”Congratulations on reaching level/score X”.  ”You did better than y people.”.  etc.  Switches to screen 2 upon interaction (button press) or after a few moments.
(b)  Screen 2:  display relevant game info:  score, time, lives left etc.Must  inform  player  if  he/she  beat  the  highscore.Thismenu should only be closed by the player, pressing a button


###GAME
Minimal components: an LCD, a joystick, a buzzer and the ledmatrix.
–Add basic sounds to the game (when ”eating” food, whendying, when finishing the level etc). Extra:add theme songs.
–It must be intuitive and fun to play.–It must make sense in the current setup.  
–The levels must progress dynamically.  

**What I did**
[![matrix.jpg](https://i.postimg.cc/jSSYCzmF/matrix.jpg)](https://postimg.cc/qzS55Cks)

**Functionality**
[Video](https://youtu.be/uuDMRFF1mbM)

**Used Components**
1. LCD
2. Matrix
3. Joystick
4. Breadboards
5. Shift Register
6. Wires