@echo off
MSBuild.exe transfile.sln /p:Configuration=Release
robocopy .\x64\Release\ .\win64\ /purge /xo /e
pause