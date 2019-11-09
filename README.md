# log filtering  
The libraries in this project can be used as preload to organize some verbose log filtering    

## Use cases  

From time to time there are applications (mainly servers) producing to much log. For example recently we observed that ZMQ based 
eventNumber-time receivers in the PITZ DAQ hosts producing to much logs (90GB in the pitzdaq5) in the case of some conditions.  In order to 
solve such verbos server issues w can redirect error or output pipe to /dev/nul, but this is not always acceptable, because from time 
to time we have some important information, that we need access. If we do not have chance to open and modify source and compilethe server agai
then one of the solution, where we will have logging, but suppressed is using LD_PRELOAD, to include there some logic of filtering unnnecessary 
logs.  


## What is implemented in this repo  

### Example (test) applications of log filtering 
  
