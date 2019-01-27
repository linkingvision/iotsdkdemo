FOR /D /R %%X IN (*.tlog) DO RD /S /Q "%%X"

rmdir %~dp0\prj_win\Debug /s /q
rmdir %~dp0\prj_win\Release /s /q
rmdir %~dp0\prj_win\x64 /s /q

rmdir %~dp0\prj_win\.vs\ /s /q



