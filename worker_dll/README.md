# RunDLL
This directory contains the boilerplate for creating a DLL with a run function that will be called by the interface.  
Once compiled, rename it with the same name as the interface EXE and it should read the `usage:` parameters from the DLL FileDescription metadata.  
The FileDescription metadata is defined on the `.rc` file.  
The payload code should go into the `run` function template in the `.h` file.  
The `.cpp` file should not be modified in most cases, since we are using templates the only static function is DllMain.
