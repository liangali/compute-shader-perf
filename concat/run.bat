
::"C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x64\dxc" -T cs_5_0 -E CSMain -Fo concat.cso concat.hlsl

"C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x64\dxc" -T "cs_6_2" -E CSMain -enable-16bit-types -Fo concat.cso concat.hlsl

::"C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x64\dxc" -E CSMain -Lx -Fo concat.cso -T "cs_6_0" concat.hlsl

xcopy concat.cso .\build\Debug\ /Y

pause