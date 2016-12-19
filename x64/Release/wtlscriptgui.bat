@echo off

if "%1" equ "" (
    echo "usage: $0 input*csv selection=a,b,c numeric=123 boolean@ output*csv"
    goto :EOF
)

echo argv: %*


