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

echo.
echo Finished
echo.

cd "%~dp0"

pause
