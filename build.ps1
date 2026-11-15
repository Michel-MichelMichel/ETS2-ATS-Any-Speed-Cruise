# Builds (64-bit) with Zig's bundled C++ compiler:
#   build\any_speed_cruise.dll         the plugin
#   build\any_speed_cruise_config.exe  the key setup tool
# Requires: zig in PATH (scoop install zig).
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$src  = Join-Path $root 'src'
$sdk  = Join-Path $root 'third_party\scs_sdk_1_16\include'
$out  = Join-Path $root 'build'

if (-not (Test-Path (Join-Path $sdk 'scssdk.h'))) {
    throw "SCS SDK not found in $sdk. Download it from https://modding.scssoft.com/wiki/Documentation/Engine/SDK/Telemetry"
}
New-Item -ItemType Directory -Force $out | Out-Null

zig c++ -target x86_64-windows-gnu -shared -O2 -s `
    -Wall -Wextra -Wno-unused-parameter `
    "-I$sdk" `
    (Join-Path $src 'any_speed_cruise.cpp') `
    (Join-Path $src 'any_speed_cruise.def') `
    -lwinmm `
    -o (Join-Path $out 'any_speed_cruise.dll')
if ($LASTEXITCODE -ne 0) { throw "Plugin build failed ($LASTEXITCODE)" }

zig c++ -target x86_64-windows-gnu -O2 -s "-Wl,--subsystem,windows" `
    -Wall -Wextra -Wno-unused-parameter `
    (Join-Path $src 'config_tool.cpp') `
    (Join-Path $src 'config_tool.rc') `
    -lwinmm -lgdi32 -lcomctl32 `
    -o (Join-Path $out 'any_speed_cruise_config.exe')
if ($LASTEXITCODE -ne 0) { throw "Config tool build failed ($LASTEXITCODE)" }

Get-ChildItem $out -Include *.dll, *.exe -Recurse | Select-Object Name, Length, LastWriteTime
