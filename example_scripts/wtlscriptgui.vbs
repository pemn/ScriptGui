' boilerplate to a generic argument based WScript
' v1.0

if WScript.Arguments.Count = 0 Then
    WScript.Echo("usage: <script> input*csv output*csv")
    WScript.Quit(1)
End If

Dim strArg
Dim i

For i = 0 To WScript.Arguments.Count - 1
    strArg = strArg & " " & WScript.Arguments(i)
Next

WScript.Echo("Arguments:" & strArg)
