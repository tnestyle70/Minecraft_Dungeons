// 명령어			옵션				원본 파일의 위치			사본 파일을 저장할 위치

xcopy			/y				.\Engine\Header\*.h			.\Reference\Header
xcopy			/y				.\Engine\Bin\Engine.lib		.\Reference\Library
xcopy			/y				.\Engine\Bin\Engine.dll		.\Client\Bin
xcopy			/y				.\Engine\Bin\fmod_vc.lib		.\Client\Bin