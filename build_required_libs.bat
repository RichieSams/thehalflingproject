@echo off

REM set msbuild=%WINDIR%\Microsoft.NET\Framework64\v3.5\MSBuild.exe
set msbuild=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\MSBuild.exe

%~d0
cd "%~dp0source\libs\assimp\workspaces\vc"

echo.
echo Building assimp...
echo.

%msbuild% assimp.vcxproj /p:configuration=debug /p:platform=win32 /v:m /nologo
%msbuild% assimp.vcxproj /p:configuration=release /p:platform=win32 /v:m /nologo
%msbuild% assimp.vcxproj /p:configuration=debug /p:platform=x64 /v:m /nologo
%msbuild% assimp.vcxproj /p:configuration=release /p:platform=x64 /v:m /nologo

cd "%~dp0source\libs\json-cpp\makefiles\vs71"

echo.
echo Building json-cpp...
echo.

%msbuild% lib_json.vcxproj /p:configuration=debug /p:platform=win32 /v:m /nologo
%msbuild% lib_json.vcxproj /p:configuration=release /p:platform=win32 /v:m /nologo
%msbuild% lib_json.vcxproj /p:configuration=debug /p:platform=x64 /v:m /nologo
%msbuild% lib_json.vcxproj /p:configuration=release /p:platform=x64 /v:m /nologo

cd "%~dp0source\libs\fastformat\projects\core\vc9"

echo.
echo Building fastformat...
echo.

%msbuild% fastformat.core.vcxproj /p:configuration="Unicode Debug Multithreaded" /p:platform=win32 /v:m /nologo
%msbuild% fastformat.core.vcxproj /p:configuration="Unicode Release Multithreaded" /p:platform=win32 /v:m /nologo
%msbuild% fastformat.core.vcxproj /p:configuration="Unicode Debug Multithreaded" /p:platform=x64 /v:m /nologo
%msbuild% fastformat.core.vcxproj /p:configuration="Unicode Release Multithreaded" /p:platform=x64 /v:m /nologo

echo.
echo Finished
echo.

cd "%~dp0"

pause
