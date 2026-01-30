@echo off
setlocal enabledelayedexpansion

title STM32工程批量瘦身工具
color 0A

echo =================================================================
echo      正在深度扫描当前目录及所有子目录...
echo      目标：清理编译垃圾 + 移除 DSP/NN/RTOS/CMSIS-Core 冗余库
echo =================================================================
echo.

:: =================================================================
:: 第一部分：清除编译产生的临时文件 
:: 解释：这些 .o, .d, .crf 都是编译时生成的，删了也能重新编译出来
:: =================================================================

echo [1/6] 正在清理编译垃圾文件 (*.o, *.axf, *.crf...)...
del *.bak /s >nul 2>nul
del *.ddk /s >nul 2>nul
del *.edk /s >nul 2>nul
del *.lst /s >nul 2>nul
del *.lnp /s >nul 2>nul
del *.mpf /s >nul 2>nul
del *.mpj /s >nul 2>nul
del *.obj /s >nul 2>nul
del *.omf /s >nul 2>nul
del *.plg /s >nul 2>nul
del *.rpt /s >nul 2>nul
del *.tmp /s >nul 2>nul
del *.__i /s >nul 2>nul
del *.crf /s >nul 2>nul
del *.o /s >nul 2>nul
del *.d /s >nul 2>nul
del *.axf /s >nul 2>nul
del *.tra /s >nul 2>nul
del *.dep /s >nul 2>nul           
del JLinkLog.txt /s >nul 2>nul
del *.iex /s >nul 2>nul
del *.htm /s >nul 2>nul
del *.sct /s >nul 2>nul
del *.map /s >nul 2>nul
del *.dbgconf /s >nul 2>nul
del *.uvguix.* /s >nul 2>nul

echo     - 编译垃圾清理完毕。
echo.

:: =================================================================
:: 第二部分：递归删除冗余库文件夹
:: =================================================================

echo [2/6] 正在搜索并删除 DSP 库 (数字信号处理库)...
for /f "delims=" %%d in ('dir /s /b /ad "DSP"') do (
    if exist "%%d" (
        :: 只有路径里包含 CMSIS 才删，防止误删用户自己建的 DSP 文件夹
        echo "%%d" | findstr "CMSIS" >nul
        if !errorlevel! equ 0 (
            echo     - 删除: %%d
            rd /s /q "%%d"
        )
    )
)

echo [3/6] 正在搜索并删除 NN 库 (神经网络库)...
for /f "delims=" %%d in ('dir /s /b /ad "NN"') do (
    if exist "%%d" (
        echo "%%d" | findstr "CMSIS" >nul
        if !errorlevel! equ 0 (
            echo     - 删除: %%d
            rd /s /q "%%d"
        )
    )
)

echo [4/6] 正在搜索并删除 RTOS 模版 (操作系统库)...
:: 同时查找 RTOS 和 RTOS2 文件夹
for /f "delims=" %%d in ('dir /s /b /ad "RTOS*"') do (
    if exist "%%d" (
        :: 必须确认是在 CMSIS 目录下才删除
        echo "%%d" | findstr "CMSIS" >nul
        if !errorlevel! equ 0 (
            echo     - 删除: %%d
            rd /s /q "%%d"
        )
    )
)

echo [5/6] 正在搜索并删除 CMSIS 文档 (docs)...
for /f "delims=" %%d in ('dir /s /b /ad "docs"') do (
    if exist "%%d" (
        echo "%%d" | findstr "CMSIS" >nul
        if !errorlevel! equ 0 (
            echo     - 删除: %%d
            rd /s /q "%%d"
        )
    )
)

echo.
echo [6/6] 正在搜索并删除冗余 Core 文件夹 (关键操作)...
echo     * 安全检查：仅删除 Drivers\CMSIS 下的 Core，保留用户代码 Core
for /f "delims=" %%d in ('dir /s /b /ad "Core"') do (
    if exist "%%d" (
        :: 【安全锁】只有当路径包含 "Drivers\CMSIS" 时才执行删除
        echo "%%d" | findstr /c:"Drivers\CMSIS" >nul
        if !errorlevel! equ 0 (
            echo     - 删除冗余模版: %%d
            rd /s /q "%%d"
        )
    )
)

echo.
echo =================================================================
echo 全部操作完成！所有子工程已应用瘦身策略。
echo =================================================================
pause