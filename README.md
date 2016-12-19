# ScriptGui
Data driven Windows gui for scripts  
## Description  
This C++ application is a standalone graphical interface toolkit for giving command line scripts a friendly user interface. The user can fill the field then click "Run" to call a script with the parameters selected as the command line arguments.  
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
