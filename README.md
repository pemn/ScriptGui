# ScriptGui
Data driven Windows gui for command line scripts.
## Description
This C++ application is a standalone graphical interface toolkit for giving command line scripts a intuitive for user input. The user can fill the fields then click "Run" to call a script with the parameters selected as the command line arguments.
## Features
- Fully standalone. No runtimes (other than MSVC, and even this can be optionally removed by compiling with static runtime)
- Data driven. The entire interface is defined with a single line with special control templates. This template line can be a comment line, so your code is effectively unchanged. The only thing you must ensure is to read the command line parameters which will contain the user input.
- Flexible. Any language that can somehow read the command line arguments can be supported. Custom handlers may be easily implemented too.
- Small. This application does not use big toolkits that add a huge footprint to the executable. The entire app is ~ 130 Kb in size. The only library used is WTL, a header only template OO library wrapping Win32 classes.
- Compatible. This interface uses the Win32 API, which is guaranteed to work on most Windows versions, from XP to Windows 10.
- Simple. The arguments are passed as simple text.
- Free. The only used library is WTL, which is licensed under the Microsoft Public License.  
## Screenshot
![screenshot](https://github.com/pemn/ScriptGui/blob/master/assets/screenshot1.png)
## How to use
### File name matters
The interfaces searches in the current working directory for all files that have the same base name, and are of a supported extension.
Ex.: "myscript.exe" will match a file named "myscript.py".
### The "usage:" line
Once the interface finds a compatible script, it will do a text search on the contents of this file looking for a magic word: `usage:`  
When it is found, its line is parsed into interface controls using the templates described below.
### Control templates
- `<name>=<value>`  
text input control
- `<name>*<extension>`  
file browse control
- `<name>:<another control name>`  
derived control that gets its list values from the file pointed by another file browse control
- `<name>@`  
checkbox boolean control
- `<name>#<control>` 
grid of controls, allowing a the user to create a list of similar values
