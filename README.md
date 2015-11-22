# itcgoal
Source code of GOAL solver, best ranked solver in ITC2011

Pre-requirements:

NetBeans IDE 8.0.2 or newer (download the version that supports all technologies)
Expat library
For Unix users these command lines makes the trick:
sh netbeans-8.0.2-linux.sh
sudo apt-get install libexpat1-dev
sudo apt-get install build-essential
sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer

How to run the solver:

Open NetBeans
File -> Open Project -> select the project of Goal Solver
Rigth click on the project -> Properties
C++ compiler -> set Additional options as "-std=c++11"
Linker -> set Libraries as "-pthread -lexpat -lm"
Run the project (F6)
