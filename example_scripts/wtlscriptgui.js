// boilerplate to a generic argument based WScript
// v1.0

if(WScript.Arguments.length == 0) {
    WScript.Echo("usage: script.js input*csv output*csv");
    WScript.Quit(1);
}

var strArg = ""

for(var i=0;i < WScript.Arguments.length; i++) {
    strArg += WScript.Arguments(i) + " ";
}
WScript.Echo("Arguments: " + strArg);
