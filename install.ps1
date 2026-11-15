# Copies the plugin into a game's bin\win_x64\plugins folder.
#   .\install.ps1                  -> Euro Truck Simulator 2, found through Steam
#   .\install.ps1 -Game ats        -> American Truck Simulator, found through Steam
#   .\install.ps1 -GamePath "D:\Games\Euro Truck Simulator 2"
#   .\install.ps1 -Uninstall
param(
    [ValidateSet('ets2', 'ats')]
    [string]$Game = 'ets2',
    [string]$GamePath,
    [switch]$Uninstall
)
$ErrorActionPreference = 'Stop'
$root  = $PSScriptRoot
$files = 'any_speed_cruise.dll', 'any_speed_cruise_config.exe', 'any_speed_cruise.ini'

function Find-SteamGame([string]$folder) {
    $steam = (Get-ItemProperty 'HKCU:\Software\Valve\Steam' -ErrorAction SilentlyContinue).SteamPath
    if (-not $steam) { return $null }
    $libraries = @($steam)
    $vdf = Join-Path $steam 'steamapps\libraryfolders.vdf'
    if (Test-Path $vdf) {
        $libraries += Select-String -Path $vdf -Pattern '"path"\s+"(.+)"' |
            ForEach-Object { $_.Matches[0].Groups[1].Value.Replace('\\', '\') }
    }
    foreach ($library in $libraries) {
        $candidate = Join-Path $library "steamapps\common\$folder"
        if (Test-Path (Join-Path $candidate 'bin\win_x64')) { return $candidate }
    }
    return $null
}

if (-not $GamePath) {
    $folder = if ($Game -eq 'ats') { 'American Truck Simulator' } else { 'Euro Truck Simulator 2' }
    $GamePath = Find-SteamGame $folder
    if (-not $GamePath) { throw "$folder not found in the Steam libraries. Use -GamePath." }
}
$bin     = Join-Path $GamePath 'bin\win_x64'
$plugins = Join-Path $bin 'plugins'
if (-not (Test-Path $bin)) { throw "Game not found: $bin" }
if (Get-Process eurotrucks2, amtrucks -ErrorAction SilentlyContinue) { throw 'Close the game first.' }

if ($Uninstall) {
    $files | ForEach-Object { Remove-Item (Join-Path $plugins $_) -ErrorAction SilentlyContinue }
    "Removed from $plugins"
    return
}

$dll  = Join-Path $root 'build\any_speed_cruise.dll'
$tool = Join-Path $root 'build\any_speed_cruise_config.exe'
if (-not ((Test-Path $dll) -and (Test-Path $tool))) { throw 'Build first: .\build.ps1' }
New-Item -ItemType Directory -Force $plugins | Out-Null
Copy-Item $dll, $tool $plugins -Force
# Keep the player's settings on update.
if (-not (Test-Path (Join-Path $plugins 'any_speed_cruise.ini'))) {
    Copy-Item (Join-Path $root 'any_speed_cruise.ini') $plugins
}
"Installed in $plugins"
Get-ChildItem $plugins | Select-Object Name, Length, LastWriteTime
