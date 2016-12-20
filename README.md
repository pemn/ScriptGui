# ScriptGui
Data driven Windows gui for command line scripts.

## Description
This C++ application is a standalone graphical interface toolkit for giving command line scripts a intuitive for user input. The user can fill the fields then click "Run" to call a script with the parameters selected as the command line arguments.

## Features
- Fully standalone. No runtimes (other than MSVC, and even this can be optionally removed by compiling with static runtime)
- Data driven. The entire interface is defined with a single line with special control templates. This template line can be a comment line, so your code is effectively unchanged. The only thing you must ensure is to read the command line parameters which will contain the user input.
- Flexible. Any language that can somehow read the command line arguments can be supported. Custom handlers may be easily implemented too.
- Small. This application does not use big toolkits that add a huge footprint to the executable. The entire app is ~ 130 Kb in size. The only library used is WTL, a header only template OO library wrapping Win32 classes.
- Future proof and backward compatible. This interface uses the Win32 API, which is guaranteed to work on all contemporary desktop Windows versions.
- Simple. The arguments are passed as simple text.
- Save and load parameters as named sets. Those parameters sets are stored as xml files in the same folder, and can be copied and edited. For this reason, the file browse controls will automatically convert paths descendant from current working diretory to relative paths so they stay valid even if the precedding hierachy changes.
- Copy command line. Not only the this application is a interface, but it also can creates a command line for your script so you can run you script with the selected arguments even where you dont have a window server, such as ssh session or a already existing command line prompt session.
- Free. The only used library is WTL, which is licensed under the Microsoft Public License. The code is licensed as Apache 2.0. 

## Screenshot
![screenshot](https://github.com/pemn/ScriptGui/blob/master/assets/screenshot1.png)

## How to use
### File name matters
The interfaces searches in the current working directory for all files that have the same base name, and are of a supported extension.
Ex.: "myscript.exe" will match a file named "myscript.py".

### The `usage:` line
Once the interface finds a compatible script, it will do a text search on the contents of this file looking for a magic word: `usage:`  
When it is found, its line is parsed into interface controls using the templates described below. This `usage:` can be a comment or, even better, can be the short help message describing its usage. Its common for a script to print their parameter syntax when called with a switch such as `/?` and `-h` or without any arguments. The origin of this interface was trying to "guess" a good control layout from existing usage lines. Conforming any already existing usage line to the templates should have no downsides and will even look informative.

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

### Example of a "usage:" line
`usage: %0 input*csv selection=a,b,c numeric=123 boolean@ output*csv`  

### Limitations
This interface was developped to catter to a already existing ecosystem of scripts and contains some custom funcionalities and assumptions that may require adjusting.  
It alsos relies on the Win32 API which is quirky and hard to understand.  

## License
Apache 2.0

