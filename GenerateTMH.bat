REM @echo off

set wpppath="C:\Program Files (x86)\Windows Kits\10\bin\10.0.16299.0\x64\tracewpp.exe"
set wppconfig="C:\Program Files (x86)\Windows Kits\10\bin\10.0.16299.0\WppConfig\Rev1"

cd LibLdap\src
%wpppath% -cfgdir:%wppconfig% -scan:LdapWpp.h *.c *.cpp

cd ..\..\LibJson\src
%wpppath% -cfgdir:%wppconfig% -scan:JsonWpp.h *.c *.cpp

cd ..\..\LibCsv\src
%wpppath% -cfgdir:%wppconfig% -scan:CsvWpp.h *.c *.cpp

cd ..\..\