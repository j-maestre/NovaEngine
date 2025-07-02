for %%i in (*.png) do texconv.exe -o . -ft DDS -reconstructz -if LINEAR -f BC6H_UF16 -pmalpha -hflip -y -dx10 -cube "%%i"
for %%i in (*.jpg) do texconv.exe -o . -ft DDS -reconstructz -if LINEAR -f BC6H_UF16 -pmalpha -hflip -y -dx10 -cube "%%i"
pause