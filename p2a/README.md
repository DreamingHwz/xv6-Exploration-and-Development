# P2A-Shell

A Unix shell is built. It includes a file "wish.c".

## Description

The shell supports these commands:
* ```exit```: exit the shell
* ```cd```: change directory
* ```path```: set path for executables
* ```if```: if statement like ```if command == 0 THEN command fi```
* other executable commands in ```/bin```, such as ```ls```, ```vim```.

To run the shell:
* ```gcc wish.c -o wish```
* For interactive mode:  ```./wish```, For batch mode: ```./wish batch.txt```
* Input command after prompt ```wish>```. For redirection, use ```>``` character like ```ls > output``` (to make my shell uers happy).


## Author
Skylar Hou 

10.12.2022 at UW-Madison
