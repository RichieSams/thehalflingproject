for /r %%f in (*) do (
    texconv.exe -ft dds -o %%~dpf %%f
) 