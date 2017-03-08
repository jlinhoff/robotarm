// main.cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////

void help(const char *program)
{
   printf("usage:\n");
   printf("%s [OPTIONS]\n",program);
   printf("OPTIONS: -?  -- show this help message\n");
   printf("--------------- commands -------------\n");
   printf("size [n] - Adjusts the number of slots, resizing if necessary. Program must start with this to be valid.\n");
   printf("add [slot] - Adds a block to the specified slot.\n");
   printf("mv [slot1] [slot2] - Moves a block from slot1 to slot2.\n");
   printf("rm [slot] - Removes a block from the slot.\n");
   printf("replay [n] - Replays the last n commands.\n");
   printf("undo [n] - Undo the last n commands.\n");
   printf("help - Prints this help.\n");
   printf("quit - Exit command loop and exit program.\n");
} // help()

////////////////////////////////////////////////////////////////////////////////

class RoboArm {
public:
   RoboArm();
   ~RoboArm();
   // all commands return <0 on error, >=0 on success
   int setProgramName(const std::string &name); // set program name
   std::string getProgramName();
   int loop(); // command loop
   int show(); // show the slots
   int cmdSize(int numSlots); // adjust number of slots
   int cmdAdd(int slot); // add a block
   int cmdMove(int slotSrc,int slotDst); // move a block
   int cmdRemove(int slot); // remove a block
   int cmdReplay(int numCommands); // replay last num commands
   int cmdUndo(int numCommands); // undo the last num commands

private:
   struct CmdRec {
      CmdRec() : cmd(-1),arg1(0),arg2(0) {}
      int cmd; // CMD_
      int arg1;
      int arg2;
   };
   int play(const CmdRec &cmd);
   int histAdd(const CmdRec &cmd);
   std::string programName;
   std::vector<int> slotCollection;
   std::vector<CmdRec> cmdHistCollection;
}; // class RoboArm
   
RoboArm::RoboArm() {
}

RoboArm::~RoboArm() {
}

int RoboArm::setProgramName(const std::string &name) {
   programName=name;
   return 0;
} // setProgramName()

std::string RoboArm::getProgramName()
{
   return programName;
} // getProgramName()

const char *whitespace = " \t";
   
int findFirstWord(std::string &outFirstWord,const std::string &str)
{ // find the first word -- whitespace delimited, return index after first word

   int first = (int)str.find_first_not_of(whitespace);
   if(first == (int)std::string::npos)
      first=0;
   int last = (int)str.find_first_of(whitespace,first);
   if(last == (int)std::string::npos)
      last=str.length();
   std::string word = str.substr(first,last);
   outFirstWord = word;
   return last;
} // findFirstWord()

// command table
#define CMDS_X \
   X(size) \
   X(add) \
   X(mv) \
   X(rm) \
   X(replay) \
   X(undo) \
   X(help) \
   X(quit)

// expand enum table
#define X(_a_) CMD_##_a_,
enum { CMDS_X };
#undef X

// expand name table
#define X(_a_) #_a_,
const char *cmdTable[] = { CMDS_X 0 };
#undef X

int findIndexOfString(const char *strTable[],const char *str)
{
   for(int i=0;strTable[i];i++) {
      if(!strcmp(strTable[i],str)) // could make case insensitive here
         return i;
   } // for
   return -1; // not found
} // findIndexOfString()

#define RETURNCODE_NO_HISTORY (-999)

int RoboArm::play(const CmdRec &cmd)
{ // run a single command
   int r=0;
   switch(cmd.cmd) {
   case CMD_size:
      r=cmdSize(cmd.arg1);
      break;
   case CMD_add:
      r=cmdAdd(cmd.arg1);
      break;
   case CMD_mv:
      r=cmdMove(cmd.arg1,cmd.arg2);
      break;
   case CMD_rm:
      r=cmdRemove(cmd.arg1);
      break;
   case CMD_replay:
      r=cmdReplay(cmd.arg1);
      break;
   case CMD_undo:
      r=cmdUndo(cmd.arg1);
      break;
   default:
      r=-1;
   } // switch
   return r;
} // RoboArm::play()

int RoboArm::loop() {
   int r=0;
   for(;;) {
      std::string line;
      std::string word;
      CmdRec cmd;
      char *s2;
      std::cout << "> ";
      std::getline(std::cin,line);
      
      // get first word and advance line to next arg
      if((r = findFirstWord(word,line))<0)
         word.clear(); // problem
      else {
         r = line.find_first_not_of(whitespace,r);
         if(r == (int)std::string::npos)
            r = line.length();
         line = line.substr(r);
      }
   
      cmd.cmd=findIndexOfString(cmdTable,word.c_str());
      switch(cmd.cmd) {
      case CMD_size:
         cmd.arg1 = strtol(line.c_str(),0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_add:
         cmd.arg1 = strtol(line.c_str(),0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_mv:
         cmd.arg1 = strtol(line.c_str(),&s2,0); // could use std::stoi() if we use C++11
         if(!s2)
            goto syntax_error;
         cmd.arg2 = strtol(s2,0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_rm:
         cmd.arg1 = strtol(line.c_str(),0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_replay:
         cmd.arg1 = strtol(line.c_str(),0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_undo:
         cmd.arg1 = strtol(line.c_str(),0,0); // could use std::stoi() if we use C++11
         break;
      case CMD_help:
         help(getProgramName().c_str());
         cmd.cmd=-1; // don't play
         break;
      case CMD_quit:
         r=0;
         goto FINALIZE;
      default:
      syntax_error:
         std::cout << "* syntax error, command '" << word << "' not handled" << std::endl;
         cmd.cmd=-1; // don't play
         break;
      } // switch
      
      // play the command & add to history if successful
      if(cmd.cmd>=0) {
         if((r=play(cmd))<0) {
            if(r!=RETURNCODE_NO_HISTORY)
               std::cout << "* error, command '" << word << "'" << std::endl;
         } 
         else {
            histAdd(cmd);
         }
      }

      // show the state
      show();
   } // for
FINALIZE:
   return r;
} // RoboArm::loop()

int RoboArm::show() {
   for(int i=0;i<(int)slotCollection.size();i++) {
      std::cout << i << ": ";
      std::cout << std::string(slotCollection[i],'X');
      std::cout << std::endl;
   } // for
   return 0;
} // RoboArm::show()

#define MAX_SLOTS 1000000

int RoboArm::cmdSize(int numSlots)
{  // adjust number of slots
   if((numSlots<=0)||(numSlots>MAX_SLOTS))
      return -1;
   slotCollection.resize(numSlots);
   return 0; 
} // RoboArm::cmdSize()

int RoboArm::cmdAdd(int slot)
{ // add a block
   if((slot<0)||(slot>=(int)slotCollection.size()))
      return -1;
   slotCollection[slot]++;
   return 0; 
}
int RoboArm::cmdMove(int slotSrc,int slotDst)
{ // move a block
   if((slotSrc<0)||(slotSrc>=(int)slotCollection.size()))
      return -1;
   if((slotDst<0)||(slotDst>=(int)slotCollection.size()))
      return -2;

   if(slotCollection[slotSrc]<=0) {
      std::cout << "- no blocks in source slot " << slotSrc << " to move" << std::endl;
      return 1; // warning, not an error
   }
   
   slotCollection[slotSrc]--;
   slotCollection[slotDst]++;
   
   return 0; 
}

int RoboArm::cmdRemove(int slot)
{  // remove a block
   if((slot<0)||(slot>=(int)slotCollection.size()))
      return -1;

   if(slotCollection[slot]<=0) {
      std::cout << "- no blocks in slot " << slot << " to remove" << std::endl;
      return 1; // warning, not an error
   }
   
   slotCollection[slot]--;
   return 0; 
}

int RoboArm::cmdReplay(int numCommands)
{ // replay last num commands (in same order)
   int r;
   if(numCommands>(int)cmdHistCollection.size())
      return -1;
   
   while(numCommands>0) {
      numCommands--;
      if((r=play(cmdHistCollection[numCommands]))<0) {
         std::cout << "* problem replaying cmd=" << cmdHistCollection[numCommands].cmd << std::endl;
         return -1;
      }
   } // while
   return RETURNCODE_NO_HISTORY;
} // RoboArm::cmdReplay()
 
int RoboArm::cmdUndo(int numCommands)
{ // undo the last num commands
   int r;
   if(numCommands>(int)cmdHistCollection.size())
      return -1;
   for(int i=0;i<numCommands;i++) {
      CmdRec cmd = cmdHistCollection[i];
      switch(cmd.cmd) {
      case CMD_add:
         cmd.cmd = CMD_rm;
         break;
      case CMD_rm:
         cmd.cmd = CMD_add;
         break;
      case CMD_mv:
         r = cmd.arg1;
         cmd.arg1=cmd.arg2;
         cmd.arg2=r;
         break;
      default:
         std::cout << "* problem not-handled undo [ " << i << "] cmd=" << cmd.cmd << std::endl;
         return -1;
      }
   
      if((r=play(cmd))<0) {
         std::cout << "* problem r=" << r << " playing undo [ " << i << "] cmd=" << cmd.cmd << std::endl;
         return -1;
      }
   } // for
   return RETURNCODE_NO_HISTORY; 
} // RoboArm::cmdUndo()  
   
#define MAX_HIST 1000
int RoboArm::histAdd(const CmdRec &cmd)
{
   cmdHistCollection.insert(cmdHistCollection.begin(),cmd);
   if(cmdHistCollection.size()>MAX_HIST)
      cmdHistCollection.resize(MAX_HIST);
   return 0;
} // histAdd()

////////////////////////////////////////////////////////////////////////////////

int main(int argc,char *argv[])
{
   int r=0; // return value
   const char *program;
   const char *str;

   // get program name -- handle either forward or backward slash
   program=*argv++,argc--;
   while((str=strchr(program,'\\'))||(str=strchr(program,'/')))
      program=str+1;
      
   printf("%s\n",program);
   
   while(argc>0) {
      str = *argv++,argc--;
      switch(*str) {
         default:
            printf("* argument '%s' not handled\n",str);
            help(program);
            r=-1; // signal not-successful
            break;
         case '-':
            switch(str[1]) {
               default:
                  printf("* switch '%c' not handled\n",str[1]);
                  help(program);
                  r=-1;
                  break;
            } // switch
            break;
      } // switch
   } // while
   
   if(r>=0) {
      RoboArm arm;
      std::string name(program);
      arm.setProgramName(name);
      r = arm.loop();
   }
   
   return r;
} // main()

// EOF