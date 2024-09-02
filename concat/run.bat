"C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x64\dxc" -T "cs_6_2" -E CSMain -enable-16bit-types -Fo concat.cso concat.hlsl

xcopy concat.cso .\build\Debug\ /Y

ConcatApp.exe 4 100 concat.cso

