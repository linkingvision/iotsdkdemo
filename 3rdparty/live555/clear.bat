FOR /D /R %%X IN (*.tlog) DO RD /S /Q "%%X"

rmdir %~dp0\Debug /s /q
rmdir %~dp0\Release /s /q
rmdir %~dp0\x64 /s /q
rmdir %~dp0\.vs /s /q


