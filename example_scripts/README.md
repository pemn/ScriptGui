# Test Dummy Scripts
Here we store minimal tests scripts for the interface. They all have a `usage:` line and echo the command line arguments back to the STDOUT.  
Some are more elaborate other more simple, and are by no means examples of how a real script should be. Those scripts are not really tests scripts, but rather dummies that can be used to check if the basic funcionallity of the interface is working.  
Copy the script from a given language to the same directory as the interface executable and run the executable. It should call the script.  
The apropriated interpreter should be visible in the executable enviroment. This usualy means ensuring that the PATH enviroment variable contains the directory of the interpreter. Ex.: For python to work, `c:\python` must be included in the PATH. Otherwise the following error message will appear:  
![msg](https://github.com/pemn/ScriptGui/blob/master/assets/error_system_cannot_find.png)
