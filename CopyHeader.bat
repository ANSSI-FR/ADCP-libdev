REM @echo off

mkdir Include
cd Include
del *.h
cd ..

copy LibCache\src\CacheLib.h Include
copy LibCsv\src\CsvLib.h Include
copy LibJson\src\JsonLib.h Include
copy LibLdap\src\LdapLib.h Include
copy LibLdap\src\LdapHelpers.h Include
copy LibLog\src\LogLib.h Include
copy LibUtils\src\UtilsLib.h Include
copy LibUtils\src\UtilsGetoptSimple.h Include
copy LibUtils\src\UtilsGetoptComplex.h Include

